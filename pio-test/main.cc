#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/clocks.h"
#include "hardware/pio.h"
#include "test.pio.h"

uint8_t reverseBits( uint8_t b ) {
    uint8_t result = 0;
    for( int i=0; i<8; i++ ) {
        result = (result << 1) | ( b & 1 );
        b = b >> 1;
    }
    return result;
}

uint32_t byteToWord( uint8_t b ) {
    uint32_t w = b;
    return (w & 0b11000000) << 24
        | (w & 0b00110000) << (16+2)
        | (w & 0b00001100) << (8+4)
        | (w & 0b00000011) << (0+6);
}

int main() {

    stdio_init_all();

    PIO pio = pio0;
    uint offset = pio_add_program(pio, &test_program);
    uint sm = 0;
    int firstPin = 16;
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
    sm_config_set_fifo_join(&c, PIO_FIFO_JOIN_TX);
    sm_config_set_out_shift(&c, /*shift_right?*/false, /*autopull?*/true, /*pull_threshold*/32);
    pio_sm_init(pio, sm, offset, &c);
    pio_sm_set_clkdiv_int_frac(pio, sm, 4, 0);
    pio_sm_set_enabled(pio, sm, true);
    int buffer_size = 52*240+2;
    uint32_t buffer[buffer_size];
    int count = 0;
    buffer[count++] = byteToWord(0b10000000);
    for( int row=1; row<=240; row++ ) {
        buffer[count++] = byteToWord( reverseBits( row ) );
        // data
        for( int col=0; col<50; col++ ) {
            buffer[count++] = byteToWord( 0b11110000 );
        }
        buffer[count++] = 0;
    }
    buffer[count++] = 0;
    for(;;) {
        buffer[0] ^= byteToWord(0b01000000);
        sleep_ms(100);
        pio_sm_put_blocking(pio, sm, buffer_size * 4);
        for( int i=0; i<buffer_size; i++ ) {
            pio_sm_put_blocking(pio, sm, buffer[i]);
        }
    }
}