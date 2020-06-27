// n64_mempak.h

#ifndef _N64_MEMPAK_h
#define _N64_MEMPAK_h

#define MEMPAK_SIZE 32768

typedef struct
{
    uint8_t dirty;
    uint8_t id;
    uint8_t *data;
    //Only if a 'virtual mempak'
    int8_t virtual_is_active;
    int8_t virtual_update_req;
    int8_t virtual_selected_row;
} n64_mempack;

void n64_mempack_read32(n64_mempack *mempack, uint16_t address, uint8_t *rx_buff);
void n64_mempack_write32(n64_mempack *mempack, uint16_t address, uint8_t *tx_buff);

#endif
