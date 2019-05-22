# Camera test app

*Build instructions:*

**Generate build folder with makefile:**

```sh
cd app
gyp --generator-output=build --depth .
```

**Build application:**
```sh
cd build
make cameraapp
```

**Clean build environment:**
```sh
cd app
rm build -rf
```

**Run application:**
```sh
 ./build/out/Default/cameraapp
```