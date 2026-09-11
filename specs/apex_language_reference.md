# Salesforce Apex Language & Runtime Reference

Dieses Dokument fasst die wichtigsten Spezifikationen und Verhaltensweisen der Apex-Sprache zusammen, die für die Implementierung eines Interpreters oder Compilers relevant sind.

---

## 1. Lexikalische Besonderheiten

- **Case-Insensitivity:**  
  Apex unterscheidet **nicht** zwischen Groß- und Kleinschreibung bei Bezeichnern (Variablen, Methoden, Klassen, SOQL-Keywords):
  `System.debug` == `system.DEBUG` == `SYSTEM.Debug`
- **String Literale:**  
  Apex verwendet standardmäßig einfache Anführungszeichen `'Mein String'`. Escaping erfolgt über `\'` und `\\`.
- **Kommentare:**  
  Einzeilig `// ...` und mehrzeilig `/* ... */`.

---

## 2. Typensystem

### Primitive Typen
- `Blob`: Binärdaten
- `Boolean`: `true` / `false` / `null`
- `Date`: Nur Datum (`YYYY-MM-DD`)
- `Datetime`: Datum + Uhrzeit (`YYYY-MM-DDTHH:MM:SSZ`)
- `Decimal`: Beliebig genaue Fließkommazahl (Standard für Währungen/Berechnungen)
- `Double`: 64-Bit Gleitkommazahl
- `Id`: 15- oder 18-stellige hexadezimale Salesforce-Objekt-ID (z. B. `001000000000001AAA`)
- `Integer`: 32-Bit Ganzzahl
- `Long`: 64-Bit Ganzzahl
- `Object`: Basistyp für alle Objekte
- `String`: Zeichenkette
- `Time`: Reine Uhrzeit

### Collections
- `List<T>`: Geordnete Liste mit Indexzugriff (`[index]` oder `.get(index)`, `.add(item)`, `.size()`).
  - Initializer: `new List<String>{'A', 'B'}`
- `Set<T>`: Ungeordnete Menge eindeutiger Werte (`.add(item)`, `.contains(item)`).
- `Map<K, V>`: Schlüssel-Wert-Paare (`.put(k, v)`, `.get(k)`, `.keySet()`, `.values()`).
  - Initializer: `new Map<String, Integer>{'A' => 1, 'B' => 2}`

### SObjects (Salesforce Objects)
- Typisierte Datenbank-Objekte wie `Account`, `Contact`, `Opportunity` oder Custom Objects `Custom_Object__c`.
- Dynamischer Feldzugriff möglich (`acc.put('Name', 'Acme')`, `acc.get('Name')`).

---

## 3. DML-Operationen & SOQL

### DML Statements
- `insert <record | list>;`
- `update <record | list>;`
- `upsert <record | list>;`
- `delete <record | list>;`
- `undelete <record | list>;`

### Inline SOQL Syntax
In Apex werden SOQL-Queries direkt in eckigen Klammern formuliert:
```apex
List<Account> accs = [SELECT Id, Name FROM Account WHERE Industry = 'Technology' LIMIT 10];
```
Mit Variablen-Binding (`:`-Präfix):
```apex
String targetIndustry = 'Technology';
List<Account> accs = [SELECT Id, Name FROM Account WHERE Industry = :targetIndustry];
```

---

## 4. Spezielle Operatoren

- Safe Navigation Operator (`?.`):  
  Verhindert `NullPointerException`. `acc?.Contacts[0]?.Name` evaluiert zu `null`, wenn ein Zwischenglied `null` ist.
- Ternärer Operator (`?:`): `cond ? val1 : val2`
