project('adafruit_ahtx0', 'cpp')

ahtx0_lib = static_library('ahtx0',
    sources: 'Adafruit_AHTX0.cpp',
    dependencies: [
        dependency('adafruit_sensor'),
        dependency('adafruit_busio'),
    ]
)

ahtx0_dep = declare_dependency(
    link_with: ahtx0_lib,
    include_directories: '.',
    dependencies: [
        dependency('adafruit_sensor'),
        dependency('adafruit_busio'),
    ],
)