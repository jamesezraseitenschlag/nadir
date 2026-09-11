# Salesforce Apex Language & Runtime Reference

> i am too lazy to translate myself so theres a slop translation for you

This document summarizes the core specifications and runtime semantics of the Salesforce Apex programming language relevant to parser, compiler, and interpreter implementations.

---

## 1. Lexical Semantics

- **Case-Insensitivity:**  
  Apex is fully case-insensitive for all identifiers, keywords, types, and SOQL clauses:  
  `System.debug` == `system.DEBUG` == `SYSTEM.Debug`
- **String Literals:**  
  Apex uses single-quoted strings: `'My String'`. Escaping is performed via `\'` and `\\`.
- **Comments:**  
  Single-line comments using `// ...` and multi-line block comments using `/* ... */`.

---

## 2. Type System

### Primitive Types
- `Blob`: Binary data payload
- `Boolean`: `true` / `false` / `null` (three-valued logic)
- `Date`: Calendar date without time component (`YYYY-MM-DD`)
- `Datetime`: ISO timestamp with timezone/UTC component (`YYYY-MM-DDTHH:MM:SSZ`)
- `Decimal`: Arbitrary-precision floating point number (standard for financial calculations and currency)
- `Double`: 64-bit IEEE 754 floating point number
- `Id`: 15-character or 18-character case-safe alphanumeric Salesforce identifier (e.g. `001000000000001AAA`)
- `Integer`: 32-bit signed integer
- `Long`: 64-bit signed integer
- `Object`: Universal base type for all reference objects and primitives
- `String`: UTF-8 character sequence
- `Time`: Pure time-of-day value

### Collections
- `List<T>`: Ordered collection with indexed element access (`[index]` or `.get(index)`, `.add(item)`, `.size()`).
  - Literal initialization: `new List<String>{'A', 'B'}`
- `Set<T>`: Unordered collection of unique values (`.add(item)`, `.contains(item)`).
- `Map<K, V>`: Hash table mapping keys to values (`.put(k, v)`, `.get(k)`, `.keySet()`, `.values()`).
  - Literal initialization: `new Map<String, Integer>{'A' => 1, 'B' => 2}`

### SObjects (Salesforce Objects)
- Strongly typed database entity models such as standard entities (`Account`, `Contact`, `Opportunity`) and custom entities (`Custom_Object__c`).
- Dynamic field manipulation via `.put(fieldName, value)` and `.get(fieldName)`.

---

## 3. DML Operations & SOQL

### DML Statements
- `insert <record | list>;`
- `update <record | list>;`
- `upsert <record | list>;`
- `delete <record | list>;`
- `undelete <record | list>;`

### Inline SOQL Syntax
SOQL queries are embedded directly within square brackets:
```apex
List<Account> accs = [SELECT Id, Name FROM Account WHERE Industry = 'Technology' LIMIT 10];
```

Variable binding utilizes the `:` prefix:
```apex
String targetIndustry = 'Technology';
List<Account> accs = [SELECT Id, Name FROM Account WHERE Industry = :targetIndustry];
```

---

## 4. Special Operators

- **Safe Navigation Operator (`?.`):**  
  Prevents `NullPointerException` during deep dereferencing. `acc?.Contacts[0]?.Name` evaluates to `null` if any intermediate property evaluates to `null`.
- **Null Coalescing Operator (`??`):**  
  Evaluates to the right-hand operand when the left-hand operand is `null`: `String name = acc.Name ?? 'Default';`
- **Exact Equality (`===` / `!==`):**  
  Reference/strict identity comparison.
- **Ternary Conditional (`?:`):**  
  `condition ? expressionIfTrue : expressionIfFalse`
