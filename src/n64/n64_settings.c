/* MIT License
 * 
 * Copyright (c) [2020] [Ryan Wendland]
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include <stdint.h>
#include "n64_settings.h"
#include "n64_wrapper.h"
#include "printf.h"

n64_settings *_settings = NULL;

void n64_settings_init(n64_settings *settings)
{
    const uint8_t _MAGIC = 0x64; //FIXME. CHECKSUM WILL BE BETTER
    if (settings->magic != _MAGIC)
    {
        printf("%s not found or invalid, setting to default\n", SETTINGS_FILENAME);
        for (int i = 0; i < MAX_CONTROLLERS; i++)
        {
            settings->magic = _MAGIC;
            strcpy(settings->default_tpak_rom[i], "");
            settings->deadzone[i] = 1;
            settings->sensitivity[i] = 3;
            settings->snap_axis[i] = 1;
        }
    }
    _settings = settings;
}

n64_settings *n64_settings_get()
{
    return _settings;
}
