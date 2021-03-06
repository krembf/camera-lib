{
    "targets": [
        {
            "target_name": "cameraaddon",
            "cflags!": ["-fno-exceptions"],
            "cflags_cc!": ["-fno-exceptions"],
            "sources": ["cameraaddon.cc"],
            "include_dirs": [
                "<!@(node -p \"require('node-addon-api').include\")",
                "<(module_root_dir)/lib",
            ],
            "libraries": [
                '-Wl,-rpath,/home/home-dev/Documents/dev/bluedragon/camera-lib/lib/build/out/Default/obj.target',
                '-L/home/home-dev/Documents/dev/bluedragon/camera-lib/lib//build/out/Default/obj.target',
                '-lcamera', # full name is libcamera.so, linker knows how to translate
                '<!@(/opt/pylon5/bin/pylon-config --libs-rpath)',
                '<!@(/opt/pylon5/bin/pylon-config --libs)',
            ],
            'defines': ['NAPI_DISABLE_CPP_EXCEPTIONS'],
        }        
    ]
}
