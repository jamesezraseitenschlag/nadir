const vscode = require('vscode');
const path = require('path');
const fs = require('fs');

let replTerminal = null;

function getNadirBinaryPath() {
  const config = vscode.workspace.getConfiguration('nadir');
  const customPath = config.get('executablePath');
  if (customPath && fs.existsSync(customPath)) {
    return customPath;
  }

  const isWin = process.platform === 'win32';
  const binName = isWin ? 'nadir.exe' : 'nadir';
  const homeDir = process.env.USERPROFILE || process.env.HOME || '';

  const candidates = [
    path.join(homeDir, 'ARFS', 'build', 'Release', binName),
    path.join(homeDir, 'ARFS', 'build', binName),
    path.join(homeDir, '.nadir', 'bin', binName)
  ];

  // Check workspace build directory
  const folders = vscode.workspace.workspaceFolders;
  if (folders && folders.length > 0) {
    for (const folder of folders) {
      const wsPath = folder.uri.fsPath;
      candidates.unshift(
        path.join(wsPath, 'build', 'Release', binName),
        path.join(wsPath, 'build', binName),
        path.join(wsPath, 'bin', binName)
      );
    }
  }

  for (const c of candidates) {
    if (c && fs.existsSync(c)) return c;
  }

  // Fallback to system path
  return binName;
}

function detectShellType() {
  const shellPath = (vscode.env.shell || '').toLowerCase();
  const terminalConfig = vscode.workspace.getConfiguration('terminal.integrated');
  
  let profile = '';
  if (process.platform === 'win32') {
    profile = (terminalConfig.get('defaultProfile.windows') || '').toLowerCase();
  } else if (process.platform === 'darwin') {
    profile = (terminalConfig.get('defaultProfile.osx') || '').toLowerCase();
  } else {
    profile = (terminalConfig.get('defaultProfile.linux') || '').toLowerCase();
  }

  const combined = `${shellPath} ${profile}`;

  if (combined.includes('powershell') || combined.includes('pwsh')) {
    return 'powershell';
  }
  if (combined.includes('cmd.exe') || combined.includes('command prompt')) {
    return 'cmd';
  }
  if (combined.includes('bash') || combined.includes('zsh') || combined.includes('fish') || combined.includes('sh') || combined.includes('wsl')) {
    return 'posix';
  }

  // Default fallback based on platform
  if (process.platform === 'win32') {
    return 'powershell';
  }
  return 'posix';
}

function sendTerminalCommand(term, bin, args = '') {
  const shellType = detectShellType();
  let cmd = '';

  if (shellType === 'powershell') {
    // PowerShell call operator
    cmd = `& "${bin}" ${args}`.trim();
  } else if (shellType === 'cmd') {
    // Windows Command Prompt
    cmd = `"${bin}" ${args}`.trim();
  } else if (shellType === 'posix') {
    // Bash, Zsh, WSL, Git Bash
    const normalizedBin = process.platform === 'win32' ? bin.replace(/\\/g, '/') : bin;
    cmd = `"${normalizedBin}" ${args}`.trim();
  } else {
    cmd = `"${bin}" ${args}`.trim();
  }

  term.sendText(cmd);
}

function getTerminal() {
  if (replTerminal && !replTerminal.exitStatus) {
    return replTerminal;
  }
  replTerminal = vscode.window.createTerminal({
    name: 'Nadir Apex REPL',
    iconPath: new vscode.ThemeIcon('terminal')
  });
  return replTerminal;
}

function activate(context) {
  // Command: Run Current Apex File
  const runFileCmd = vscode.commands.registerCommand('nadir.runFile', (uri) => {
    const filePath = uri ? uri.fsPath : vscode.window.activeTextEditor?.document.uri.fsPath;
    if (!filePath) {
      vscode.window.showErrorMessage('No active Apex file to run.');
      return;
    }

    const bin = getNadirBinaryPath();
    const term = getTerminal();
    term.show();

    const config = vscode.workspace.getConfiguration('nadir');
    if (config.get('clearTerminalBeforeRun')) {
      term.sendText('clear');
    }

    sendTerminalCommand(term, bin, `"${filePath}"`);
  });

  // Command: Run Selection
  const runSelectionCmd = vscode.commands.registerCommand('nadir.runSelection', () => {
    const editor = vscode.window.activeTextEditor;
    if (!editor) return;

    const selection = editor.document.getText(editor.selection);
    if (!selection) {
      vscode.window.showInformationMessage('Please select some Apex code first.');
      return;
    }

    const bin = getNadirBinaryPath();
    const term = getTerminal();
    term.show();
    sendTerminalCommand(term, bin);
    term.sendText(selection);
  });

  // Command: Start REPL
  const startReplCmd = vscode.commands.registerCommand('nadir.startRepl', () => {
    const bin = getNadirBinaryPath();
    const term = getTerminal();
    term.show();
    sendTerminalCommand(term, bin);
  });

  // Command: Initialize SFDX Project & Metadata
  const initProjectCmd = vscode.commands.registerCommand('nadir.initProject', (uri) => {
    const folders = vscode.workspace.workspaceFolders;
    const targetDir = uri ? uri.fsPath : (folders ? folders[0].uri.fsPath : '.');

    const bin = getNadirBinaryPath();
    const term = getTerminal();
    term.show();
    sendTerminalCommand(term, bin, `--init "${targetDir}"`);
  });

  context.subscriptions.push(runFileCmd, runSelectionCmd, startReplCmd, initProjectCmd);
}

function deactivate() {
  if (replTerminal) {
    replTerminal.dispose();
  }
}

module.exports = {
  activate,
  deactivate
};
