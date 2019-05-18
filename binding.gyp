{
    'targets': [
        {
            'target_name': 'libcamera',
            'conditions': [
                ['OS=="linux"', {
                    'cflags': [
                        '-fPIC',
                    ]
                }]
            ],
            'type': 'shared_library',
            'include_dirs': [
                "/opt/pylon5/include",
            ],
            'CFLAGS_CC_Default': [
                '-fPIC'
            ],
            'sources': [
                'camera.hpp',
                'camera.cpp'
            ],
            'libraries': [
                '<!@(/opt/pylon5/bin/pylon-config --libs-rpath)',
                '<!@(/opt/pylon5/bin/pylon-config --libs)',                
            ]
        },
        {
            'target_name': 'cameraapp',
            'type': 'executable',
            "include_dirs": [
                "<out/Default/obj.target",
            ],
            "libraries": [
                '-Wl,-rpath,/home/home-dev/Documents/dev/bluedragon/camera-lib/build/out/Default/obj.target',
                '-L/home/home-dev/Documents/dev/bluedragon/camera-lib/build/out/Default/obj.target',
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
