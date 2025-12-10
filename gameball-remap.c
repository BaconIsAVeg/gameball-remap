#include <fcntl.h>
#include <libevdev/libevdev-uinput.h>
#include <libevdev/libevdev.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "config.h"

// Helper function to determine event type from event code
unsigned int get_event_type(unsigned int code) {
  // Check if it's a button (BTN_* or KEY_*)
  if (code >= BTN_MISC && code < KEY_MAX) {
    return EV_KEY;
  }
  // Check if it's a relative axis (REL_*)
  if (code < REL_MAX) {
    return EV_REL;
  }
  return 0; // Unknown type
}

// Helper function to check if direction matches
int direction_matches(int required_direction, int actual_value) {
  if (required_direction == 0) { // ANY (defined as 0 in config.h)
    return 1;                    // Match any value
  }
  if (required_direction > 0 && actual_value > 0) {
    return 1; // Positive/UP/RIGHT
  }
  if (required_direction < 0 && actual_value < 0) {
    return 1; // Negative/DOWN/LEFT
  }
  return 0;
}

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

  if (NUM_EVENT_REMAPS > 0) {
    printf("Event remappings (%zu total):\n", NUM_EVENT_REMAPS);
    for (i = 0; i < NUM_EVENT_REMAPS; i++) {
      unsigned int from_type = get_event_type(event_remaps[i].from_code);
      unsigned int to_type = get_event_type(event_remaps[i].to_code);
      const char *direction = "";
      if (event_remaps[i].from_direction > 0) {
        direction = " (UP/RIGHT)";
      } else if (event_remaps[i].from_direction < 0) {
        direction = " (DOWN/LEFT)";
      }
      printf("  %s%s -> %s\n",
             libevdev_event_code_get_name(from_type, event_remaps[i].from_code),
             direction,
             libevdev_event_code_get_name(to_type, event_remaps[i].to_code));
    }
  }

  // Event loop
  // Track which regular scroll events were remapped (to suppress high-res
  // variants)
  int remapped_wheel = 0;
  int remapped_hwheel = 0;

  while (1) {
    rc = libevdev_next_event(dev, LIBEVDEV_READ_FLAG_NORMAL, &ev);

    if (rc == 0) {
      int event_handled = 0;

      // Reset tracking on SYN_REPORT (marks end of event batch)
      if (ev.type == EV_SYN && ev.code == SYN_REPORT) {
        remapped_wheel = 0;
        remapped_hwheel = 0;
        libevdev_uinput_write_event(uidev, ev.type, ev.code, ev.value);
        continue;
      }

      // Check all event remappings
      for (i = 0; i < NUM_EVENT_REMAPS; i++) {
        // Check if event code matches
        if (ev.code == event_remaps[i].from_code) {
          // Check if direction matches
          if (direction_matches(event_remaps[i].from_direction, ev.value)) {
            unsigned int from_type = get_event_type(event_remaps[i].from_code);
            unsigned int to_type = get_event_type(event_remaps[i].to_code);

            // Handle REL -> KEY conversion (scroll to button)
            if (from_type == EV_REL && to_type == EV_KEY) {
              // Track which scroll events we've remapped
              if (ev.code == REL_WHEEL) {
                remapped_wheel = 1;
              } else if (ev.code == REL_HWHEEL) {
                remapped_hwheel = 1;
              }

              // Generate button press
              libevdev_uinput_write_event(uidev, EV_KEY,
                                          event_remaps[i].to_code, 1);
              libevdev_uinput_write_event(uidev, EV_SYN, SYN_REPORT, 0);
              // Generate button release
              libevdev_uinput_write_event(uidev, EV_KEY,
                                          event_remaps[i].to_code, 0);
              libevdev_uinput_write_event(uidev, EV_SYN, SYN_REPORT, 0);
            }
            // Handle KEY -> KEY conversion (button to button)
            else if (from_type == EV_KEY && to_type == EV_KEY) {
              libevdev_uinput_write_event(uidev, EV_KEY,
                                          event_remaps[i].to_code, ev.value);
            }
            // Handle other conversions (pass through with new code)
            else {
              libevdev_uinput_write_event(uidev, to_type,
                                          event_remaps[i].to_code, ev.value);
            }

            event_handled = 1;
            break;
          }
        }
      }

      // Suppress high-res variants if we remapped the regular scroll event
      if (!event_handled) {
        if ((ev.code == REL_WHEEL_HI_RES && remapped_wheel) ||
            (ev.code == REL_HWHEEL_HI_RES && remapped_hwheel)) {
          // Suppress this event (don't pass it through)
          event_handled = 1;
        }
      }

      // If event wasn't remapped or suppressed, pass it through unchanged
      if (!event_handled) {
        libevdev_uinput_write_event(uidev, ev.type, ev.code, ev.value);
      }
    }
  }

  libevdev_uinput_destroy(uidev);
  libevdev_free(dev);
  close(fd);
  return 0;
}
