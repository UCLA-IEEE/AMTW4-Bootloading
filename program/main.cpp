#include "drivers/driver_pin.h"
#include "registers/registers_gpio.h"
#include "driverlib/sysctl.h"
#include "inc/hw_gpio.h"
#include "driverlib/gpio.h"
#include "inc/hw_memmap.h"

typedef Pin::DigitalPin<SYSCTL_PERIPH_GPIOF, GPIO_PORTF_BASE, 3> RED_LED;

int main(void)
{ 
    SysCtlClockSet(SYSCTL_SYSDIV_2_5 | SYSCTL_USE_PLL | SYSCTL_OSC_MAIN
                   | SYSCTL_XTAL_16MHZ);

    RED_LED::init();

    RED_LED::setMode(Pin::Mode_e::MODE_OUTPUT);

    RED_LED::setState(Pin::State_e::STATE_HIGH);

    while(1)
    {
        SysCtlDelay(F_CPU / 3);

        RED_LED::setState((RED_LED::getState() == Pin::State_e::STATE_HIGH) ?
                            (Pin::State_e::STATE_LOW) :
                            (Pin::State_e::STATE_HIGH));
    
    }

    return 0;
}
