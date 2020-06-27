#ifndef EXT_FLASH_H_
#define EXT_FLASH_H_

#include <stdint.h>

#ifdef __cplusplus
extern "C"{
#endif
void qspi_init(uint32_t *block_size, uint32_t *flash_size);
void qspi_get_flash_properties(uint32_t *block_size, uint32_t *flash_size);
uint8_t qspi_read(uint32_t addr, uint32_t size, uint8_t *dst);
uint8_t qspi_write(uint32_t addr, uint32_t size, uint8_t *src);
uint8_t qspi_erase(uint32_t addr, uint32_t size);
void qspi_erase_chip();
#ifdef __cplusplus
}
#endif
#endif /* EXT_FLASH_H_ */