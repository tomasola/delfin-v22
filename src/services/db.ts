import { doc, getDoc, setDoc } from 'firebase/firestore';
import { db, isConfigured } from '../firebase';
import type { ReferenceData } from '../types';

const COLLECTION_NAME = 'references';

export const getReferenceData = async (code: string): Promise<ReferenceData | null> => {
    // Fail-fast if not configured to avoid timeouts
    if (!isConfigured) {
        console.warn("Firebase not configured. Using localStorage.");
        const localData = localStorage.getItem(`ref_${code}`);
        return localData ? JSON.parse(localData) as ReferenceData : null;
    }

    try {
        const docRef = doc(db, COLLECTION_NAME, code);
        const docSnap = await getDoc(docRef);

        if (docSnap.exists()) {
            return docSnap.data() as ReferenceData;
        } else {
            // Fallback to localStorage if not found in DB
            const localData = localStorage.getItem(`ref_${code}`);
            if (localData) {
                return JSON.parse(localData) as ReferenceData;
            }
            return null;
        }
    } catch (error) {
        console.error("Error getting document:", error);
        const localData = localStorage.getItem(`ref_${code}`);
        if (localData) {
            return JSON.parse(localData) as ReferenceData;
        }
        return null;
    }
};

export const saveReferenceData = async (code: string, data: ReferenceData): Promise<boolean> => {
    // Always save to localStorage first for immediate feedback/offline
    localStorage.setItem(`ref_${code}`, JSON.stringify(data));

    if (!isConfigured) {
        return true; // "Saved" locally successfully
    }

    try {
        await setDoc(doc(db, COLLECTION_NAME, code), data);
        return true;
    } catch (error) {
        console.error("Error writing document:", error);
        return false;
    }
};
