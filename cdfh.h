#ifndef _CDFH_H
#define _CDFH_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct CDFileHandle {
    int32_t  (*Seek)(struct CDFileHandle *handle, int32_t offset, int32_t whence);
    int32_t  (*Tell)(struct CDFileHandle *handle);
    int32_t  (*Read)(struct CDFileHandle *handle, void *ptr, int32_t size);
    uint8_t  (*Get)(struct CDFileHandle *handle);
    uint8_t  (*Eof)(struct CDFileHandle *handle);
    int32_t  offset; // start block of file
    int32_t  length; // length of file
    int32_t  block; // current block in buffer
    int32_t  pos; // current position in file
} CDFileHandle_t;

extern CDFileHandle_t *cd_handle_from_name(char *name);
extern CDFileHandle_t *cd_handle_from_offset(int32_t offset, int32_t length);
extern void delete_cd_handle(CDFileHandle_t *handle);

#ifdef __cplusplus
}
#endif

#endif
