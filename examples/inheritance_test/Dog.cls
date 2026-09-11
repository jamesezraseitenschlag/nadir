// Dog.cls - Derived class extending Mammal in its own file
public class Dog extends Mammal {
    public String breed;

    public Dog(String br, Integer a) {
        this.species = 'Canine';
        this.age = a;
        this.breed = br;
        this.hasWarmBlood = true;
    }

    public override String makeSound() {
        return 'Woof! Woof!';
    }

    public String fetchToy(String toy) {
        return this.breed + ' is fetching the ' + toy;
    }
}
