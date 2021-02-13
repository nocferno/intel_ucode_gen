/* author - nocferno */

#ifndef __INTEL_MICROCODE_H__
#define __INTEL_MICROCODE_H__

#include <stdbool.h>
#include <stdint.h>


/*      Since offsets are 0x4 bytes apart, you can use that value as a pivot from one component to the next.
    This allows for implementation of a smaller, more efficient algorithm to retrieve all components in the
    microcode update file. Once 0x20 is reached, add 0x10 to get a pointer to the actual encrypted data.

    offsets relative to the base of the microcode update

    INTEL_HEADER_VER    = 0x0,      header version     
    INTEL_UPDATE_REV    = 0x4,      update revision 
    INTEL_DATE          = 0x8,      date of creation 
    INTEL_PROC_SIG      = 0xC,      processor signature
    INTEL_CHECKSUM      = 0x10,     checksum number 
    INTEL_LOADER_REV    = 0x14,     loader revision
    INTEL_PROC_FLAGS    = 0x18,     processor flags
    INTEL_DATA_SIZE     = 0x1C,     data segment size
    INTEL_TOTAL_SIZE    = 0x20,     overall size
    INTEL_DATA          = 0x30,     update data
*/

/* Intel microcode data structure */
struct __intel_microcode_fw {
    uint8_t     header_ver;
    uint8_t     loader_rev;    
    uint16_t    proc_flags;
 
    uint32_t    update_rev;     
    uint32_t    update_date;
    uint32_t    proc_sig;      
    uint32_t    checksum;      
    uint32_t    data_size;      
    uint32_t    total_size;     
    

    unsigned char *     fw_data;

    void *              _map_ptr;
    int                 _map_fd;
    uint32_t            _map_len;     
    bool                _init;         
};

typedef struct __intel_microcode_fw     intel_microcode;
typedef struct __intel_microcode_fw *   intel_microcode_ptr;


#endif

