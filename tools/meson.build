read_tap_from_serial_py = files('read_tap_from_serial.py')[0]

elf2esbuild_py = files('elf2esbuild.py')[0]
cpp_filt_exe = find_program('c++filt')

pymod = import('python')
python_exe = pymod.find_installation('python')

test('elf2esbuild usage',
    python_exe,
    args: [
        elf2esbuild_py,
        '--help',
    ],
)