/* Linker simplificado usado só pelos testes do emulador (tools/emu430.py):
   código em 0x4400, dados em 0x2400, sem rotina de boot da RTS. */
-l msp430f5529.cmd
MEMORY
{
    RAM   : origin = 0x2400, length = 0x2000
    FLASH : origin = 0x4400, length = 0xBB80
}
SECTIONS
{
    .data  : {} > RAM
    .bss   : {} > RAM
    .stack : {} > RAM (HIGH)
    .text  : {} > FLASH
}
