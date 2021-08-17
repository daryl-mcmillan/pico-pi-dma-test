#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/clocks.h"
#include "hardware/pio.h"
#include "test.pio.h"

int main() {

    stdio_init_all();

    PIO pio = pio0;
    uint offset = pio_add_program(pio, &test_program);
    uint sm = 0;
    int firstPin = 25;
    int data = firstPin;
    int clk = firstPin+1;
    int enabled = firstPin+2;

    pio_gpio_init(pio, data);
    pio_gpio_init(pio, clk);
    pio_gpio_init(pio, enabled);
    pio_sm_set_consecutive_pindirs(pio, sm, firstPin, 3, true);
    pio_sm_config c = test_program_get_default_config(offset);
    sm_config_set_out_pins(&c, data, 1);
    sm_config_set_sideset_pins(&c, clk);
    //sm_config_set_fifo_join(&c, PIO_FIFO_JOIN_TX);
    sm_config_set_out_shift(&c, false/*shift_right*/, true/*autopull*/, 32/*pull_threshold*/);
    pio_sm_init(pio, sm, offset, &c);
    pio_sm_set_clkdiv_int_frac(pio, sm, 65535, 0);
    pio_sm_set_enabled(pio, sm, true);
    for(;;) {
        sleep_ms(2000);
        pio->txf[sm] = 32;
        sleep_ms(2000);
        pio->txf[sm] = 0b11001100101010101100110011110001;
        //sleep_ms(10000);
        for(;;) {}
    }
}