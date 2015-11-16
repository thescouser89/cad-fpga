## Compiling instructions

```
make
```

## Running instructions

```
# to run in parallel
./main <circuit> parallel

# to run graphics
./main <circuit> graphics

# to run normal
./main <circuit>

# Note: <circuit> can be cct1, cct2, cct3, or cct4.
```

Note: the graphics part only work on the non-parallel version of
      depth-first-search.

## To measure runtime
To measure the runtime, the command `time` is used.

```
# e.g
/usr/bin/time ./main cct3
```

## Flags passed to makefile
The author has modified the `makefile` so that `g++` compiles with the `-O2`
flag.
