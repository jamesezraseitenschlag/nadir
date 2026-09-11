# @nadir/cli

Cross-platform Node.js wrapper and CLI for **Nadir**, the high-performance native Salesforce Apex runtime engine.

## Installation

```bash
npm install -g @nadir/cli
```

Or run directly using `npx`:

```bash
npx @nadir/cli path/to/script.apex
```

## Usage

### Run an Apex script
```bash
nadir path/to/MyScript.apex
```

### Start interactive REPL
```bash
nadir
```

### Initialize an SFDX Project
```bash
nadir --init force-app
```

## Programmatic API

```javascript
const { runFile, runSource, initProject } = require('@nadir/cli');

// Run Apex code dynamically
const result = runSource("System.debug('Hello from Node!');");
console.log(result.stdout);

// Initialize SFDX project schema
const initRes = initProject('./force-app');
console.log(initRes.stdout);
```
