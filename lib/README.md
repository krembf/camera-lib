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

**Image Format Converter using Pylon SDK**
https://stackoverflow.com/questions/24101877/basler-pylon-4-sdk-and-opencv-2-4-9-cpylonimage-to-mat
