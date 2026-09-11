# Nadir Apex Runner for VS Code

Instant native Salesforce Apex execution, REPL, and SFDX metadata initialization right inside Visual Studio Code.

## Features

- **Lightning-Fast Execution**: Run Apex scripts (`.cls`, `.apex`, `.trigger`) locally without waiting for cloud deployment.
- **Interactive REPL**: Launch a native Nadir REPL session directly in your VS Code terminal.
- **SFDX Metadata Integration**: Automatically discovers and initializes CustomObjects (`*__c`), Custom Metadata (`*__mdt`), Custom Fields, and platform mock tables from `sfdx-project.json` and `force-app`.
- **Run Selections**: Highlight any block of Apex code in the editor and execute it immediately.

## Commands

| Command | Description |
|---|---|
| `Nadir: Run Current Apex File` | Execute the open Apex file in the terminal |
| `Nadir: Execute Selected Apex Code` | Run currently highlighted Apex snippet |
| `Nadir: Open Interactive Apex REPL` | Launch a live Nadir interactive session |
| `Nadir: Initialize SFDX Project & Metadata` | Scan and load project metadata into mock DB |

## Settings

- `nadir.executablePath`: Specify a custom path to your `nadir` / `nadir.exe` binary.
- `nadir.clearTerminalBeforeRun`: Automatically clear terminal before script execution (default: `true`).
