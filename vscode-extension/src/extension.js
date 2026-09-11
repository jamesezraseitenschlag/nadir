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

  // Check workspace build directory
  const folders = vscode.workspace.workspaceFolders;
  if (folders && folders.length > 0) {
    const wsPath = folders[0].uri.fsPath;
    const candidates = [
      path.join(wsPath, 'build', 'Release', binName),
      path.join(wsPath, 'build', binName),
      path.join(wsPath, 'bin', binName)
    ];
    for (const c of candidates) {
      if (fs.existsSync(c)) return c;
    }
  }

  // Fallback to system path
  return binName;
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

    term.sendText(`"${bin}" "${filePath}"`);
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
    term.sendText(`"${bin}"`);
    term.sendText(selection);
  });

  // Command: Start REPL
  const startReplCmd = vscode.commands.registerCommand('nadir.startRepl', () => {
    const bin = getNadirBinaryPath();
    const term = getTerminal();
    term.show();
    term.sendText(`"${bin}"`);
  });

  // Command: Initialize SFDX Project & Metadata
  const initProjectCmd = vscode.commands.registerCommand('nadir.initProject', (uri) => {
    const folders = vscode.workspace.workspaceFolders;
    const targetDir = uri ? uri.fsPath : (folders ? folders[0].uri.fsPath : '.');

    const bin = getNadirBinaryPath();
    const term = getTerminal();
    term.show();
    term.sendText(`"${bin}" --init "${targetDir}"`);
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
