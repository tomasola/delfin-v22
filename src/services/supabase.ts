import { createClient } from '@supabase/supabase-js';

const supabaseUrl = import.meta.env.VITE_SUPABASE_URL;
const supabaseAnonKey = import.meta.env.VITE_SUPABASE_ANON_KEY;

export const supabase = createClient(supabaseUrl, supabaseAnonKey);

export function dataURLtoBlob(dataurl: string) {
    var arr = dataurl.split(','), mime = arr[0].match(/:(.*?);/)![1],
        bstr = atob(arr[1]), n = bstr.length, u8arr = new Uint8Array(n);
    while (n--) {
        u8arr[n] = bstr.charCodeAt(n);
    }
    return new Blob([u8arr], { type: mime });
}

export async function uploadCapture(refCode: string, dataUrl: string) {
    const blob = dataURLtoBlob(dataUrl);
    const fileName = `${refCode}_${Date.now()}.jpg`;
    const filePath = `${refCode}/${fileName}`;

    const { error: uploadError } = await supabase.storage
        .from('captures')
        .upload(filePath, blob, {
            contentType: 'image/jpeg',
            cacheControl: '3600',
            upsert: false
        });

    if (uploadError) throw uploadError;

    const { data: { publicUrl } } = supabase.storage
        .from('captures')
        .getPublicUrl(filePath);

    return publicUrl;
}

export async function saveCaptureMetadata(refCode: string, imageUrl: string, embedding: number[]) {
    const { error } = await supabase
        .from('user_captures')
        .insert([
            {
                ref_code: refCode,
                image_url: imageUrl,
                embedding: embedding
            }
        ]);

    if (error) throw error;
}

export async function fetchAllCaptures() {
    const { data, error } = await supabase
        .from('user_captures')
        .select('*')
        .order('created_at', { ascending: false });

    if (error) throw error;
    return data;
}

export function subscribeToCaptures(callback: (payload: any) => void) {
    return supabase
        .channel('public:user_captures')
        .on('postgres_changes', { event: 'INSERT', schema: 'public', table: 'user_captures' }, callback)
        .subscribe();
}
