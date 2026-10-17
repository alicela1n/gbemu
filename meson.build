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

test_adc = executable('test_adc',
                  'src/tests/test_adc.c',
                  'src/cpu.c',
                  'src/cpu_instr.c',
                  'src/ppu.c',
                  'src/rom.c',
                  'src/memory.c',
                  'src/tests/unity.c')

test_sub = executable('test_sub',
                  'src/tests/test_sub.c',
                  'src/cpu.c',
                  'src/cpu_instr.c',
                  'src/ppu.c',
                  'src/rom.c',
                  'src/memory.c',
                  'src/tests/unity.c')

test_sbc = executable('test_sbc',
                  'src/tests/test_sbc.c',
                  'src/cpu.c',
                  'src/cpu_instr.c',
                  'src/ppu.c',
                  'src/rom.c',
                  'src/memory.c',
                  'src/tests/unity.c')

test('test_add', test_add)
test('test_adc', test_adc)
test('test_sub', test_sub)
test('test_sbc', test_sbc)
