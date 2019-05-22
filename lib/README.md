# Camera lib

*Build instructions:*

**Generate build folder with makefile:**

```sh
cd lib
gyp --generator-output=build --depth .
```

**Build library (.so):**
```sh
cd build
make libcamera
```

**Clean build environment:**
```sh
cd lib
rm build -rf
```