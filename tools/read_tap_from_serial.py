import re
import subprocess
import sys
from typing import List

import serial

# Set up the serial port (adjust as needed)
SERIAL_PORT = "/dev/ttyUSB0"  # Change to COMx on Windows
BAUD_RATE = 115200

# Compile the regex pattern
stop_pattern = re.compile(r"^1\.\.")


def uploadFirmware(upload_command: List[str]) -> bytes:
    return subprocess.check_output(upload_command)

def captureTAPFromSerial() -> None:
    with serial.Serial(SERIAL_PORT, BAUD_RATE, timeout=30) as ser:
        while True:
            line = ser.readline().decode("utf-8", errors="replace")
            print(line)
            if stop_pattern.match(line):
                break


if __name__ == "__main__":
    if len(sys.argv) <= 1:
        print(f"Usage: {sys.argv[0]} [avrdude command]")
        sys.exit(1)

    # TODO(Antony): The startup and teardown sequence is supposed to be run in a
    # hardware test framework (e.g. Google OpenHTF).
    upload_log = uploadFirmware(sys.argv[1:])
    captureTAPFromSerial()

    # Print the upload log
    print('# ' + '\n# '.join(upload_log.decode('utf-8', errors='replace').splitlines()))