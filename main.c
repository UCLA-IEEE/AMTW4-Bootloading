#include "driverlib/sysctl.h"
#include "driverlib/flash.h"
#include "drivers/driver_serial.h"
#include <stdlib.h>

#undef Serial_module_debug
#define Serial_module_debug (Serial_module_3)

#define dptr(x) (*((volatile uint32_t*)(x)))

//#define DRY_RUN

const uint8_t RX_START_SENTINEL = 'S';
const uint8_t RX_TERMINATOR = 'T';
const uint8_t RX_ACK = 'K';
const uint8_t RX_NACK = 'X';
const uint8_t TX_START_SENTINEL = 's';
const uint8_t TX_TERMINATOR = 't';
const uint8_t TX_ACK = 'k';
const uint8_t TX_NACK = 'x';
const uint8_t BOOT_SENTINEL = 'q';

extern uint32_t __prog_text_start__;
extern uint32_t __prog_text_end__;

bool checkRange(uint32_t addr)
{
    return ((addr >= (uint32_t)(&__prog_text_start__)) &&
            (addr < (uint32_t)(&__prog_text_end__)));
}

typedef struct
{
    uint32_t addr;
    uint32_t len;
    uint8_t buf[1024];
} program_block_t;

static program_block_t main_block;

/**
 * @brief Programs a block of the flash memory.
 *
 * First, we perform a bounds check on the provided block address to ensure that
 * we are writing into the flash, and that we are not overwriting the bootloader
 * itself. Then, we erase the block with FlashErase(addr). Finally, we program
 * the block with FlashProgram(buf, addr, len).
 *
 * @param block The program_block_t* indicating the data to program and where to
 *              program it.
 * @return true on success, false otherwise.
 */
bool program_block_write(const program_block_t* block)
{
    if(!checkRange(block->addr) || !checkRange(block->addr + block->len - 1))
        return false;

    /*
     * Erase the flash block. This will return 0 on success.
     */
    if(FlashErase(block->addr) != 0)
        return false;

    /*
     * Program the flash block. This will return 0 on success.
     */
    if(FlashProgram((uint32_t*)block->buf, block->addr, block->len) != 0)
        return false;

    return true;
}

typedef enum
{
    RXTX_STATE_IDLE,
    RXTX_STATE_ADDR,
    RXTX_STATE_DATA_A,
    RXTX_STATE_DATA_B,
    RXTX_STATE_TERMINATOR
} program_rxtx_state_t;

typedef enum
{
    RXSTATUS_SUCCESS,
    RXSTATUS_FAILURE,
    RXSTATUS_DONE
} program_rx_status_t;

/**
 * @brief Converts a given ASCII-coded hexadecimal character into its
 *        corresponding binary nibble.
 */
uint8_t ascii_to_b16_idx(uint8_t c)
{
    if(c < '0' || c > 'z' || (c > 'Z' && c < 'a'))
        return 0;

    if(c <= '9')
        return c - '0';
    if(c >= 'a')
        return c - 'a' + 10;
    if(c >= 'A')
        return c - 'A' + 10;

    return 0;
}

/**
 * @brief Converts a given binary nibble into the corresponding ASCII-coded
 *        hexadecimal character.
 */
uint8_t b16_idx_to_ascii(uint8_t h)
{
    if(h > 15)
        return 0;
    if(h < 10)
        return '0' + h;
    return 'A' + h - 10;
}

/**
 * @brief Receives a block of program data over the debug serial port, and ACKs
 *        it.
 *
 * @param block A pointer to a program_block_t into which to store the program
 *              data.
 * @return A boolean value indicating success.
 */
program_rx_status_t program_block_rx(program_block_t* block)
{
    program_rxtx_state_t rxstate = RXTX_STATE_IDLE;
    uint8_t addrcounter = 0;
    uint8_t rxbyte = 0;

    while(1)
    {
        // Receive a single character
        if(Serial_avail(Serial_module_debug))
        {
            rxbyte = Serial_getc(Serial_module_debug);
        }
        else
        {
            continue;
        }

        // Process that character
        switch(rxstate)
        {
            /*
             * We were idle; if this character is the start sentinel, set stuff
             * up.
             */
            case RXTX_STATE_IDLE:
                if(rxbyte == RX_START_SENTINEL)
                    rxstate = RXTX_STATE_ADDR;
                if(rxbyte == BOOT_SENTINEL)
                    return RXSTATUS_DONE;
                block->len = 0;
                block->addr = 0;
                break;

            /*
             * We are receiving the block address. Shift nibbles into the
             * address field of the block one at a time, until we have 8.
             */
            case RXTX_STATE_ADDR:
                if(rxbyte == RX_TERMINATOR)
                {
                    Serial_putc(Serial_module_debug, RX_NACK);
                    return RXSTATUS_FAILURE;
                }

                /*
                 * Nibbles are given MS first, so we need to shift them in from
                 * the right.
                 */
                block->addr <<= 4;
                block->addr |= ascii_to_b16_idx(rxbyte);

                /*
                 * When we have shifted in all 8, move on to receiving the data.
                 */
                addrcounter++;
                if(addrcounter == 8)
                    rxstate = RXTX_STATE_DATA_A;
                break;

            /*
             * We are receiving the block data. Trade-off states between DATA_A
             * and DATA_B to receive the upper and lower nibbles of each byte,
             * respectively. Increment the data len field whenever we finish
             * with a lower nibble. If we see the terminator in DATA_A, we ACK
             * and return true. If we see the terminator in DATA_B, it's an
             * error and we NACK and return false.
             */
            case RXTX_STATE_DATA_A:
                if(rxbyte == RX_TERMINATOR)
                {
                    Serial_putc(Serial_module_debug, RX_ACK);
                    return RXSTATUS_SUCCESS;
                }

                block->buf[block->len] = ascii_to_b16_idx(rxbyte) << 4;
                rxstate = RXTX_STATE_DATA_B;

                break;

            case RXTX_STATE_DATA_B:
                if(rxbyte == RX_TERMINATOR)
                {
                    Serial_putc(Serial_module_debug, RX_NACK);
                    return RXSTATUS_FAILURE;
                }

                block->buf[block->len++] |= ascii_to_b16_idx(rxbyte);
                rxstate = RXTX_STATE_DATA_A;

                break;
        }
    }

    return false;
}

uint8_t getcBlocking()
{
    while(!Serial_avail(Serial_module_debug));
    return (uint8_t)Serial_getc(Serial_module_debug);
}

/**
 * @brief Sends a program_block_t back to the host bootloader program for
 *        for verification.
 */
bool program_block_tx(const program_block_t* block)
{
    Serial_putc(Serial_module_debug, TX_START_SENTINEL);
    size_t i = 0;
    for(i = 0; i < 8; i++)
    {
        Serial_putc(Serial_module_debug,
                    b16_idx_to_ascii((block->addr >> (7-i)*4)&0xF));
    }

    for(i = 0; i < block->len; i++)
    {
        uint8_t txdata = block->buf[i];
        uint8_t txupper = b16_idx_to_ascii(txdata >> 4);
        uint8_t txlower = b16_idx_to_ascii(txdata & 0xF);
        Serial_putc(Serial_module_debug, txupper);
        Serial_putc(Serial_module_debug, txlower);
    }

    Serial_putc(Serial_module_debug, TX_TERMINATOR);

    uint8_t c = getcBlocking();
    if(c == TX_ACK)
        return true;
    return false;
}

void runTargetProgram()
{
    Serial_puts(Serial_module_debug, "Booting target...");

    /*
     * Relocate VTABLE.
     */
    dptr(0xE000E000 + 0xD08) = __prog_text_start__;

    /*
     * Do some things:
     *
     * Load the SP with the target program initial SP, given in the zeroth entry
     * of the target program's vector table. We get the address of the vector
     * table from the symbol __prog_text_start__, which is defined by the
     * linker script as 0x00001000. This is also where the vector table starts,
     * as the linker script specifies to place the vector table into the flash
     * ahead of all other text.
     *
     * Load the PC with the target program's RESET vector, given in the first
     * entry of the target program's vector table.
     */
    asm volatile(
        "ldr r0, =__prog_text_start__\n\t"
        "ldr r1, [r0]\n\t"
        "mov sp, r1\n\t"
        "add r0, #4\n\t"
        "dsb\n\t"
        "isb\n\t"
        "ldr pc, [r0]\n\t"
    );
}

void runBootloader()
{
    program_rx_status_t rxstatus;
    while(1)
    {
        rxstatus = program_block_rx(&main_block);

        if(rxstatus == RXSTATUS_FAILURE)
            continue;

        if(rxstatus == RXSTATUS_DONE)
        {
            runTargetProgram();
        }

        if(!program_block_tx(&main_block))
            continue;
#ifndef DRY_RUN
        program_block_write(&main_block);
#endif
    }
}

int main(void)
{
    SysCtlClockSet(SYSCTL_SYSDIV_2_5 | SYSCTL_USE_PLL | SYSCTL_XTAL_16MHZ
                   | SYSCTL_OSC_MAIN);

    /*
     * Initialize the serial port.
     */
    Serial_init(Serial_module_debug, 115200);

    /*
     * Put an information string to the host.
     */
    Serial_puts(Serial_module_debug, "Send any char to halt bootloader");

    /*
     * Flush any recieved characters.
     */
    while(Serial_avail(Serial_module_debug))
        Serial_getc(Serial_module_debug);

    /*
     * Wait for the host to attach. Send a '.' every second. If we receive a
     * character back, it means that the host has seen our '.' and is
     * responding. If we time out (after 5 seconds), then give up and run the
     * target program.
     */
    int i;
    for(i = 0; i < 5; i++)
    {
        if(Serial_avail(Serial_module_debug))
        {
            while(Serial_avail(Serial_module_debug))
                Serial_getc(Serial_module_debug);
            runBootloader();
        }
        else
        {
            SysCtlDelay(F_CPU / 3 / 1);
            Serial_putc(Serial_module_debug, '.');
        }
    }

    runTargetProgram();

    return 0;
}
