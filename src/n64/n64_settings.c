#include <stdint.h>
#include "n64_mempak.h"
#include "n64_virtualpak.h"
#include "n64_settings.h"
#include "n64_transferpak_gbcarts.h"
#include "n64_controller.h"
#include "n64_wrapper.h"

#define SETTINGS_FILENAME "set.dat"

static void n64_settings_init(n64_settings *settings)
{
    settings->default_tpak_rom = 0;

    settings->deadzone[0] = 1;
    settings->deadzone[1] = 1;

    settings->octa[0] = 1;
    settings->octa[1] = 1;

    settings->sensitivity[0] = 5;
    settings->sensitivity[1] = 5;

    settings->snap_axis[0] = 1;
    settings->snap_axis[1] = 1;
}

//FIXME - Should be abstracted with n64_wrapper
void n64_settings_read(n64_settings *settings)
{
}

//FIXME - Should be abstracted with n64_wrapper
void n64_settings_write(n64_settings *settings)
{
}
