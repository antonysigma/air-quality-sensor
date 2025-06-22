project('ENS160_driver', 'cpp')

arduinocore_proj = subproject('arduinocore-avr')

ens160_inc = include_directories('src')

ens160_lib = static_library('sciosense_ens160',
    sources: 'src/ScioSense_ENS160.cpp',
    cpp_args: ['-DARDUINO=180'],
    dependencies: [
        arduinocore_proj.get_variable('arduinocore_dep'),
        arduinocore_proj.get_variable('arduinocore_wire_dep'),
    ],
)

ens160_dep = declare_dependency(
    link_with: ens160_lib,
    compile_args: ['-DARDUINO=180'],
    include_directories: ens160_inc,
)