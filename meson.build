project('gbemu', 'c',
  version : '0.1',
  default_options : ['warning_level=3'])

executable('gbemu',
           'src/main.c',
           'src/cpu.c',
           'src/cpu_instr.c',
           'src/ppu.c',
           'src/rom.c',
           'src/memory.c',
           install : true)

test_add = executable('test_add',
                  'src/tests/test_add.c',
                  'src/cpu.c',
                  'src/cpu_instr.c',
                  'src/ppu.c',
                  'src/rom.c',
                  'src/memory.c',
                  'src/tests/unity.c')

test('test_add', test_add)
