const { spawnSync, execFileSync } = require('child_process');
const path = require('path');
const fs = require('fs');

function getNadirBinary() {
  const isWin = process.platform === 'win32';
  const binName = isWin ? 'nadir.exe' : 'nadir';

  const candidates = [
    path.join(__dirname, '..', '..', 'build', 'Release', binName),
    path.join(__dirname, '..', '..', 'build', binName),
    path.join(__dirname, '..', 'bin', binName),
    process.env.NADIR_BIN
  ].filter(Boolean);

  for (const p of candidates) {
    if (fs.existsSync(p)) return p;
  }
  return binName;
}

function runFile(filePath, options = {}) {
  const bin = getNadirBinary();
  const res = spawnSync(bin, [filePath], {
    encoding: 'utf-8',
    ...options
  });
  return {
    stdout: res.stdout || '',
    stderr: res.stderr || '',
    status: res.status,
    success: res.status === 0
  };
}

function runSource(apexSource, options = {}) {
  const bin = getNadirBinary();
  const res = spawnSync(bin, [], {
    input: apexSource,
    encoding: 'utf-8',
    ...options
  });
  return {
    stdout: res.stdout || '',
    stderr: res.stderr || '',
    status: res.status,
    success: res.status === 0
  };
}

function initProject(projectPath = '.', options = {}) {
  const bin = getNadirBinary();
  const res = spawnSync(bin, ['--init', projectPath], {
    encoding: 'utf-8',
    ...options
  });
  return {
    stdout: res.stdout || '',
    stderr: res.stderr || '',
    status: res.status,
    success: res.status === 0
  };
}

module.exports = {
  getNadirBinary,
  runFile,
  runSource,
  initProject
};
