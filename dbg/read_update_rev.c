
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <cpuid.h>

#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include <stdio.h>

#include <errno.h>


static const char *_msr_dev_path = {"/dev/cpu/0/msr"};

int main(int argc, const char **argv) {
    int msr_dev_fd = open(_msr_dev_path, O_RDWR | O_NONBLOCK),
        ret_main = 0x1;
    
    if(msr_dev_fd > 0x0) {
        uint8_t msr_offset = 0x8B;  /* IA32_BIOS_SIGN_ID */
        char reg_pair[0x8] = {0x0};
        
        bool io_ret = 
            (pwrite(msr_dev_fd, reg_pair, 0x8, msr_offset) == 0x8);

        if(io_ret) {
            uint32_t eax, ebx, ecx, edx;

            fprintf(stdout, "0x%x bytes from MSR 0x%x\n",
                             pread(msr_dev_fd, reg_pair, 0x8, msr_offset),
                             msr_offset);
            fprintf(stdout, "0x%x -- 0x%x\n", reg_pair[0x0], reg_pair[0x1]);

            if(__get_cpuid(0x1, &eax, &ebx, &ecx, &edx)) {
                uint32_t i = 0x0, mask = 0xFF;

                for(i ^= i; i < 0x4; i++)
                    reg_pair[i] = (eax >> (i * 0x8)) & mask;
                for(i = 0x4; i < 0x8; i++)
                    reg_pair[i] = (edx >> (i * 0x8)) & mask;

                fprintf(stdout, "\n0x%x bytes to MSR 0x%x\n",
                                pwrite(msr_dev_fd, reg_pair, 0x8, msr_offset),
                                msr_offset);

                pread(msr_dev_fd, reg_pair, 0x8, msr_offset),
                fprintf(stdout, "0x%x -- 0x%x\n", reg_pair[0x0], reg_pair[0x1]);
            }
        }

        ret_main ^= ret_main;
        close(msr_dev_fd);
    }

    return ret_main;
}

/* author - nocferno */
