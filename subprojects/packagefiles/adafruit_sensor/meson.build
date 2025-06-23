project('Adafruit_Sensor', 'cpp')

arduinocore_proj = subproject('arduinocore-avr')

adafruit_sensor_lib = static_library('sciosense_ens160',
    sources: 'Adafruit_Sensor.cpp',
    cpp_args: ['-DARDUINO=180'],
    dependencies: [
        arduinocore_proj.get_variable('arduinocore_dep'),
        arduinocore_proj.get_variable('arduinocore_wire_dep'),
    ],
)

adafruit_sensor_dep = declare_dependency(
    link_with: adafruit_sensor_lib,
    compile_args: ['-DARDUINO=180'],
    include_directories: '.',
)