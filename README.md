# ENGINE

## add git submodule
```sh
git submodule add git@github.com:jefersonbelmiro/engine.git
```

## build cli

```sh
mkdir -p bin/
gcc engine/tools/cli.c -o bin/cli -g -I./engine/src -I./src
```

## run cli

```sh
bin/cli
```



## debug

```sh
# debug memory leadk
./build.sh && valgrind -s --leak-check=full --track-origins=yes ./build/main

# debug segfault
gdb -batch -ex "run" -ex "bt" ./build/linux/main

# debug struct memory layout ans holes
# >> requied to compile with debug info (-g)
# with gdb
pahole -E -p -C struct_name bin_or_object_path
# in gdb
(gdb) ptype /o arena_t
```
