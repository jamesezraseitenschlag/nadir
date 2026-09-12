const { spawnSync, execFileSync } = require('child_process');
const path = require('path');
const fs = require('fs');

function getNadirBinary() {
  const isWin = process.platform === 'win32';
  const binExt = isWin ? '.exe' : '';
  const arch = process.arch;
  const platform = process.platform;

  if (process.env.NADIR_BIN && fs.existsSync(process.env.NADIR_BIN)) {
    return process.env.NADIR_BIN;
  }

  const candidates = [
    path.join(__dirname, '..', 'bin', `nadir-${platform}-${arch}${binExt}`),
    path.join(__dirname, '..', 'bin', `nadir-${platform}-x64${binExt}`),
    path.join(__dirname, '..', 'bin', `nadir${binExt}`),
    path.join(__dirname, '..', '..', 'build', 'Release', `nadir${binExt}`),
    path.join(__dirname, '..', '..', 'build', `nadir${binExt}`)
  ];

  for (const p of candidates) {
    if (fs.existsSync(p)) return p;
  }
  return isWin ? 'nadir.exe' : 'nadir';
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
