/* A basic PRNG based on getrandom() */ 
/* author - nocferno */

#include "../include/rand_32.h"
#include <stdio.h>

static uint8_t get_rand_total_32(uint32_t, uint32_t);
static uint8_t get_byte_total_32(uint32_t);
static uint8_t get_bit_width_32(uint32_t);


/* generate a 32-bit random number in the range [x,y] */
uint32_t get_rand_32(uint32_t x, uint32_t y) {
    uint32_t z_rand = 0x0;

    if(x < y) {
        /* get bit width for both x and y */
        uint32_t x_bits = get_bit_width_32(x),
                 y_bits = get_bit_width_32(y);

        if((x_bits > 0x0) && (y_bits > 0x0)) {
            uint8_t byte_len = get_rand_total_32(x_bits, y_bits);

            /* random number generation */
            if((byte_len > 0x0) && (byte_len <= 0x8)) {
                while((z_rand < x) || (z_rand > y))
                    getrandom(&z_rand, byte_len, GRND_NONBLOCK);
            }
        }
    }

    /* return to random number to caller */
    return z_rand;
}


/* get random byte total of x and y */
static uint8_t get_rand_total_32(uint32_t x_bits, uint32_t y_bits) {
    uint8_t byte_total = 0x0;

    if((x_bits > 0x0) && (x_bits <= 0x20) && 
       (y_bits > 0x0) && (y_bits <= 0x20)) {

        if(y_bits >= x_bits) {
            uint8_t x_bytes = 0x0, y_bytes = 0x0, i = 0x0;
            char rand_data[0x20] = {0x0};
            bool rand_loop = true;

            x_bytes = get_byte_total_32(x_bits), 
            y_bytes = get_byte_total_32(y_bits);

            /* equivalent byte lengths can't be randomized (x == y) */
            if(x_bytes == y_bytes) {
                byte_total = x_bytes;
                rand_loop = false;
            }

            /* 
                get a byte length that sits in the range of [x,y] 

                ex: if x is 2-bytes, and y is 6-bytes, then byte length
                    can be [2,3,4,5,6]
            */
            while(rand_loop) {
                if(getrandom(rand_data, 0x20, GRND_NONBLOCK) == 0x20) {
                    for(i ^= i; i < 0x20; i++) {
                        byte_total = rand_data[i] & 0xF;

                        if((byte_total >= x_bytes) && (byte_total <= y_bytes)) {
                            rand_loop = false;
                            i = 0x20;
                        }
                    }
                }
            }
        }
    }

    return byte_total;
}

/* get total bytes, given bit width */
static uint8_t get_byte_total_32(uint32_t bits) {
    uint8_t bytes = 0x0;

    if( (bits > 0x0) && (bits <= 0x20) ) {
        bytes++;

        if( (bits > 0x8) && (bits <= 0x10) )
           bytes++;
        else if( (bits > 0x10) && (bits <= 0x18) )
           bytes += 0x2;
        else if( (bits > 0x18) && (bits <= 0x20) )
           bytes += 0x3;
    }

    return bytes;
}


/* get the width of a 32-bit number */
static uint8_t get_bit_width_32(uint32_t x) {
    uint8_t bits = 0x0, i = 0x0;

    if(x > 0x0) {
        uint32_t bit_mask_1 = 0x1,
                 bit_mask_2 = 0x2,
                 bit_mask_4 = 0x4,
                 bit_mask_8 = 0x8;

        for(i ^= i; i < 0x8; i++) {
            if( (x & bit_mask_1) ||
                (x & bit_mask_2) ||
                (x & bit_mask_4) ||
                (x & bit_mask_8) ) {
                bits = (i + 0x1);
            }

            bit_mask_1 <<= 0x4,
            bit_mask_2 <<= 0x4,
            bit_mask_4 <<= 0x4,
            bit_mask_8 <<= 0x4;
        }

        bits *= 0x4;
    }

    return bits;
}

