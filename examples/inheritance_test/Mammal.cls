// Mammal.cls - Intermediate class extending Animal in another file
public virtual class Mammal extends Animal {
    public Boolean hasWarmBlood;

    public Mammal(String spec, Integer a) {
        this.species = spec;
        this.age = a;
        this.hasWarmBlood = true;
    }

    public virtual Boolean isWarmBlooded() {
        return this.hasWarmBlood;
    }

    public virtual String getCategory() {
        return 'Mammalia';
    }
}
