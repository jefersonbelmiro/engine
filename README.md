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
