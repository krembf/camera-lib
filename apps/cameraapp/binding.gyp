{
    'targets': [
        {
            'target_name': 'cameraapp',
            'type': 'executable',
            "include_dirs": [
                "<out/Default/obj.target",
                '/home/home-dev/Documents/dev/bluedragon/camera-lib/lib'
            ],
            "libraries": [
                '-Wl,-rpath,/home/home-dev/Documents/dev/bluedragon/camera-lib/lib/build/out/Default/obj.target',
                '-L/home/home-dev/Documents/dev/bluedragon/camera-lib/lib/build/out/Default/obj.target',
                '-lcamera', # full name is libcamera.so, linker knows how to translate
                '<!@(/opt/pylon5/bin/pylon-config --libs-rpath)',
                '<!@(/opt/pylon5/bin/pylon-config --libs)',
            ],
            'sources': [
                'cameraapp.cpp'
            ]
        }
    ]
}
