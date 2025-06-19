// generateCredits.js
const fs = require('fs');
const path = require('path');

const CREDITS_FILE = 'credits.json';
const DEFAULT_CREDIT = "Credit";
const DEFAULT_DESCRIPTION = "Description";

function findFragFiles(dir, found = []) {
  const items = fs.readdirSync(dir, { withFileTypes: true });

  for (const item of items) {
    const fullPath = path.join(dir, item.name);
    if (item.isDirectory()) {
      findFragFiles(fullPath, found);
    } else if (item.isFile() && item.name.endsWith('.frag')) {
      found.push(fullPath);
    }
  }

  return found;
}

function generateCreditsJson(fragFiles) {
  return {
    credits: fragFiles.map(file => ({
      name: path.basename(file, '.frag'), // Remove .frag extension
      credit: DEFAULT_CREDIT,
      description: DEFAULT_DESCRIPTION
    }))
  };
}

const fragFiles = findFragFiles(process.cwd());
const creditsJson = generateCreditsJson(fragFiles);

fs.writeFileSync(CREDITS_FILE, JSON.stringify(creditsJson, null, 2));
console.log(`Generated ${CREDITS_FILE} with ${fragFiles.length} entries.`);