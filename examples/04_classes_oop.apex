// 04_classes_oop.apex
public class Calculator {
    private String name;

    public Calculator(String name) {
        this.name = name;
    }

    public Integer multiply(Integer a, Integer b) {
        return a * b;
    }

    public void printInfo() {
        System.debug('Calculator Name: ' + this.name);
    }
}

Calculator calc = new Calculator('ApexCalc C-Native');
calc.printInfo();
Integer result = calc.multiply(8, 9);
System.debug('8 * 9 = ' + result);
