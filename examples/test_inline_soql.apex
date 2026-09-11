// test_inline_soql.apex - Comprehensive inline SOQL / SQL test suite

System.debug('=== Running Inline SOQL / SQL Test Suite ===');

// 1. Insert multiple test records
Account a1 = new Account(Name = 'Salesforce Inc', Industry = 'Software', NumberOfEmployees = 50000);
insert a1;

Account a2 = new Account(Name = 'Alphabet Inc', Industry = 'Software', NumberOfEmployees = 180000);
insert a2;

Account a3 = new Account(Name = 'Boeing Co', Industry = 'Aerospace', NumberOfEmployees = 140000);
insert a3;

Account a4 = new Account(Name = 'Airbus SE', Industry = 'Aerospace', NumberOfEmployees = 130000);
insert a4;

// 2. Test Basic SELECT and WHERE equality
List<Account> softwareAccounts = [SELECT Id, Name, Industry FROM Account WHERE Industry = 'Software'];
System.debug('Software Accounts found: ' + softwareAccounts.size());
System.assertEquals(2, softwareAccounts.size());

// 3. Test Variable Binding in WHERE clause
String targetIndustry = 'Aerospace';
List<Account> aerospaceAccounts = [SELECT Id, Name, Industry FROM Account WHERE Industry = :targetIndustry];
System.debug('Aerospace Accounts found: ' + aerospaceAccounts.size());
System.assertEquals(2, aerospaceAccounts.size());

// 4. Test Numeric Comparison (> and <)
Integer minEmployees = 150000;
List<Account> largeCompanies = [SELECT Id, Name, NumberOfEmployees FROM Account WHERE NumberOfEmployees > :minEmployees];
System.debug('Companies with > 150k employees: ' + largeCompanies.size());
System.assertEquals(1, largeCompanies.size());

// 5. Test LIMIT clause
List<Account> limitedAccounts = [SELECT Id, Name FROM Account WHERE Industry = 'Aerospace' LIMIT 1];
System.debug('Limited query result count: ' + limitedAccounts.size());
System.assertEquals(1, limitedAccounts.size());

// 6. Test SOQL with Contact SObjects
Contact c1 = new Contact(FirstName = 'Marc', LastName = 'Benioff', AccountId = a1.Id);
insert c1;
Contact c2 = new Contact(FirstName = 'Sundar', LastName = 'Pichai', AccountId = a2.Id);
insert c2;

List<Contact> contacts = [SELECT Id, FirstName, LastName, AccountId FROM Contact WHERE LastName = 'Benioff'];
System.debug('Contact query found: ' + contacts.size());
System.assertEquals(1, contacts.size());
System.debug('Contact Full Name: ' + contacts[0].FirstName + ' ' + contacts[0].LastName);

// 7. Test DML Update and Re-Query
a1.Name = 'Salesforce.com Inc';
update a1;

List<Account> updatedQuery = [SELECT Id, Name FROM Account WHERE Name = 'Salesforce.com Inc'];
System.assertEquals(1, updatedQuery.size());
System.debug('Updated Account Name verified: ' + updatedQuery[0].Name);

// 8. Test Savepoints & Rollbacks with SOQL
Integer sp = Database.setSavepoint();

Account aTemp = new Account(Name = 'Temporary Corp', Industry = 'Finance');
insert aTemp;

List<Account> beforeRollback = [SELECT Id, Name FROM Account WHERE Name = 'Temporary Corp'];
System.assertEquals(1, beforeRollback.size());

Database.rollback(sp);

List<Account> afterRollback = [SELECT Id, Name FROM Account WHERE Name = 'Temporary Corp'];
System.assertEquals(0, afterRollback.size());
System.debug('Rollback verified: temporary record was successfully rolled back');

// 9. Test Delete operation and SOQL filtering
delete a4;
List<Account> postDeleteQuery = [SELECT Id, Name FROM Account WHERE Industry = 'Aerospace'];
System.assertEquals(1, postDeleteQuery.size());
System.debug('Post-delete Aerospace count: ' + postDeleteQuery.size());

System.debug('=== All Inline SOQL / SQL tests passed successfully! ===');
