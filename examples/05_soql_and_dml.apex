// 05_soql_and_dml.apex
Account acc1 = new Account();
acc1.Name = 'Acme Corp';
acc1.Industry = 'Technology';
insert acc1;

Account acc2 = new Account();
acc2.Name = 'Global Media';
acc2.Industry = 'Media';
insert acc2;

System.debug('Inserted ID: ' + acc1.Id);

List<Account> techList = [SELECT Id, Name, Industry FROM Account WHERE Industry = 'Technology'];

System.debug('Found ' + techList.size() + ' tech account(s):');
for (Account a : techList) {
    System.debug('Account: ' + a.Name + ' (' + a.Id + ')');
}
