#include <fcntl.h>
#include <libevdev/libevdev-uinput.h>
#include <libevdev/libevdev.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "config.h"

int main() {
  struct libevdev *dev;
  struct libevdev_uinput *uidev;
  struct input_event ev;
  int fd, rc;
  unsigned int i;

  // Open the real mouse device
  fd = open(DEFAULT_DEVICE, O_RDONLY);
  if (fd < 0) {
    perror("Failed to open mouse device");
    fprintf(stderr,
            "Tip: Find your mouse device with 'evtest' or check /dev/input/\n");
    return 1;
  }

  rc = libevdev_new_from_fd(fd, &dev);
  if (rc < 0) {
    fprintf(stderr, "Failed to initialize libevdev: %s\n", strerror(-rc));
    close(fd);
    return 1;
  }

  // Set custom name for the virtual device
  libevdev_set_name(dev, VIRTUAL_DEVICE_NAME);

  // Create virtual device (duplicate of real one)
  int uifd = open("/dev/uinput", O_RDWR);
  if (uifd < 0) {
    perror("Failed to open uinput");
    libevdev_free(dev);
    close(fd);
    return 1;
  }

  rc = libevdev_uinput_create_from_device(dev, uifd, &uidev);
  if (rc < 0) {
    fprintf(stderr, "Failed to create uinput device: %s\n", strerror(-rc));
    close(uifd);
    libevdev_free(dev);
    close(fd);
    return 1;
  }

  // Grab the original device to prevent double events
  rc = libevdev_grab(dev, LIBEVDEV_GRAB);
  if (rc < 0) {
    fprintf(stderr, "Warning: Failed to grab device: %s\n", strerror(-rc));
  }

  printf("Mouse remapping active\n");
  printf("Source device: %s\n", DEFAULT_DEVICE);
  printf("Virtual device: %s (%s)\n", libevdev_uinput_get_devnode(uidev),
         libevdev_get_name(dev));

  if (NUM_BUTTON_REMAPS > 0) {
    printf("Button remappings (%zu total):\n", NUM_BUTTON_REMAPS);
    for (i = 0; i < NUM_BUTTON_REMAPS; i++) {
      printf("  %s -> %s\n",
             libevdev_event_code_get_name(EV_KEY, button_remaps[i].from),
             libevdev_event_code_get_name(EV_KEY, button_remaps[i].to));
    }
  }

  // Event loop
  while (1) {
    rc = libevdev_next_event(dev, LIBEVDEV_READ_FLAG_NORMAL, &ev);

    if (rc == 0) {
      // Check if this is a key event that needs button remapping
      if (ev.type == EV_KEY) {
        for (i = 0; i < NUM_BUTTON_REMAPS; i++) {
          if (ev.code == button_remaps[i].from) {
            ev.code = button_remaps[i].to;
            break;
          }
        }
      }

      // Write the (possibly modified) event to virtual device
      libevdev_uinput_write_event(uidev, ev.type, ev.code, ev.value);
    }
  }

  libevdev_uinput_destroy(uidev);
  libevdev_free(dev);
  close(fd);
  return 0;
}
