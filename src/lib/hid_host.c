// Copyright 2020, Ryan Wendland, usb64
// SPDX-License-Identifier: MIT

#include "tusb_option.h"

#if (TUSB_OPT_HOST_ENABLED && CFG_TUH_HID)

#include "host/usbh.h"
#include "host/usbh_classdriver.h"
#include "class/hid/hid_host.h"

// Invoked when device with hid interface is mounted
// Report descriptor is also available for use. tuh_hid_parse_report_descriptor()
// can be used to parse common/simple enough descriptor.
// Note: if report descriptor length > CFG_TUH_ENUMERATION_BUFSIZE, it will be skipped
// therefore report_desc = NULL, desc_len = 0
void tuh_hid_mount_cb(uint8_t dev_addr, uint8_t instance, uint8_t const* desc_report, uint16_t desc_len)
{
  uint16_t vid, pid;
  tuh_vid_pid_get(dev_addr, &vid, &pid);

  TU_LOG2("HID device address = %d, instance = %d is mounted\r\n", dev_addr, instance);
  TU_LOG2("VID = %04x, PID = %04x\r\n", vid, pid);
}

// Invoked when received report from device via interrupt endpoint
void tuh_hid_report_received_cb(uint8_t dev_addr, uint8_t instance, uint8_t const* report, uint16_t len)
{
  // continue to request to receive report
  if ( !tuh_hid_receive_report(dev_addr, instance) )
  {
    TU_LOG2("Error: cannot request to receive report\r\n");
  }
}

#endif