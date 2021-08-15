#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/dma.h"

int main() {

    stdio_init_all();

    int zero = 0;
    int scratch = 0;
    int output = 0;
    int doubleOutput = 0;

    int val1 = 3;
    int mult1 = 13;
    int val2 = 5;
    int mult2 = 17;
    int val3 = 7;
    int mult3 = 19;
    int val4 = 11;
    int mult4 = 23;

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

    // CHx_READ_ADDR
    // CHx_WRITE_ADDR
    // CHx_TRANS_COUNT
    // CHx_CTRL_TRIG

    // summing dma channel
    // transfer count=1
    // chain to control channel
    // control dma channel

    dma_channel_config c = dma_channel_get_default_config(data_channel);
    channel_config_set_chain_to(&c, ctrl_channel);

    // steps:
    // copy zero to DMA checksum
    channel_config_set_sniff_enable(&c, false);
    commands[count++] = (uint32_t)&zero;
    commands[count++] = (uint32_t)&(dma_hw->sniff_data);
    commands[count++] = 1;
    commands[count++] = c.ctrl;
    // copy val1 to scratch with summing
    channel_config_set_sniff_enable(&c, true);
    commands[count++] = (uint32_t)&val1;
    commands[count++] = (uint32_t)&scratch;
    commands[count++] = 1;
    commands[count++] = c.ctrl;
    // copy val2 to scratch with summing
    channel_config_set_sniff_enable(&c, true);
    commands[count++] = (uint32_t)&val2;
    commands[count++] = (uint32_t)&scratch;
    commands[count++] = 1;
    commands[count++] = c.ctrl;
    // copy val3 to scratch with summing
    channel_config_set_sniff_enable(&c, true);
    commands[count++] = (uint32_t)&val3;
    commands[count++] = (uint32_t)&scratch;
    commands[count++] = 1;
    commands[count++] = c.ctrl;
    // copy val4 to scratch with summing
    channel_config_set_sniff_enable(&c, true);
    commands[count++] = (uint32_t)&val4;
    commands[count++] = (uint32_t)&scratch;
    commands[count++] = 1;
    commands[count++] = c.ctrl;
    // copy DMA checksum to output (no summing)
    channel_config_set_sniff_enable(&c, false);
    commands[count++] = (uint32_t)&(dma_hw->sniff_data);
    commands[count++] = (uint32_t)&output;
    commands[count++] = 1;
    commands[count++] = c.ctrl;
    // copy DMA checksum to scratch with summing
    channel_config_set_sniff_enable(&c, true);
    commands[count++] = (uint32_t)&(dma_hw->sniff_data);
    commands[count++] = (uint32_t)&scratch;
    commands[count++] = 1;
    commands[count++] = c.ctrl;
    // copy DMA checksum to doubleOutput (no summing)
    channel_config_set_sniff_enable(&c, false);
    commands[count++] = (uint32_t)&(dma_hw->sniff_data);
    commands[count++] = (uint32_t)&doubleOutput;
    commands[count++] = 1;
    commands[count++] = c.ctrl;
    // stop
    commands[count++] = 0;
    commands[count++] = 0;
    commands[count++] = 0;
    commands[count++] = 0;

    // configure control channel
    channel_config_set_sniff_enable(&c, false);
    channel_config_set_read_increment(&c, true);
    channel_config_set_write_increment(&c, true);
    channel_config_set_chain_to(&c, ctrl_channel);
    /*
    dma_channel_configure(
        ctrl_channel,
        &c,
        &dma_hw->ch[data_channel].read_addr, // write all 4 control registers
        &commands[0], // read address
        4, // 1 command at a time
        true // start
    );
    */

    dma_channel_configure(
        ctrl_channel,
        &c,
        &output,
        &val1, // read address
        1, // 1 command at a time
        true // start
    );

    for(;;) {
        printf("sniff: %d\n", dma_hw->sniff_data);
        printf("sum: %d\n", output);
        printf("doublesum: %d\n", doubleOutput);
        sleep_ms(1000);
    }
}