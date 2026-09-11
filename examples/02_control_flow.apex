// 02_control_flow.apex
Integer score = 85;

if (score >= 90) {
    System.debug('Grade: A');
} else if (score >= 80) {
    System.debug('Grade: B');
} else {
    System.debug('Grade: C or below');
}

Integer count = 0;
while (count < 3) {
    System.debug('While count: ' + count);
    count = count + 1;
}

Integer total = 0;
for (Integer i = 1; i <= 5; i++) {
    total = total + i;
}
System.debug('Total 1..5 = ' + total);
