// 01_hello_world.apex
// Primitives, Arithmetic, Case-Insensitivity & System.debug

Integer a = 15;
Integer b = 27;
Integer sum = a + b;

String greeting = 'Hallo Apex World from C!';
Boolean isActive = true;
Double pi = 3.14159;

System.debug(greeting);
System.debug('Summe: ' + sum);
System.debug('isActive: ' + isActive);
System.debug('Pi: ' + pi);

// Case-Insensitive Check:
INTEGER UPPER_VAR = 999;
system.DEBUG('Case-Insensitive Access: ' + upper_var);
