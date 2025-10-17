project('air-quality-sensor', 'cpp',
    default_options: [
        'cpp_std=c++20'
    ],
)

assert(meson.is_cross_build(), 'Must be a cross-compile environment.')

boost_sml_inc = subproject('boost-sml').get_variable('boost_sml_inc')
avr_stdcpp_dep = subproject('avr-libstdcpp').get_variable('avr_stdcpp_dep')

cib_proj = subproject('compile-time-init-build')
cib_dep = cib_proj.get_variable('cib_dep')

arduinocore_proj = subproject('arduinocore-avr')
common_inc = include_directories('.')

avr_objcopy_exe = find_program(meson.get_cross_property('objcopy_exe'))
avrdude_exe = cib_proj.get_variable('avrdude_exe')
avrdude_conf = cib_proj.get_variable('avrdude_conf')

subdir('tools')
subdir('test_framework')
subdir('16bit-accelerator')

main_exe = executable('main',
    sources: [
        'main.cpp',
    ],
    cpp_args: [
        # https://github.com/boost-ext/sml/issues/617
        '-DBOOST_SML_CREATE_DEFAULT_CONSTRUCTIBLE_DEPS',
        '-Wno-volatile',
        '-Wno-array-bounds',

        # gcc internal compiler bug: add_clobbers for Serial.print
        #'-fno-lto',

        # Trying to compensate for the lack of LTO.
        '-fwhole-program',
        '-ffreestanding',
    ],
    include_directories: [
        boost_sml_inc,
        common_inc,
    ],
    dependencies: [
        cib_dep,
        # avr-ar does not support fat object files, why?
        arduinocore_proj.get_variable('arduinocore_dep'),
        arduinocore_proj.get_variable('arduinocore_wire_dep'),
        dsu_funcs_dep,
    ],
)

main_hex = custom_target('main.hex',
    input: main_exe,
    output: 'main.hex',
    command: [
        avr_objcopy_exe,
        '-O', 'ihex',
        '-R', '.eeprom',
        '@INPUT@',
        '@OUTPUT@',
    ],
)

flash_log = custom_target('flash.log',
    input: [
        main_hex,
    ],
    output: 'flash.log',
    command: [
        avrdude_exe, '-q', '-V', '-v',
        '-p', 'atmega328p',
        '-C', avrdude_conf,
        '-D', '-c', 'arduino',
        '-b', '57600',
        '-P', '/dev/ttyUSB0',
        '-U', 'flash:w:@INPUT0@:i',
    ],
    capture: true,
)

alias_target('flash', flash_log)