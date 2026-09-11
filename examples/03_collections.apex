// 03_collections.apex
List<String> colors = new List<String>{'Red', 'Green', 'Blue'};
colors.add('Yellow');

System.debug('Colors size: ' + colors.size());
System.debug('First color: ' + colors.get(0));

for (String c : colors) {
    System.debug('Color item: ' + c);
}

Map<String, Integer> fruitPrices = new Map<String, Integer>{
    'Apple' => 3,
    'Banana' => 2
};
fruitPrices.put('Orange', 4);

System.debug('Price Apple: ' + fruitPrices.get('Apple'));
System.debug('Fruit count: ' + fruitPrices.size());
