{
    'targets': [
        {
            'target_name': 'libcamera',
            'conditions': [
                ['OS=="linux"', {
                    'cflags': [
                        '-fPIC'
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
        }
    ]
}
