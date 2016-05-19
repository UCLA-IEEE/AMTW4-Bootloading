#include "driverlib/sysctl.h"
#include "driverlib/flash.h"
#include "drivers/driver_serial.h"
#include <stdlib.h>

#undef Serial_module_debug
#define Serial_module_debug (Serial_module_3)

int main(void)
{
    SysCtlClockSet(SYSCTL_SYSDIV_2_5 | SYSCTL_USE_PLL | SYSCTL_XTAL_16MHZ
                   | SYSCTL_OSC_MAIN);

    Serial_init(Serial_module_debug, 115200);

    while(1)
    {
        Serial_puts(Serial_module_debug, "Hello, world!\r\n");
        SysCtlDelay(F_CPU / 3);
    }

    return 0;
}
