/******************************************************************************
 *
 * Default Linker script for the Texas Instruments TM4C123GH6PM
 *
 * This is derived from revision 12770 of the TivaWare Library.
 *
 *****************************************************************************/

MEMORY
{
    BOOT_FLASH (RX) : ORIGIN = 0x00000000, LENGTH = 0x00001000
    PROG_FLASH (RX) : ORIGIN = 0x00001000, LENGTH = 0x0003F000
    SRAM (WX)  : ORIGIN = 0x20000000, LENGTH = 0x00008000
}

REGION_ALIAS("REGION_BOOT_TEXT", BOOT_FLASH);
REGION_ALIAS("REGION_PROG_TEXT", PROG_FLASH);
REGION_ALIAS("REGION_BSS", SRAM);
REGION_ALIAS("REGION_DATA", SRAM);
REGION_ALIAS("REGION_STACK", SRAM);
REGION_ALIAS("REGION_HEAP", SRAM);
REGION_ALIAS("REGION_BOOT_ARM_EXIDX", BOOT_FLASH);
REGION_ALIAS("REGION_BOOT_ARM_EXTAB", BOOT_FLASH);
REGION_ALIAS("REGION_PROG_ARM_EXIDX", PROG_FLASH);
REGION_ALIAS("REGION_PROG_ARM_EXTAB", PROG_FLASH);

SECTIONS {

    PROVIDE (__prog_text_start__ = 0x00001000);
    PROVIDE (__prog_text_end__ = 0x00040000);

    PROVIDE (_intvecs_base_address = 0x0);

    .isr_vector (_intvecs_base_address) : AT (_intvecs_base_address) {
        __text_start__ = .;
        KEEP (*(.isr_vector))
    } > REGION_BOOT_TEXT

    PROVIDE (_vtable_base_address = 0x20000000);

    .vtable (_vtable_base_address) : AT (_vtable_base_address) {
        KEEP (*(.vtable))
    } > REGION_DATA

    .text : {
        CREATE_OBJECT_SYMBOLS
        KEEP (*(.text))
        *(.text.*)
        . = ALIGN(0x4);
        KEEP (*(.ctors))
        . = ALIGN(0x4);
        KEEP (*(.dtors))
        . = ALIGN(0x4);
        __init_array_start = .;
        KEEP (*(.init_array*))
        __init_array_end = .;
        *(.init)
        *(.fini*)
        __text_end__ = .;
    } > REGION_BOOT_TEXT

    PROVIDE (__etext = .);
    PROVIDE (_etext = .);
    PROVIDE (etext = .);
    /*PROVIDE (__text_end__ = .);*/

    .rodata : {
        *(.rodata)
        *(.rodata*)
    } > REGION_BOOT_TEXT

    .data : ALIGN (4) {
        __data_load__ = LOADADDR (.data);
        __data_start__ = .;
        KEEP (*(.data))
        KEEP (*(.data*))
        . = ALIGN (4);
        __data_end__ = .;
    } > REGION_DATA AT> REGION_BOOT_TEXT

    .ARM.exidx : {
        __exidx_start = .;
        *(.ARM.exidx* .gnu.linkonce.armexidx.*)
        __exidx_end = .;
    } > REGION_BOOT_ARM_EXIDX

    .ARM.extab : {
        *(.ARM.extab* .gnu.linkonce.armextab.*)
    } > REGION_BOOT_ARM_EXTAB

    .bss : {
        __bss_start__ = .;
        *(.shbss)
        KEEP (*(.bss))
        *(.bss.*)
        *(COMMON)
        . = ALIGN (4);
        __bss_end__ = .;
    } > REGION_BSS

    .heap : {
        __heap_start__ = .;
        end = __heap_start__;
        _end = end;
        __end = end;
        KEEP(*(.heap))
        __heap_end__ = .;
        __HeapLimit = __heap_end__;
    } > REGION_HEAP

    .stack : ALIGN(0x8) {
        _stack = .;
        __stack = .;
        KEEP(*(.stack))
    } > REGION_STACK
}
