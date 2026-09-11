// Animal.cls - Base class in its own file
public virtual class Animal {
    public String species;
    public Integer age;

    public Animal(String spec, Integer a) {
        this.species = spec;
        this.age = a;
    }

    public virtual String makeSound() {
        return 'generic animal sound';
    }

    public virtual String getSpeciesDescription() {
        return 'Species: ' + this.species + ', Age: ' + this.age;
    }

    public Integer getHumanAgeYears(Integer factor) {
        return this.age * factor;
    }
}
