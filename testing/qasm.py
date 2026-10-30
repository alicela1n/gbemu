#!/usr/bin/env python3
import sys
import subprocess
import os
# Quick wrapper script around rgbds to make assembling
# test assembly quicker

# main(): Main function
def main():
  asm_filename = sys.argv[1]
  object_filename = os.path.splitext(asm_filename)[0]+'.o'
  gb_filename = os.path.splitext(asm_filename)[0]+'.gb'

  subprocess.run(["rgbasm", asm_filename, "-o", object_filename], capture_output=True)
  subprocess.run(["rgblink", object_filename, "-o", gb_filename], capture_output=True)
  subprocess.run(["rgbfix", gb_filename, "-o", gb_filename], capture_output=True)

  with open(gb_filename, "rb") as gb_file:
    gb_contents = gb_file.read()
  
  gb_contents = gb_contents.strip(b'\0')
  hex_string = ', '.join(f"{b:#02x}" for b in gb_contents)
  print(f'dummy_rom array: {{{hex_string}}}')
  print(f'dummy_rom size : {len(gb_contents)}')

if __name__ == '__main__':
  main()

