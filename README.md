# server-side-ecma-script

## Build

```bash
make
```


### Prepare v8

```bash
git clone git@github.com:v8/v8-git-mirror.git deps/v8
cd deps/v8
git checkout 3.31.1
```

##### References

* [Changelog](https://code.google.com/p/v8/source/browse/trunk/ChangeLog)
* [Build](https://code.google.com/p/v8-wiki/wiki/BuildingWithGYP)


### Prepare libuv

```bash
git clone git@github.com:libuv/libuv.git deps/uv
cd deps/uv
git checkout v1.0.1
```

##### References

* [Changelog](https://github.com/libuv/libuv/blob/master/ChangeLog)
* [Build](https://github.com/libuv/libuv#build-instructions)
