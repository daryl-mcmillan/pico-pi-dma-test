#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/dma.h"
#include "hardware/pio.h"
#include "multiply.pio.h"

int main() {

    stdio_init_all();

    PIO pio = pio0;
    uint program_offset = pio_add_program(pio, &multiply_program);
    pio_sm_config pio_config = multiply_program_get_default_config(program_offset);
    sm_config_set_out_shift(&pio_config, /*shift_right?*/false, /*autopull?*/false, 32/*pull_threshold*/);
    sm_config_set_in_shift(&pio_config, /*shift_right?*/true, /*autopush?*/true, 32/*push_threshold*/);
    pio_sm_init(pio, 0, program_offset, &pio_config);
    pio_sm_init(pio, 1, program_offset, &pio_config);
    pio_sm_init(pio, 2, program_offset, &pio_config);
    pio_sm_init(pio, 3, program_offset, &pio_config);
    pio_sm_set_enabled(pio, 0, true);
    pio_sm_set_enabled(pio, 1, true);
    pio_sm_set_enabled(pio, 2, true);
    pio_sm_set_enabled(pio, 3, true);

    //for( ;; ) {
    //    pio_sm_put_blocking(pio, 0, val);
    //    pio_sm_put_blocking(pio, 0, 0b1110010);
    //    for( int i=0; i<16; i++ ) {
    //        uint32_t result = pio_sm_get_blocking(pio, 0);
    //        printf("result: %d\n", result);
    //    }
    //    sleep_ms(3000);
    //    val++;
    //}

    int zero = 0;
    int scratch = 0;
    int output = 0;

    int val[4];
    int mult[4];
    val[0] = 3;
    mult[0] = 13;
    val[1] = 5;
    mult[1] = 17;
    val[2] = 7;
    mult[2] = 19;
    val[3] = 11;
    mult[3] = 23;

    int ctrl_channel = dma_claim_unused_channel(true);
    int data_channel = dma_claim_unused_channel(true);

    // set DMA checksum type to sum
    dma_sniffer_enable(
        data_channel,
        0xf, // 32-bit sum
        false // only enable for specific transfers
    );

    uint32_t commands[1024];
    int count = 0;

    // each command writes:
    //   READ_ADDR
    //   WRITE_ADDR
    //   TRANS_COUNT
    //   CTRL_TRIG

    // data_channel chains back to control channel
    dma_channel_config c = dma_channel_get_default_config(data_channel);
    channel_config_set_chain_to(&c, ctrl_channel);
    channel_config_set_sniff_enable(&c, false);

    // copy zero to DMA checksum
    commands[count++] = (uint32_t)&zero;
    commands[count++] = (uint32_t)&(dma_hw->sniff_data);
    commands[count++] = 1;
    commands[count++] = c.ctrl;

    for( int sm = 0; sm < 4; sm++ ) {
        // copy val x to state machine x
        commands[count++] = (uint32_t)&val[sm];
        commands[count++] = (uint32_t)&pio->txf[sm];
        commands[count++] = 1;
        commands[count++] = c.ctrl;
        // copy mult x to state machine x
        commands[count++] = (uint32_t)&mult[sm];
        commands[count++] = (uint32_t)&pio->txf[sm];
        commands[count++] = 1;
        commands[count++] = c.ctrl;
    }

    // enable summing for shift and add
    channel_config_set_sniff_enable(&c, true);
    int mult_bits = 16;
    for( ;; ) {
        for( int sm = 0; sm < 4; sm++ ) {
            // copy state machine output to scratch with summing
            // wait for pio fifo ready
            channel_config_set_dreq(&c, pio_get_dreq(pio, sm, /*is_tx?*/false));
            commands[count++] = (uint32_t)&pio->rxf[sm];
            commands[count++] = (uint32_t)&scratch;
            commands[count++] = 1;
            commands[count++] = c.ctrl;
        }
        mult_bits--;
        if( mult_bits == 0 ) {
            break;
        } else {
            // copy DMA checksum to scratch with summing
            channel_config_set_dreq(&c, DREQ_FORCE); // no waiting
            commands[count++] = (uint32_t)&(dma_hw->sniff_data);
            commands[count++] = (uint32_t)&scratch;
            commands[count++] = 1;
            commands[count++] = c.ctrl;
        }
    }
    channel_config_set_sniff_enable(&c, false);
    channel_config_set_dreq(&c, DREQ_FORCE); // no waiting

    // copy DMA checksum to output (no summing)
    commands[count++] = (uint32_t)&(dma_hw->sniff_data);
    commands[count++] = (uint32_t)&output;
    commands[count++] = 1;
    commands[count++] = c.ctrl;

    // last command restarts the control channel
    // no chaining because the write triggers the next transfer
    channel_config_set_chain_to(&c, data_channel);
    uint32_t commandBufferStart = (uint32_t)&commands[0];
    commands[count++] = (uint32_t)&commandBufferStart;
    commands[count++] = (uint32_t)&dma_hw->ch[ctrl_channel].al3_read_addr_trig;
    commands[count++] = 1;
    commands[count++] = c.ctrl;

    // configure control channel
    channel_config_set_sniff_enable(&c, false);
    channel_config_set_read_increment(&c, true);
    channel_config_set_write_increment(&c, true);
    channel_config_set_chain_to(&c, ctrl_channel);
    channel_config_set_ring(
        &c,
        true, // write ring
        4 // 16 byte ring - 4 x 4 byte registers
    );

    dma_channel_configure(
        ctrl_channel,
        &c,
        &dma_hw->ch[data_channel].read_addr, // write all 4 control registers
        &commands[0], // read address
        4, // 1 command at a time
        true // start
    );

    for(;;) {
        printf("sum: %d\n", output);
        val[0]++;
        sleep_ms(1000);
    }
}