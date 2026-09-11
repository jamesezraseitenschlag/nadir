// =========================================================================
// Salesforce Apex - Comprehensive Language Features & Keywords Test Suite
// Covers: OOP, Modifiers, Types, Collections, Control Flow, SOQL, DML & Operators
// =========================================================================

// --- 1. ENUMS & INTERFACES ---
public enum AccountStatus {
    ACTIVE,
    PENDING,
    INACTIVE
}

public interface IAuditable {
    String getAuditLog();
}

// --- 2. ABSTRACT & INHERITANCE (Virtual, Override, Super, This) ---
public abstract class AbstractEntity {
    protected String entityId;
    public abstract String getEntityType();
}

public virtual class BaseRecord extends AbstractEntity implements IAuditable {
    public String createdBy;

    public BaseRecord(String createdBy) {
        this.createdBy = createdBy;
    }

    public virtual String getAuditLog() {
        return 'Created by: ' + this.createdBy;
    }

    public override String getEntityType() {
        return 'BaseRecord';
    }
}

// --- 3. CLASS WITH SHARING, STATIC, FINAL, PROPERTIES & CONSTRUCTOR ---
public with sharing class AccountManager extends BaseRecord {
    public static final String VERSION = '2.0';
    private String organizationName;
    public Integer transactionCount;

    // Property with getter & setter
    public String statusNote { get; set; }

    public AccountManager(String orgName, String user) {
        this.organizationName = orgName;
        this.createdBy = user;
        this.transactionCount = 0;
        this.statusNote = 'Initialized';
    }

    public override String getAuditLog() {
        return 'Org: ' + this.organizationName + ', ' + this.createdBy;
    }

    public Integer calculateDiscount(Integer basePrice, Integer percent) {
        Integer discount = (basePrice * percent) / 100;
        return basePrice - discount;
    }
}

// --- 4. PRIMITIVE DATA TYPES ---
Integer myInt = 100;
Long myLong = 5000000;
Double myDouble = 3.14159;
Decimal myDecimal = 199.99;
Boolean isActive = true;
String greeting = 'Apex Runtime Test Suite';
Id sampleId = '001000000000001AAA';
Date todayDate = '2026-09-11';
Datetime currentDt = '2026-09-11T12:00:00Z';
Time sampleTime = '12:00:00';
Blob myBlob = 'SampleBlobData';
Object genericObj = 'Generic Object String';

System.debug('=== [1] PRIMITIVES & OPERATORS ===');
System.debug('Greeting: ' + greeting);
System.debug('Integer: ' + myInt + ' | Double: ' + myDouble + ' | Decimal: ' + myDecimal);
System.debug('IsActive: ' + isActive + ' | SampleId: ' + sampleId);

// Operators: Arithmetic, Compound, Comparison, Logical, Unary
Integer a = 20;
Integer b = 10;
a += 5;
b *= 2;
Integer result = (a + b) - (5 * 2) / 2;
System.debug('Arithmetik (a=25, b=20): ' + result);

Boolean testEq = (a == 25) && (b != 15) && !(a < 10);
System.debug('Logische Operatoren (Expected True): ' + testEq);

// Safe Navigation (?.) and Null Coalesce (??)
AccountManager nullMgr = null;
String safeAccess = nullMgr?.statusNote;
System.debug('Safe Navigation Result: ' + safeAccess);

// --- 5. COLLECTIONS (List, Map, Set) ---
System.debug('=== [2] COLLECTIONS ===');

// List
List<String> frameworkList = new List<String>{'Salesforce', 'Apex', 'SOQL'};
frameworkList.add('Lightning');
System.debug('Framework List Size: ' + frameworkList.size());
System.debug('First Item: ' + frameworkList.get(0));

// Map
Map<String, Integer> scoreMap = new Map<String, Integer>{
    'Alice' => 95,
    'Bob' => 88
};
scoreMap.put('Charlie', 92);
System.debug('Score for Alice: ' + scoreMap.get('Alice'));
System.debug('ScoreMap Size: ' + scoreMap.size());

// --- 6. CONTROL FLOW (if-else, while, for, foreach) ---
System.debug('=== [3] CONTROL FLOW ===');

// If - Else If - Else
Integer score = 88;
if (score >= 90) {
    System.debug('Performance: Excellent');
} else if (score >= 80) {
    System.debug('Performance: Very Good');
} else {
    System.debug('Performance: Needs Improvement');
}

// While Loop
Integer counter = 1;
while (counter <= 3) {
    System.debug('While Counter: ' + counter);
    counter++;
}

// For Loop
Integer loopSum = 0;
for (Integer i = 1; i <= 4; i++) {
    loopSum += i;
}
System.debug('For Loop Sum 1..4 = ' + loopSum);

// For-Each Loop
System.debug('Iterating Frameworks:');
for (String fw : frameworkList) {
    System.debug(' - Framework: ' + fw);
}

// --- 7. OOP INSTANTIATION & METHODS ---
System.debug('=== [4] OOP EXECUTION ===');
AccountManager mgr = new AccountManager('CloudTech Systems', 'AdminUser');
mgr.statusNote = 'Running Full Feature Test';
System.debug(mgr.getAuditLog());
System.debug('Status Note: ' + mgr.statusNote);

Integer finalPrice = mgr.calculateDiscount(500, 20);
System.debug('Discounted Price (500 - 20%): ' + finalPrice);

// --- 8. SOBJECTS, DML & INLINE SOQL ---
System.debug('=== [5] SOBJECTS, DML & INLINE SOQL ===');

// SObject creation
Account acc1 = new Account();
acc1.Name = 'Apex Global Innovations';
acc1.Industry = 'Technology';
acc1.AnnualRevenue = 1500000.00;
insert acc1;

Account acc2 = new Account();
acc2.Name = 'Apex Retail Partners';
acc2.Industry = 'Retail';
acc2.AnnualRevenue = 450000.00;
insert acc2;

System.debug('Inserted Account 1 ID: ' + acc1.Id);
System.debug('Inserted Account 2 ID: ' + acc2.Id);

// DML Update
acc1.Name = 'Apex Global Innovations Ltd.';
update acc1;
System.debug('Updated Account 1 Name: ' + acc1.Name);

// Inline SOQL Query with WHERE, ORDER BY, LIMIT
String targetIndustry = 'Technology';
List<Account> queriedTech = [SELECT Id, Name, Industry FROM Account WHERE Industry = 'Technology' ORDER BY Name ASC LIMIT 5];

System.debug('Found ' + queriedTech.size() + ' Technology Account(s):');
for (Account acc : queriedTech) {
    System.debug('Queried Account: ' + acc.Name + ' (ID: ' + acc.Id + ')');
}

// Built-in Namespaces (Math, String, System)
System.debug('=== [6] BUILT-IN SYSTEM NAMESPACES ===');
System.debug('Math.max(10, 45) = ' + Math.max(10, 45));
System.debug('Math.min(10, 45) = ' + Math.min(10, 45));
System.debug('Math.abs(-99) = ' + Math.abs(-99));
System.debug('String.isBlank("") = ' + String.isBlank(''));
System.debug('String.isNotBlank("Apex") = ' + String.isNotBlank('Apex'));

System.debug('=== ALL APEX FEATURES & KEYWORDS TEST COMPLETED SUCCESSFULLY ===');
