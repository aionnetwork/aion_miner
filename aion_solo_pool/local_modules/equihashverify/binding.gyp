{
    "targets": [
        {
            "target_name": "equihashverify",
            "dependencies": [
                "libequi",
            ],
            "sources": [
                "equihashverify.cc",
            ],
            "include_dirs": [
                "<!(node -e \"require('nan')\")"
            ],
            "defines": [
            ],
            "cflags_cc": [
                "-std=c++11",
                "-Wl,--whole-archive",
                "-fPIC",
            ],
            "link_settings": {
                "libraries": [
                    "-Wl,-rpath,./build/Release/",
                ]
            },
        },
        {
            "target_name": "libequi",
            "type": "<(library)",
            "dependencies": [
            ],
            "sources": [
                "src/blake/blake2-config.h",
                "src/blake/blake2-impl.h",
                "src/blake/blake2-round.h",
                "src/blake/blake2.h",
                "src/blake/blake2b-load-sse2.h",
                "src/blake/blake2b-load-sse41.h",
                "src/blake/blake2b-round.h",
                "src/blake/blake2b.cpp",
                "src/equi/equi210.cpp",
                "src/equi/endian.c",
            ],
            "include_dirs": [
            ],
            "defines": [
            ],
            "cflags_c": [
                "-std=c11",
                "-Wl,--whole-archive",
                "-fPIC",
                "-Wno-pointer-sign",
                "-D_GNU_SOURCE"
            ],
            "link_settings": {
                "libraries": [
                ],
            },
        }
    ]
}

