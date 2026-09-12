# @jamesezraseitenschlag/nadir

Cross-platform Node.js wrapper and CLI for **Nadir**, the high-performance native Salesforce Apex runtime engine.

## Installation from GitHub Packages

Configure npm to use the GitHub Packages registry for the `@jamesezraseitenschlag` scope:

```bash
npm config set @jamesezraseitenschlag:registry https://npm.pkg.github.com
```

Install globally:

```bash
npm install -g @jamesezraseitenschlag/nadir
```

Or run directly using `npx`:

```bash
npx @jamesezraseitenschlag/nadir path/to/script.apex
```

## CLI Usage

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
const { runFile, runSource, initProject } = require('@jamesezraseitenschlag/nadir');

// Run Apex code dynamically
const result = runSource("System.debug('Hello from Node!');");
console.log(result.stdout);

// Initialize SFDX project schema
const initRes = initProject('./force-app');
console.log(initRes.stdout);
```
