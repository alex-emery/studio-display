#include <errno.h>
#include <fcntl.h>
#include <libudev.h>
#include <linux/hidraw.h>
#include <linux/input.h>
#include <linux/types.h>
#include <locale.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define APPLE_STUDIO_DISPLAY_VENDOR_ID "05ac"
#define APPLE_STUDIO_DISPLAY_PRODUCT_ID "1114"

char correct_report[4] = {0x05, 0x80, 0x09, 0x01};
int checkReport(char *incoming) {
  for (int i = 0; i < 4; i++) {
    if (incoming[i] != correct_report[i]) {
      return 0;
    }
  }

  return 1;
}

int main(int argc, char **argv) {

  long brightness = 60000;
  if (argc > 1) {
    brightness = strtol(argv[1], NULL, 10);
  }

  char brightnessLE[2];
  brightnessLE[0] = brightness & 0xff;
  brightnessLE[1] = brightness >> 8;

  struct udev *udev;
  struct udev_enumerate *enumerate;
  struct udev_list_entry *devices, *dev_list_entry;
  struct udev_device *dev;

  /* Create the udev object */
  udev = udev_new();
  if (!udev) {
    printf("Can't create udev\n");
    exit(1);
  }

  enumerate = udev_enumerate_new(udev);
  udev_enumerate_add_match_subsystem(enumerate, "hidraw");
  udev_enumerate_scan_devices(enumerate);
  devices = udev_enumerate_get_list_entry(enumerate);

  udev_list_entry_foreach(dev_list_entry, devices) {
    const char *path;

    path = udev_list_entry_get_name(dev_list_entry);
    dev = udev_device_new_from_syspath(udev, path);

    const char *devicePath = udev_device_get_devnode(dev);

    dev =
        udev_device_get_parent_with_subsystem_devtype(dev, "usb", "usb_device");
    if (!dev) {
      printf("Unable to find parent usb device.");
      exit(1);
    }

    const char *idVendor = udev_device_get_sysattr_value(dev, "idVendor");
    if (strcmp(idVendor, APPLE_STUDIO_DISPLAY_VENDOR_ID)) {
      udev_device_unref(dev);
      continue;
    }

    const char *idProduct = udev_device_get_sysattr_value(dev, "idProduct");
    if (strcmp(idProduct, APPLE_STUDIO_DISPLAY_PRODUCT_ID)) {
      udev_device_unref(dev);
      continue;
    }

    udev_device_unref(dev);

    int fd = open(devicePath, O_RDWR | O_NONBLOCK);

    if (fd < 0) {
      perror("Unable to open device");
      return 1;
    }

    int i, res, desc_size = 0;
    struct hidraw_report_descriptor rpt_desc;
    struct hidraw_devinfo info;
    char buf[7];
    memset(buf, 0x0, sizeof(buf));

    /* Get Report Descriptor Size */
    res = ioctl(fd, HIDIOCGRDESCSIZE, &desc_size);
    if (res < 0) {
      close(fd);
      perror("HIDIOCGRDESCSIZE");
      continue;
    }

    rpt_desc.size = desc_size;
    res = ioctl(fd, HIDIOCGRDESC, &rpt_desc);
    if (res < 0) {
      perror("HIDIOCGRDESC");
      close(fd);
      continue;
    }

    if (checkReport(rpt_desc.value) == 0) {
      close(fd);
      continue;
    }

    memset(buf, 0x0, sizeof(buf));
    buf[0] = 0x1;
    buf[1] = brightnessLE[0];
    buf[2] = brightnessLE[1];
    res = write(fd, buf, 7);
    if (res < 0) {
      printf("Error: %d\n", errno);
    }
    close(fd);
    break;
  }

  /* Free the enumerator object */
  udev_enumerate_unref(enumerate);

  udev_unref(udev);
}
