# How to compile
Just type `make`

# How to run
```
./main <cct>

# e.g
./main cct2
```

To get the part 4 result, after you run `./main cct2`, click on 'Proceed'.


To get the part 1 result, just run `./main cct1`

To get the part 2 result, you'll have to modify the main.cpp file a little bit.
- Comment line 136 and line 143
- Uncomment line 138, 144 - 148
- Compile via `make'`
- Run via `./main cct2`
- Click on 'Proceed' to get another randomized I/O placement


To get the part 3 result, you'll have to modify the main.cpp file a little bit.
- Comment line 143
- Uncomment line 144 - 148
- Compile via `make`
- Run via `./main cct3`
- Click on 'Proceed' to see the blocks being divided
- You can continue clicking on 'Proceed' to see the overlap removal being
  applied recursively
- You can modify the weight of the net in `analytical_placer.cpp`, variable
  `weight`.
