/* author - nocferno */

#ifndef __UCODE_GEN_H__
#define __UCODE_GEN_H__

#include "intel_microcode.h"

struct __device_pathnames {
    const char *    _fw_path;           /* firmware path            */
    const char *    _ucode_load_path;   /* late loader path         */
    const char *    _msr_dev_path;      /* MSR kernel module path   */

    bool            _init;         /* init flag                */
};

/* type definitions */
typedef struct __device_pathnames       dev_path;
typedef struct __device_pathnames *     dev_path_ptr;


/* microcode generator main */
extern int ucode_gen_main(dev_path_ptr);

#endif

