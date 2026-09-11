// test_interfaces.apex - Testing multi-file interfaces and inheritance
System.debug('=== Testing Multi-File Interface Inheritance ===');

AuditService audit = new AuditService();
audit.logMessage('Security token verified');

String status = audit.getAuditStatus();
System.debug('Audit Status: ' + status);
System.assertEquals('[AUDIT] Security token verified', status);

System.debug('=== Multi-File Interface Test Succeeded! ===');
