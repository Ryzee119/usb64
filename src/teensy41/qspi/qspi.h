/*
 * Portions of this software were developed at http://www.pjrc.com/
 * Those portions licensed under MIT License Agreement, (the "License");
 * You may not use these files except in compliance with the License.
 * You may obtain a copy of the License at: http://opensource.org/licenses/MIT
 * Unless required by applicable law or agreed to in writing, software distributed
 * under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions
 * and limitations under the License.
*/
#ifndef EXT_FLASH_H_
#define EXT_FLASH_H_

#include <stdint.h>

#ifdef __cplusplus
extern "C"{
#endif
void qspi_init(uint32_t *sector_size, uint32_t *flash_size);
void qspi_get_flash_properties(uint32_t *sector_size, uint32_t *flash_size);
uint8_t qspi_read(uint32_t addr, uint32_t size, uint8_t *dst);
uint8_t qspi_write(uint32_t addr, uint32_t size, uint8_t *src);
uint8_t qspi_erase(uint32_t addr, uint32_t size);
void qspi_erase_chip();
#ifdef __cplusplus
}
#endif
#endif /* EXT_FLASH_H_ */