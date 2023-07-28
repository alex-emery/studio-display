#include <fcntl.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

int main(int argc, char **argv) {
  int fd;
  int res = 0;
  char buf[7];

  char *device = "/dev/hidraw0";

  // default brightness, goes from 60000 to 4000
  long brightness = 60000;
  if (argc > 1)
    device = argv[1];

  if (argc > 2)
    brightness = strtol(argv[2], NULL, 10);

  char brightnessLE[2];
  brightnessLE[0] = brightness & 0xff;
  brightnessLE[1] = brightness >> 8;

  /* Open the Device with non-blocking reads. In real life,
     don't use a hard coded path; use libudev instead. */
  fd = open(device, O_RDWR | O_NONBLOCK);

  if (fd < 0) {
    perror("Unable to open device");
    return 1;
  }

  memset(buf, 0x0, sizeof(buf));

  buf[0] = 0x01;
  buf[1] = brightnessLE[0];
  buf[2] = brightnessLE[1];

  res = write(fd, buf, 7);
  if (res < 0) {
    printf("Error: %d\n", errno);
    perror("write");
  } else {
    printf("write() wrote %d bytes\n", res);
  }

  close(fd);
  return 0;
}
