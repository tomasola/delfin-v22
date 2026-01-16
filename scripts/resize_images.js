const Jimp = require('jimp');
const path = require('path');

const images = [
    { src: 'public/icon-192.png', w: 192, h: 192 },
    { src: 'public/icon-512.png', w: 512, h: 512 },
    { src: 'public/screenshot-narrow.png', w: 540, h: 720 },
    { src: 'public/screenshot-wide.png', w: 720, h: 540 }
];

async function resizeAll() {
    for (const img of images) {
        try {
            const imagePath = path.resolve(__dirname, '..', img.src);
            console.log(`Reading ${imagePath}...`);
            const jimpImage = await Jimp.read(imagePath);

            console.log(`Resizing to ${img.w}x${img.h}...`);
            await jimpImage
                .resize(img.w, img.h)
                .writeAsync(imagePath);

            console.log(`Success: ${img.src}`);
        } catch (error) {
            console.error(`Error processing ${img.src}:`, error);
        }
    }
}

resizeAll();
