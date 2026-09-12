#!/usr/bin/env node

const { spawnSync } = require('child_process');
const path = require('path');
const fs = require('fs');

function findBinary() {
  const isWin = process.platform === 'win32';
  const binExt = isWin ? '.exe' : '';
  const arch = process.arch;
  const platform = process.platform;

  // 1. Explicit environment variable override
  if (process.env.NADIR_BIN && fs.existsSync(process.env.NADIR_BIN)) {
    return process.env.NADIR_BIN;
  }

  // 2. Bundled platform-specific binary in bin/
  const platformBinNames = [
    `nadir-${platform}-${arch}${binExt}`,
    `nadir-${platform}-x64${binExt}`,
    `nadir${binExt}`
  ];

  for (const name of platformBinNames) {
    const p = path.join(__dirname, name);
    if (fs.existsSync(p)) return p;
  }

  // 3. Local build folder fallback (development mode)
  const localCandidates = [
    path.join(__dirname, '..', '..', 'build', 'Release', `nadir${binExt}`),
    path.join(__dirname, '..', '..', 'build', `nadir${binExt}`)
  ];

  for (const p of localCandidates) {
    if (fs.existsSync(p)) return p;
  }

  // 4. Fallback to system PATH
  return isWin ? 'nadir.exe' : 'nadir';
}

const bin = findBinary();
const args = process.argv.slice(2);

const res = spawnSync(bin, args, {
  stdio: 'inherit'
});

if (res.error) {
  console.error(`Failed to execute Nadir binary at "${bin}":`, res.error.message);
  process.exit(1);
}

process.exit(res.status !== null ? res.status : 0);
