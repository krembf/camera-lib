{
    "targets": [
        {
            "target_name": "cameraaddon",
            "cflags!": ["-fno-exceptions"],
            "cflags_cc!": ["-fno-exceptions"],
            "sources": ["../../lib/cameraaddon.cc"],
            "include_dirs": [
                "<!@(node -p \"require('node-addon-api').include\")",
                "<(module_root_dir)/lib",
                "<!@(node -p \"require('napi-thread-safe-callback').include\")"
            ],
            "libraries": [
                # -L is for linkage time location of the library
                "-L<(module_root_dir)/../../lib/build/out/Default/obj.target",
                # -Wl is for run time location of the library. TODO is to figure out 
                # the target location when making release
                "-Wl,-rpath,<(module_root_dir)/../../lib/build/out/Default/obj.target",
                '-lcamera',  # full name is libcamera.so, linker knows how to translate
                '<!@(/opt/pylon5/bin/pylon-config --libs-rpath)',
                '<!@(/opt/pylon5/bin/pylon-config --libs)',
            ]
        }
    ]
}
