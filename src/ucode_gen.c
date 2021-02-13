/* Attempts to generate an Intel microcode blob */
/* author - nocferno */

#include "../include/ucode_gen.h"
#include "../include/rand_32.h"

#include <sys/random.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <sys/time.h>

#include <unistd.h>
#include <fcntl.h>
#include <cpuid.h>
#include <time.h>

#include <errno.h>
#include <string.h>
#include <stdio.h>


static struct __microcode_path {
    char *          _ucode_file_path;
    uint16_t        _ucode_path_len;

    char            _ucode_name[0x8];
    const char *    _ucode_path;

    bool            _init;
} __ucode_path;

typedef struct __microcode_path    ucode_path;
typedef struct __microcode_path *  ucode_path_ptr;


/* forward declarations */
static void _set_header_version(intel_microcode_ptr);
static void _set_loader_revision(intel_microcode_ptr);
static void _set_total_size(intel_microcode_ptr);

static bool _sanitize_dev_path_(dev_path_ptr);
static bool _get_proc_sig(intel_microcode_ptr);
static bool _build_microcode_data(intel_microcode_ptr);

static int _gen_microcode_data(intel_microcode_ptr);
static int _gen_update_revision(intel_microcode_ptr);
static int _gen_update_time(intel_microcode_ptr);
static int _gen_proc_flags(intel_microcode_ptr,
                           const char *);
static int _gen_checksum(intel_microcode_ptr);
static int _gen_data_size(intel_microcode_ptr);
static int _gen_microcode_file_path(intel_microcode_ptr,
                                    ucode_path_ptr);
static bool _build_ucode_name(uint8_t,
                              uint8_t,
                              uint8_t,
                              ucode_path_ptr);
static char _hex2ascii(uint8_t);


int ucode_gen_main(dev_path_ptr args) {
    int ret = 0x1;
    if(!args)
        return ret;

    if(_sanitize_dev_path_(args))
        ret ^= ret;

    if(!ret) {
        intel_microcode _data = { 0x0 };
        ucode_path _path = { 0x0 };
        
        if(!(_gen_proc_flags(&_data, args->_msr_dev_path)) &&
           _get_proc_sig(&_data)) {
            _path._ucode_path = args->_fw_path;
            ret = _gen_microcode_file_path(&_data, &_path);
        }

        if(!ret) {
            int _flags = (O_CREAT | O_RDWR);
            mode_t _access_mode = (S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
            unlink(_path._ucode_file_path);

            _data._map_fd = open(_path._ucode_file_path, _flags, _access_mode);

            if(_data._map_fd <= 0x0)
                ret = 0x2;

            if(!_gen_data_size(&_data)) {
                _set_total_size(&_data);
                _data._map_len = _data.total_size;
            }

            if(!_data._map_len) {
                if(!ret)
                    close(_data._map_fd);
                ret = 0x3;
            }

            if(!ret) {
                int _map_prot = (PROT_WRITE | PROT_READ),
                    _map_flags = (MAP_PRIVATE | MAP_ANONYMOUS);

                _data._map_ptr = mmap(NULL, _data._map_len, 
                                      _map_prot, _map_flags,
                                      -1, 0x0);

                if(_data._map_ptr != MAP_FAILED) {
                    if(!_build_microcode_data(&_data)) {
                        ret = 0x5;
                    } else {
                        if(write(_data._map_fd, 
                                 _data._map_ptr,
                                 _data._map_len) != _data._map_len)
                            ret = 0x6;
                    }
                } else {
                    fprintf(stdout, "%s\n", strerror(errno));
                    ret = 0x4;
                }

                if(!ret)
                    munmap(_data._map_ptr, _data._map_len);

                close(_data._map_fd);
            }
        }

        if(_path._init)
            munmap(_path._ucode_file_path, 
                   _path._ucode_path_len);

    } else {
        ret  = 0x5;
    }

    return ret;
}


static bool _sanitize_dev_path_(dev_path_ptr args) {
    bool ret = false;

    if(!args)
        return ret;

    if(args->_fw_path && 
       args->_ucode_load_path && 
       args->_msr_dev_path)

        if(args->_init)
            ret = args->_init;

    return ret;
}


static void _set_header_version(intel_microcode_ptr primary) {
    if(primary)
        primary->header_ver = 0x1;
    return;
}


static void _set_loader_revision(intel_microcode_ptr primary) {
    if(primary)
        primary->loader_rev = 0x1;
    return;
}


static void _set_total_size(intel_microcode_ptr primary) {
    if(primary)
        if(primary->data_size > 0x0)
            primary->total_size = 
                    (primary->data_size + 0x30);

    return;
}


static bool _get_proc_sig(intel_microcode_ptr primary) {
    if(primary) {
        uint32_t eax, ebx, ecx, edx;
        if(__get_cpuid(0x1, &eax, &ebx, &ecx, &edx))
            primary->proc_sig = eax;
    }

    return (primary->proc_sig) > 0x0 ? true : false;
}


static int _gen_update_revision(intel_microcode_ptr primary) {
    int ret = 0x1;
    if(primary) {
        primary->update_rev = 
            get_rand_32(0x1000000, 0xF0000000);
        ret ^= ret;
    }

    return ret;
}

static int _gen_update_time(intel_microcode_ptr primary) {
    int ret = 0x1;      
    if(primary) {
        time_t _current = time(0x0);
        struct tm *_time_data = localtime(&_current);

        if(_time_data) {
            uint8_t _date_hex[0x8] = {0x0}, i = 0x0;

            _time_data->tm_year += 0x76C;
            _time_data->tm_mon++;

            /* year */
            _date_hex[0x3] = ((_time_data->tm_year)   / 0x3E8),
            _date_hex[0x2] = ((_time_data->tm_year)   / 0x64) - 
                              (_date_hex[0x3] * 0xA),
            _date_hex[0x1] = ((_time_data->tm_year)   / 0xA) - 
                              (_date_hex[0x3] * 0x64),
            _date_hex[0x0] = ((_time_data->tm_year) - 
                              (_date_hex[0x3] * 0x3E8));

            if(_date_hex[0x0] > 0xA)
                _date_hex[0x0] -= 0xA;

            /* day */
            _date_hex[0x5] = 0x0;
            if(_time_data->tm_mday >= 0xA) {
                _date_hex[0x5] = ((_time_data->tm_mday) / 0xA),
                _date_hex[0x4] = ((_time_data->tm_mday) - (_date_hex[0x5] * 0xA));
            } else {
                _date_hex[0x4] = _time_data->tm_mday;
            }

            /* month */
            _date_hex[0x7] = 0x0;
            if(_time_data->tm_mon >= 0xA) {
                _date_hex[0x7] = ((_time_data->tm_mon) / 0xA),
                _date_hex[0x6] = ((_time_data->tm_mon) - (_date_hex[0x7] * 0xA));
            } else {
                _date_hex[0x6] = _time_data->tm_mon;
            }

            primary->update_date ^= primary->update_date;

            /* assemble date */
            for(i ^= i; i < 0x8; i++)
                primary->update_date |= _date_hex[i] << (i * 0x4);

            if(primary->update_date > 0x0)
                ret ^= ret;
        }
    }

    return ret;
}

static int _gen_proc_flags(intel_microcode_ptr primary,
                           const char *msr_device) {
    char _msr_data[0x8] = {0x0};    int msr_fd = 0x0, ret = 0x1;

    if(!primary || !msr_device)
        return ret;

    msr_fd =
        open(msr_device, O_RDONLY);

    if(msr_fd > 0x0) {
        size_t bytes_read = 
                pread(msr_fd, _msr_data, 0x8, 0x17);

        if(bytes_read == 0x8) {
            primary->proc_flags =
                (_msr_data[0x6] & 0x8)     |   /* bit 51 */
                (_msr_data[0x6] & 0x10)    |   /* bit 52 */
                (_msr_data[0x6] & 0x20);       /* bit 53 */

            ret ^= ret;
        }

        close(msr_fd);
    }
    
    return ret;
}


static int _gen_checksum(intel_microcode_ptr primary) {
    int ret = 0x1;

    if(primary) {
        uint32_t *microcode_ptr = primary->_map_ptr;
        uint32_t i = 0x0, _sum = 0x0;

        for(i ^= i; i < primary->total_size / 0x4; i++)
            _sum += microcode_ptr[i];

        primary->checksum = (0xFFFFFFFF - _sum) + 0x1;
        ret ^= ret;
    }

    return ret;
}


static int _gen_data_size(intel_microcode_ptr primary) {
    int ret = 0x1;
    if(primary) {
        do {
            primary->data_size = get_rand_32(0x7D0, 0x5000);
        } while((primary->data_size % 0x4) != 0x0);
        ret = (primary->data_size) ? 0x0 : 0x2;
    }

    return ret;
}


static int _gen_microcode_file_path(intel_microcode_ptr primary, 
                                    ucode_path_ptr path) {
    uint32_t stepping_id = 0x0,
             model_id = 0x0,
             family_id = 0x0,
             ex_model_id = 0x0,
             ex_family_id = 0x0;
    int ret = 0x1;

    if(!primary || !path)
        return ret;

    stepping_id =
        (primary->proc_sig & 0xF),
    model_id =
        (primary->proc_sig & 0xF0) >> 0x4,
    family_id =
        (primary->proc_sig & 0xF00) >> 0x8,
    ex_model_id =
        (primary->proc_sig & 0xF0000) >> 0x10,
    ex_family_id =
        (primary->proc_sig & 0xFF00000) >> 0x14;

    if(primary->proc_sig > 0x0) {
        if(family_id == 0x6 || family_id == 0xF)
            model_id +=
                (ex_model_id << 0x4); 

        if(family_id == 0xF)
            family_id +=
                ex_family_id;

        /* family_id, model_id, stepping_id  -- (8-byte file name) */
        ret ^= ret;
    } else {
        ret++;
    }

    if(!ret) {
        if(_build_ucode_name((family_id & 0xFF),
                             (model_id & 0xFF),
                             (stepping_id & 0xFF),
                             path)) {

            int map_prot = (PROT_READ | PROT_WRITE),
                map_flag = (MAP_ANONYMOUS | MAP_PRIVATE),
                i = 0x0, n = 0x0;
            path->_ucode_path_len ^= path->_ucode_path_len;

            for(i ^= i; path->_ucode_path[i] != '\0'; i++);
            path->_ucode_path_len = i + 0x8; /* path + name */

            path->_ucode_file_path =
                mmap(NULL, path->_ucode_path_len, 
                           map_prot, map_flag,
                     -1, 0x0);

            if(path->_ucode_file_path != MAP_FAILED) {
                char *_ptr = (char *)(path->_ucode_path);
                for(i ^= i, n = i; i < path->_ucode_path_len; i++, n++) {
                    if((path->_ucode_path_len - i) == 0x8) {
                        _ptr = path->_ucode_name;
                        n ^= n;
                    }

                    path->_ucode_file_path[i] = _ptr[n];
                }

                path->_init = true;
            }
        }        
    }

    

    return ret;
}

static bool _build_ucode_name(uint8_t family_id,
                              uint8_t model_id,
                              uint8_t stepping_id,
                              ucode_path_ptr path) {
    bool ret = false;

    if(path) {
        path->_ucode_name[0x0] = _hex2ascii((family_id & 0xF0) >> 0x4),
        path->_ucode_name[0x1] = _hex2ascii(family_id & 0xF),
        path->_ucode_name[0x2] = 0x2D,

        path->_ucode_name[0x3] = _hex2ascii((model_id & 0xF0) >> 0x4),
        path->_ucode_name[0x4] = _hex2ascii(model_id & 0xF),
        path->_ucode_name[0x5] = 0x2D,

        path->_ucode_name[0x6] = _hex2ascii((stepping_id & 0xF0) >> 0x4),
        path->_ucode_name[0x7] = _hex2ascii(stepping_id & 0xF);

        ret = true;
    }

    return ret;
}


static char _hex2ascii(uint8_t num) {
    return (num >= 0xA && num <= 0xF) ? 
            (char)(num + 0x31) : (char)(num + 0x30);
}


static int _gen_microcode_data(intel_microcode_ptr primary) {
    int ret = 0x1;

    if(!primary)
        return ret;

    if(primary->data_size > 0x0) {
        int _prot = (PROT_WRITE | PROT_READ),
            _flag = (MAP_PRIVATE | MAP_ANONYMOUS);

        primary->fw_data =
                mmap(NULL, primary->data_size, _prot, _flag, -1, 0x0);

        /* 
            NOTE: A microcode encrypted data header spans the first 48-bytes
                  relative to offset of the encrypted data itself.

                from primary->fw_data + 0x30 --> primary->fw_data + 0x60
                lays a slab of bytes, that account for data that will
                ultimately be used by the CPU/BIOS to continue with the
                update process.

                let us assume that:
                   [[ uint32_t *encrypted_data_hdr = (primary->fw_data + 0x30): ]]
                then:

                     (microcode encrypted data header -- 8-bytes )
                    encrypted_data_hdr[0x0] = 0x0,          4-bytes
                    encrypted_data_hdr[0x1] = 0xA1000000,   4-bytes
                    encrypted_data_hdr[0x2] = 0x1,          2-bytes
                    encrypted_data_hdr[0x3] = 0x2,          2-bytes
                       (end of microcode encrypted data header)

                    encrypted_data_hdr[0x4] = primary->update_rev,  4-bytes
                    encrypted_data_hdr[0x5] = 0x0,                  4-bytes
                    encrypted_data_hdr[0x6] = 0x0,                  4-bytes
                    encrypted_data_hdr[0x7] = primary->update_date  4-bytes

                    (possible checksum of all data relative to
                     (primary->fw_data + 0x60) ?..)
                    encrypted_data_hdr[0x8] = ??                    4-bytes

                    (loader revision/update header?)
                    encrypted_data_hdr[0x9] = 0x1                   4-bytes
                    encrypted_data_hdr[0xA] = primary->proc_sig     4-bytes
                    encrypted_data_hdr[0xB] =   
        */

        if(primary->fw_data != MAP_FAILED) {
            uint32_t size_ctr = 0x0;
            uint8_t rand_len = 0x40;

            for(size_ctr ^= size_ctr;
                size_ctr < primary->data_size; 
                size_ctr += rand_len)
                if(getrandom(primary->fw_data, 
                             rand_len, 
                             GRND_NONBLOCK) == rand_len)
                    primary->fw_data += rand_len;

            primary->fw_data -= primary->data_size;
            ret ^= ret;
        }
    }

    return ret;
}


static bool _build_microcode_data(intel_microcode_ptr primary) {
    bool ret = false;
    if(!primary)
        return ret;

    _set_header_version(primary);
    _set_loader_revision(primary);

    if(!_gen_update_revision(primary) &&
       !_gen_update_time(primary)) {
        uint32_t *file_map = primary->_map_ptr;

        /* write all members of intel_microcode_ptr to primary->_map_ptr */
        if(file_map) {
            unsigned char *file_map_char = primary->_map_ptr;
            uint32_t i = 0x0, n = 0x0;

            file_map[0x0] = primary->header_ver,
            file_map[0x1] = primary->update_rev,
            file_map[0x2] = primary->update_date,
            file_map[0x3] = primary->proc_sig,
            file_map[0x5] = primary->loader_rev,
            file_map[0x6] = primary->proc_flags,
            file_map[0x7] = primary->data_size,
            file_map[0x8] = primary->total_size;
            
            if(!_gen_microcode_data(primary)) {
                for(i ^= i, n = 0x30; i < primary->data_size; i++, n++)
                    file_map_char[n] = primary->fw_data[i];

                if(!_gen_checksum(primary)) {
                    file_map[0x4] = primary->checksum;
        
                    if(!munmap(primary->fw_data, primary->data_size))
                        primary->fw_data = NULL;

                    ret = true;
                }
            }
        }
    }

    return ret;
}



