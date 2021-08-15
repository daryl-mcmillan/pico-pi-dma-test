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

    int commands[1024];
    int count = 0;

    // CHx_READ_ADDR
    // CHx_WRITE_ADDR
    // CHx_TRANS_COUNT
    // CHx_CTRL_TRIG
    
    // summing dma channel
    // SNIFF_CTRL
    // SNIFF_DATA
    // transfer count=1
    // chain to control channel
    // control dma channel
    // steps:
    // copy zero to DMA checksum
    // copy val1 to scratch with summing
    // copy val2 to scratch with summing
    // copy val3 to scratch with summing
    // copy val4 to scratch with summing
    // copy DMA checksum to output (no summing)
    // copy DMA checksum to scratch with summing
    // copy DMA checksum to doubleOutput (no summing)
    // stop

    for(;;) {
        printf("sniff: %d\n", dma_hw->sniff_data);
        printf("sum: %d\n", output);
        printf("doublesum: %d\n", doubleOutput);
        sleep_ms(1000);
    }
}