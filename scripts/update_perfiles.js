import fs from 'fs';
import path from 'path';
import { fileURLToPath } from 'url';

const __filename = fileURLToPath(import.meta.url);
const __dirname = path.dirname(__filename);

const ROOT_DIR = path.resolve(__dirname, '..');
const REFERENCES_PATH = path.join(ROOT_DIR, 'public', 'references.json');
const IMAGES_DIR = path.join(ROOT_DIR, 'public', 'images', 'perfiles');

// Check if images dir exists
if (!fs.existsSync(IMAGES_DIR)) {
    console.error(`Error: Images directory not found at ${IMAGES_DIR}`);
    process.exit(1);
}

// Read existing references
let references = [];
try {
    const data = fs.readFileSync(REFERENCES_PATH, 'utf-8');
    references = JSON.parse(data);
} catch (error) {
    console.error('Error reading references.json:', error);
    process.exit(1);
}

const existingCodes = new Set(references.map(r => r.code));
const files = fs.readdirSync(IMAGES_DIR);
let addedCount = 0;

console.log(`Scanning ${files.length} files in ${IMAGES_DIR}...`);

files.forEach(file => {
    const ext = path.extname(file);
    if (!['.jpg', '.jpeg', '.png', '.bmp', '.webp'].includes(ext.toLowerCase())) return;

    const code = path.basename(file, ext); // e.g. "P100001"

    // Check if already exists (avoid duplicates by code)
    if (!existingCodes.has(code)) {
        references.push({
            code: code,
            category: 'PERFILES',
            image: `/images/perfiles/${file}`
        });
        existingCodes.add(code);
        addedCount++;
    }
});

// Write update
fs.writeFileSync(REFERENCES_PATH, JSON.stringify(references, null, 2));
console.log(`✅ Successfully added ${addedCount} new references to category 'PERFILES'.`);
console.log(`Total references: ${references.length}`);
