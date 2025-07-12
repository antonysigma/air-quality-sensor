dsu_funcs_lib = static_library('dsu_funcs',
    sources: '__dsu_funcs.S',
    c_args: [
        '-DENABLE_DSU_INTERNAL_FUNCS=0',
        '-ffunction-sections',
    ],
)

dsu_funcs_dep = declare_dependency(
    link_with: dsu_funcs_lib,
    include_directories: '.',
)

dsu_tests_exe = executable('dsu_tests',
    sources: [
        'test_main.cpp',
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
    ],
    include_directories: [
        boost_sml_inc,
        '..',
    ],
    dependencies: [
        cib_dep,
        # avr-ar does not support fat object files, why?
        arduinocore_proj.get_variable('arduinocore_dep'),
        arduinocore_proj.get_variable('arduinocore_wire_dep'),
        dsu_funcs_dep,
        mcu_tests_dep,
    ],
)

dsu_tests_hex = custom_target('dsu_tests.hex',
    input: dsu_tests_exe,
    output: 'dsu_tests.hex',
    command: [
        avr_objcopy_exe,
        '-O', 'ihex',
        '-R', '.eeprom',
        '@INPUT@',
        '@OUTPUT@',
    ],
)

dsu_tests_upload_command = [
    avrdude_exe.full_path(), '-q', '-V', '-v',
    '-p', 'atmega328p',
    '-C', avrdude_conf,
    '-D', '-c', 'arduino',
    '-b', '57600',
    '-P', '/dev/ttyUSB0',
    '-U', 'flash:w:@0@:i'.format(dsu_tests_hex.full_path()),
]

test('16-bit arithmetic co-processor (uDSC)',
    pymod.find_installation('python3'),
    args: [
        read_tap_from_serial_py,
    ] + test_runner_upload_command,
    depends: test_runner_hex,
    suite: 'dsu',
    protocol: 'tap',
    is_parallel: false,
)