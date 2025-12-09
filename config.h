#ifndef CONFIG_H
#define CONFIG_H

#include <libevdev/libevdev.h>

// Platform-specific default device paths
// You can override this by changing the path below
#ifdef __linux__
#define DEFAULT_DEVICE "/dev/input/event5"
#elif defined(__FreeBSD__)
#define DEFAULT_DEVICE "/dev/input/event0"
#endif

// Virtual device name (shows up in /proc/bus/input/devices)
#define VIRTUAL_DEVICE_NAME "Remapped Gameball Mouse"

// Button remapping configuration
// Each entry maps: {from_button, to_button}
//
// Available mouse button codes (from linux/input-event-codes.h):
//   BTN_LEFT, BTN_RIGHT, BTN_MIDDLE - Standard mouse buttons
//   BTN_SIDE, BTN_EXTRA             - Side buttons (often buttons 4 & 5)
//   BTN_FORWARD, BTN_BACK           - Forward/back navigation buttons
//   BTN_TASK                        - Task button
//
// Full list:
// https://github.com/torvalds/linux/blob/master/include/uapi/linux/input-event-codes.h
// (Search for "BTN_" to see all available button codes)
//
struct button_remap {
  unsigned int from;
  unsigned int to;
};

// Define your button remappings here
static const struct button_remap button_remaps[] = {
    {BTN_RIGHT, BTN_EXTRA},
    {BTN_EXTRA, BTN_RIGHT},
    // Add more mappings here, for example:
    // {BTN_MIDDLE, BTN_FORWARD}, // Map middle button to forward
    // {BTN_FORWARD, BTN_BACK},   // Swap forward and back buttons
};

// Calculate number of remappings automatically
#define NUM_BUTTON_REMAPS (sizeof(button_remaps) / sizeof(button_remaps[0]))

#endif // CONFIG_H
