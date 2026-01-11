
import fs from 'fs';
import path from 'path';
import { fileURLToPath } from 'url';
import sharp from 'sharp';
import '@tensorflow/tfjs-backend-wasm';
import * as tf from '@tensorflow/tfjs';
import * as mobilenet from '@tensorflow-models/mobilenet';

const __filename = fileURLToPath(import.meta.url);
const __dirname = path.dirname(__filename);
const ROOT_DIR = path.resolve(__dirname, '..');
const EMBEDDINGS_FILE = path.join(ROOT_DIR, 'public', 'embeddings.json');
const TEST_IMAGE = path.join(ROOT_DIR, 'test_input.jpg');

async function main() {
    console.log('Setting backend to WASM...');
    await tf.setBackend('wasm');

    console.log('Loading MobileNet...');
    const model = await mobilenet.load({ version: 2, alpha: 1.0 });

    console.log('Loading embeddings...');
    const embeddings = JSON.parse(fs.readFileSync(EMBEDDINGS_FILE));
    console.log(`Loaded ${embeddings.length} embeddings.`);

    console.log(`Processing test image: ${TEST_IMAGE}`);

    // Process input image exactly like the generation script
    // SIMULATE CENTER CROP
    const buffer = fs.readFileSync(TEST_IMAGE);
    // Get metadata to calculate crop
    const metadata = await sharp(buffer).metadata();
    const width = metadata.width;
    const height = metadata.height;
    // Crop center 50%
    const cropSize = Math.min(width, height) * 0.5;
    const left = Math.floor((width - cropSize) / 2);
    const top = Math.floor((height - cropSize) / 2);

    console.log(`Simulating Crop: ${Math.floor(cropSize)}x${Math.floor(cropSize)} at ${left},${top}`);

    // APPLY CROP + GREYSCALE + THRESHOLD
    const { data, info } = await sharp(buffer)
        .extract({ left, top, width: Math.floor(cropSize), height: Math.floor(cropSize) }) // EXTRACT CROP
        .grayscale()
        .threshold(128)
        .toColourspace('srgb') // Force back to 3 channels (R=G=B) so MobileNet is happy
        .resize(224, 224, { fit: 'fill' })
        .removeAlpha()
        .raw()
        .toBuffer({ resolveWithObject: true });

    const tensor = tf.tidy(() => {
        // Data is now guaranteed to be 3 channels
        return tf.tensor3d(new Uint8Array(data), [224, 224, 3]).toFloat();
    });

    const logits = model.infer(tensor, true);
    const inputVector = await logits.array();
    const inputFlat = inputVector[0];

    tf.dispose([tensor, logits]);

    // Calculate Cosine Similarity
    console.log('Calculating matches...');
    const matches = embeddings.map(record => {
        const score = cosineSimilarity(inputFlat, record.embedding);
        return { code: record.code, score };
    });

    matches.sort((a, b) => b.score - a.score);

    console.log('--- TOP 10 MATCHES ---');
    matches.slice(0, 10).forEach((m, i) => {
        console.log(`#${i + 1}: ${m.code} (Score: ${m.score.toFixed(4)})`);
    });

    const target = matches.find(m => m.code === '10.008');
    if (target) {
        const rank = matches.indexOf(target) + 1;
        console.log(`\nTARGET '10.008' found at rank #${rank} with score ${target.score.toFixed(4)}`);
    } else {
        console.log("\nTARGET '10.008' NOT FOUND in database.");
    }
}

function cosineSimilarity(a, b) {
    let dotProduct = 0;
    let normA = 0;
    let normB = 0;
    for (let i = 0; i < a.length; i++) {
        dotProduct += a[i] * b[i];
        normA += a[i] * a[i];
        normB += b[i] * b[i];
    }
    return dotProduct / (Math.sqrt(normA) * Math.sqrt(normB));
}

main().catch(console.error);
