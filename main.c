/* author - nocferno */

#include "include/ucode_gen.h"


int main(int argc, const char **argv) {
    dev_path current = {0x0};

    /* set the appropriate pathnames */
    current._fw_path = 
        "/usr/lib/firmware/intel-ucode/";
    current._ucode_load_path = 
        "/sys/devices/system/cpu/microcode/reload";
    current._msr_dev_path = 
        "/dev/cpu/0/msr";

    current._init = true;
    return ucode_gen_main(&current);
}

