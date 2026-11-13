/*
** protocol.h -- Saves the protocol structures for file sending
*/

#ifndef PROTOCOL_H
#define PROTOCOL_H

#include <stdint.h>
#define MAX_NAME 200
#define CHUNK 64000 // Bytes

struct header {
    char     name[MAX_NAME];
    uint8_t  name_size; // 1 bytes
    uint64_t file_size; // 8 bytes
};

struct file {
    struct header header;
    unsigned char chunk[CHUNK]; // information
};

#endif
