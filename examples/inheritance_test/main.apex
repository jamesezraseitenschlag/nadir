// main.apex - Driver script testing multi-file class inheritance

System.debug('=== Testing Multi-File Class Inheritance ===');

// 1. Instantiate Dog (subclass of Mammal, which is a subclass of Animal)
Dog myDog = new Dog('Golden Retriever', 3);

// 2. Test field inheritance across 3 levels
System.debug('Species: ' + myDog.species);
System.assertEquals('Canine', myDog.species);

System.debug('Breed: ' + myDog.breed);
System.assertEquals('Golden Retriever', myDog.breed);

System.debug('Age: ' + myDog.age);
System.assertEquals(3, myDog.age);

System.debug('Warm Blooded: ' + myDog.hasWarmBlood);
System.assertEquals(true, myDog.hasWarmBlood);

// 3. Test method overriding (Dog.makeSound overrides Animal.makeSound)
String sound = myDog.makeSound();
System.debug('Sound: ' + sound);
System.assertEquals('Woof! Woof!', sound);

// 4. Test method inheritance from level 2 (Mammal.isWarmBlooded and Mammal.getCategory)
Boolean warm = myDog.isWarmBlooded();
System.debug('Is warm blooded: ' + warm);
System.assertEquals(true, warm);

String category = myDog.getCategory();
System.debug('Category: ' + category);
System.assertEquals('Mammalia', category);

// 5. Test method inheritance from level 1 root (Animal.getHumanAgeYears and Animal.getSpeciesDescription)
Integer humanYears = myDog.getHumanAgeYears(7);
System.debug('Human age equivalent (x7): ' + humanYears);
System.assertEquals(21, humanYears);

String desc = myDog.getSpeciesDescription();
System.debug('Species description: ' + desc);
System.assertEquals('Species: Canine, Age: 3', desc);

// 6. Test Dog's own specialized method
String fetch = myDog.fetchToy('tennis ball');
System.debug('Fetch result: ' + fetch);
System.assertEquals('Golden Retriever is fetching the tennis ball', fetch);

System.debug('=== Multi-File Inheritance Test Succeeded! ===');
