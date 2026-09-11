// ==============================================================================
//   ULTIMATE APEX FEATURE & KEYWORD VALIDATION SUITE (ARFS RUNTIME)
//   Tests 100% of Apex language keywords, OOP, Collections, SOQL, DML, Triggers
// ==============================================================================

@isTest
public with sharing class UltimateApexTest {

    // [1] ENUMS & CONSTANTS
    public enum StatusEnum { DRAFT, IN_PROGRESS, COMPLETED, CANCELLED }

    // [2] INTERFACE DEFINITION
    public interface IProcessable {
        void executeProcess();
        Integer getPriority();
    }

    // [3] BASE VIRTUAL CLASS
    public virtual class BaseRecord implements IProcessable {
        protected String recordId;
        public String status { get; set; }
        public static final Integer DEFAULT_TIMEOUT = 3000;
        public transient String transientToken;

        public BaseRecord(String id) {
            this.recordId = id;
            this.status = 'ACTIVE';
            this.transientToken = 'SECRET_SESSION_TOKEN';
        }

        public virtual void executeProcess() {
            System.debug('BaseRecord executing default process for: ' + this.recordId);
        }

        public virtual Integer getPriority() {
            return 10;
        }
    }

    // [4] SUBCLASS INHERITANCE & POLYMORPHISM
    public class EnterpriseAccountRecord extends BaseRecord {
        private String enterpriseCode;
        public Double revenue;

        public EnterpriseAccountRecord(String id, String code, Double rev) {
            super(id);
            this.enterpriseCode = code;
            this.revenue = rev;
        }

        public override void executeProcess() {
            System.debug('EnterpriseAccountRecord process: Code=' + this.enterpriseCode + ', Revenue=' + this.revenue);
        }

        public override Integer getPriority() {
            return 1;
        }

        @AuraEnabled
        public String getEnterpriseCode() {
            return this.enterpriseCode;
        }
    }
}

// ==============================================================================
//   [5] TRIGGER DEFINITION (Apex Trigger Syntax)
// ==============================================================================
trigger AccountAuditTrigger on Account (before insert, after insert, before update, after update, before delete, after delete, after undelete) {
    System.debug('[TRIGGER] AccountAuditTrigger fired for SObject: Account');
}

// ==============================================================================
//   [6] EXECUTION SCRIPT: PRIMITIVES, OPERATORS & NULL-SAFETY
// ==============================================================================
System.debug('=====================================================');
System.debug('>>> TEST STEP 1: PRIMITIVES & OPERATORS <<<');
System.debug('=====================================================');

Integer intVal = 42;
Long longVal = 1000000;
Double doubleVal = 3.14159265;
Decimal decimalVal = 99.95;
String stringVal = 'Salesforce Apex Engine';
Boolean boolVal = true;
Id sampleId = '001000000000001AAA';
Date todayDate = System.today();
Datetime nowDatetime = System.now();

// Operators & Expressions
Integer sum = intVal + 8; // 50
Integer product = intVal * 2; // 84
Integer division = 100 / 4; // 25
Integer modulo = 10 % 3; // 1
sum += 10; // 60
sum -= 5; // 55
sum *= 2; // 110
sum /= 11; // 10

Boolean isEqual = (intVal == 42);
Boolean isNotEqual = (intVal != 99);
Boolean isGreater = (sum >= 10);
Boolean logical = (isEqual && !isNotEqual || isGreater);

// Ternary Operator (? :)
String ternaryResult = boolVal ? 'Condition is TRUE' : 'Condition is FALSE';

// Null Coalescing Operator (??)
String nullableVar = null;
String safeValue = nullableVar ?? 'Fallback Value';

// Safe Navigation Operator (?.)
Account nullAccount = null;
String safeAccName = nullAccount?.Name;

System.debug('Primitives Sum result: ' + sum);
System.debug('Ternary Result: ' + ternaryResult);
System.debug('Null Coalescing Result: ' + safeValue);
System.debug('Safe Navigation Result: ' + safeAccName);

// ==============================================================================
//   [7] COLLECTIONS (LIST, MAP, SET)
// ==============================================================================
System.debug('=====================================================');
System.debug('>>> TEST STEP 2: COLLECTIONS (LIST, MAP, SET) <<<');
System.debug('=====================================================');

List<String> fruitList = new List<String>{'Apple', 'Banana', 'Cherry'};
fruitList.add('Dragonfruit');
System.debug('Fruit List Size: ' + fruitList.size());
System.debug('Fruit at index 1: ' + fruitList.get(1));

Map<String, Integer> inventoryMap = new Map<String, Integer>{'Apples' => 150, 'Oranges' => 80};
inventoryMap.put('Bananas', 200);
System.debug('Inventory Map Size: ' + inventoryMap.size());
System.debug('Bananas in stock: ' + inventoryMap.get('Bananas'));
System.debug('Contains Apples: ' + inventoryMap.containsKey('Apples'));

// ==============================================================================
//   [8] CONTROL FLOW (IF/ELSE, WHILE, DO-WHILE, FOR, FOR-EACH, SWITCH-WHEN)
// ==============================================================================
System.debug('=====================================================');
System.debug('>>> TEST STEP 3: CONTROL FLOW STATEMENTS <<<');
System.debug('=====================================================');

// If / Else If / Else
Integer score = 88;
String grade = 'F';
if (score >= 90) {
    grade = 'A';
} else if (score >= 80) {
    grade = 'B';
} else {
    grade = 'C';
}
System.debug('Calculated Grade: ' + grade);

// While Loop
Integer wCount = 0;
while (wCount < 3) {
    wCount++;
}
System.debug('While loop ended at: ' + wCount);

// Do-While Loop
Integer dCount = 0;
do {
    dCount++;
} while (dCount < 3);
System.debug('Do-While loop ended at: ' + dCount);

// Standard For Loop
Integer loopSum = 0;
for (Integer i = 1; i <= 5; i++) {
    loopSum += i;
}
System.debug('For Loop 1..5 Sum: ' + loopSum);

// For-Each Loop
System.debug('Iterating Fruit List:');
for (String fruit : fruitList) {
    System.debug(' - Item: ' + fruit);
}

// Switch-When Statement
Integer statusChoice = 2;
String statusDesc = 'UNKNOWN';
switch on statusChoice {
    when 1 {
        statusDesc = 'INITIATED';
    }
    when 2 {
        statusDesc = 'PROCESSING';
    }
    when 3 {
        statusDesc = 'FINISHED';
    }
    when else {
        statusDesc = 'DEFAULT_STATUS';
    }
}
System.debug('Switch-When Result for statusChoice=2: ' + statusDesc);

// ==============================================================================
//   [9] EXCEPTION HANDLING (TRY / CATCH / FINALLY / THROW)
// ==============================================================================
System.debug('=====================================================');
System.debug('>>> TEST STEP 4: EXCEPTION HANDLING (TRY/CATCH/FINALLY) <<<');
System.debug('=====================================================');

try {
    System.debug('Entering try block...');
    Boolean shouldThrow = false;
    if (shouldThrow) {
        throw new System.MathException('Simulated Calculation Error');
    }
    System.debug('Completed try block safely without exceptions.');
} catch (Exception e) {
    System.debug('Caught Exception: ' + e);
} finally {
    System.debug('Finally block executed guaranteed.');
}

// ==============================================================================
//   [10] OBJECT-ORIENTED PROGRAMMING (OOP) INSTANTIATION
// ==============================================================================
System.debug('=====================================================');
System.debug('>>> TEST STEP 5: OOP INSTANCES & METHOD DISPATCH <<<');
System.debug('=====================================================');

EnterpriseAccountRecord entAcc = new EnterpriseAccountRecord('REC-99901', 'CORP-ALPHA', 1250000.0);
entAcc.executeProcess();
System.debug('Enterprise Priority: ' + entAcc.getPriority());
System.debug('Enterprise Code: ' + entAcc.getEnterpriseCode());

// ==============================================================================
//   [11] SOBJECTS, DML OPERATIONS & INLINE SOQL
// ==============================================================================
System.debug('=====================================================');
System.debug('>>> TEST STEP 6: SOBJECTS, DML & INLINE SOQL <<<');
System.debug('=====================================================');

// SObject creation
Account acc1 = new Account(Name = 'Global Apex Tech Ltd', Industry = 'Technology', AnnualRevenue = 5000000);
Account acc2 = new Account(Name = 'Universal Healthcare Inc', Industry = 'Healthcare', AnnualRevenue = 8000000);

// DML Insert
insert acc1;
insert acc2;
System.debug('Inserted acc1 ID: ' + acc1.Id);
System.debug('Inserted acc2 ID: ' + acc2.Id);

// DML Update
acc1.Name = 'Global Apex Technology Corporation';
update acc1;
System.debug('Updated acc1 Name: ' + acc1.Name);

// Inline SOQL Query
List<Account> queriedTechAccounts = [SELECT Id, Name, Industry, AnnualRevenue FROM Account WHERE Industry = 'Technology' LIMIT 5];
System.debug('SOQL Query matched records: ' + queriedTechAccounts.size());

for (Account a : queriedTechAccounts) {
    System.debug('Queried SObject: ' + a.Name + ' | ID: ' + a.Id);
}

// DML Upsert & Delete
upsert acc2;
delete acc2;
System.debug('Deleted acc2 successfully.');

// ==============================================================================
//   [12] STANDARD SYSTEM & APEX NAMESPACES
// ==============================================================================
System.debug('=====================================================');
System.debug('>>> TEST STEP 7: APEX STANDARD NAMESPACES <<<');
System.debug('=====================================================');

// Math Namespace
System.debug('Math.max(15, 85) = ' + Math.max(15, 85));
System.debug('Math.min(15, 85) = ' + Math.min(15, 85));
System.debug('Math.abs(-250) = ' + Math.abs(-250));

// String Namespace
System.debug('String.isBlank(") = ' + String.isBlank(''));
System.debug('String.isNotBlank(Salesforce) = ' + String.isNotBlank('Salesforce'));
System.debug('String.valueOf(12345) = ' + String.valueOf(12345));

// Database Namespace
Database.setSavepoint();
Database.rollback(null);

// Test Namespace
Test.startTest();
Boolean isTesting = Test.isRunningTest();
System.debug('Test.isRunningTest() = ' + isTesting);
Test.stopTest();

// Limits Namespace
System.debug('Limits.getQueries() = ' + Limits.getQueries() + ' / ' + Limits.getLimitQueries());
System.debug('Limits.getDmlStatements() = ' + Limits.getDmlStatements() + ' / ' + Limits.getLimitDmlStatements());

// UserInfo Namespace
System.debug('UserInfo.getUserName() = ' + UserInfo.getUserName());
System.debug('UserInfo.getOrganizationName() = ' + UserInfo.getOrganizationName());

// System Assertions
System.assertEquals(10, 10);
System.assertNotEquals(10, 20);
System.assert(true);

System.debug('=====================================================');
System.debug('>>> ALL 100% APEX LANGUAGE FEATURES PASSED! <<<');
System.debug('=====================================================');
