test('example tap report',
    find_program('cat'),
    args: [
        files('test_report.txt'),
    ],
    protocol: 'tap',
)

mcu_tests_dep = declare_dependency(
    include_directories: '.',
)

test_runner_exe = executable('test_runner',
    sources: [
        'test_main.cpp',
        #'main.cpp',
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
        '..',
    ],
    dependencies: [
        cib_dep,
        # avr-ar does not support fat object files, why?
        arduinocore_proj.get_variable('arduinocore_dep'),
        arduinocore_proj.get_variable('arduinocore_wire_dep'),
    ],
)

test_runner_hex = custom_target('test_runner.hex',
    input: test_runner_exe,
    output: 'test_runner.hex',
    command: [
        avr_objcopy_exe,
        '-O', 'ihex',
        '-R', '.eeprom',
        '@INPUT@',
        '@OUTPUT@',
    ],
)

test_runner_upload_command = [
    avrdude_exe.full_path(), '-q', '-V', '-v',
    '-p', 'atmega328p',
    '-C', avrdude_conf,
    '-D', '-c', 'arduino',
    '-b', '57600',
    '-P', '/dev/ttyUSB0',
    '-U', 'flash:w:@0@:i'.format(test_runner_hex.full_path()),
]

pymod = import('python')

test('Capture test script over serial',
    pymod.find_installation('python3'),
    args: [
        read_tap_from_serial_py,
    ] + test_runner_upload_command,
    depends: test_runner_hex,
    suite: 'test_framework',
    protocol: 'tap',
    is_parallel: false,
)