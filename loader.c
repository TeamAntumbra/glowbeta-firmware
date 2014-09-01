#include <avr/io.h>
#include <avr/interrupt.h>

#include "led.h"
#include "rawusb.h"
#include "flash.h"
#include "proto.h"

int main(void)
{
    cli();
    led_init();
    MCUCR = _BV(IVCE);
    MCUCR = _BV(IVSEL);
    sei();

    DDRB &= ~_BV(DDB2);
    PORTB |= _BV(PORTB2);

    if (~PINB & _BV(PINB2))
        led_set_rgb(0, 0, 1);
    else {
        cli();
        MCUCR = _BV(IVCE);
        MCUCR = 0;
        __asm__("jmp 0");
    }

    rawusb_init();

    while (1) {
        rawusb_tick();

        uint32_t api;
        uint16_t cmd;
        const void *cmdbuf = proto_recv(&api, &cmd);
        if (cmdbuf) {
            if (api == 0 && cmd == 0)
                proto_send(0, cmdbuf, 56);

            else if (api == 0 && cmd == 1) {
                uint32_t qapi = proto_get_u32(&cmdbuf);
                uint8_t sup = qapi == 0 ? 1 : 0;
                proto_send(0, &sup, 1);
            }

            else {
                proto_send_start(1);
                proto_send_end();
            }
        }
    }
}
