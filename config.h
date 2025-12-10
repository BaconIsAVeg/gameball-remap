#ifndef CONFIG_H
#define CONFIG_H

#include <libevdev/libevdev.h>

// Default device path
// Change this to match your mouse device (find it with evtest or check
// /dev/input/)
#define DEFAULT_DEVICE "/dev/input/event5"

// Virtual device name (shows up in /proc/bus/input/devices)
#define VIRTUAL_DEVICE_NAME "Remapped Gameball Mouse"

// Direction constants for remapping
#define ANY 0   // Match any value/state (use for button→button remaps)
#define UP 1    // Positive value (scroll up/right)
#define DOWN -1 // Negative value (scroll down/left)

// Aliases for clarity
#define POSITIVE 1
#define NEGATIVE -1
#define RIGHT 1
#define LEFT -1

// Event remapping configuration
// Each entry maps: {from_code, from_direction, to_code}
//
// Available event codes (from linux/input-event-codes.h):
//
// Mouse buttons (BTN_*):
//   BTN_LEFT, BTN_RIGHT, BTN_MIDDLE - Standard mouse buttons
//   BTN_SIDE, BTN_EXTRA             - Side buttons (often buttons 4 & 5)
//   BTN_FORWARD, BTN_BACK           - Forward/back navigation buttons
//   BTN_TASK                        - Task button
//
// Scroll/movement events (REL_*):
//   REL_WHEEL   - Vertical scroll wheel
//   REL_HWHEEL  - Horizontal scroll wheel
//   REL_X, REL_Y - Mouse movement
//
// Full list:
// https://github.com/torvalds/linux/blob/master/include/uapi/linux/input-event-codes.h
//
struct event_remap {
  unsigned int from_code; // Source event code (REL_HWHEEL, BTN_EXTRA, etc.)
  int from_direction;     // Direction to match (UP/DOWN/ANY)
  unsigned int to_code;   // Target event code (BTN_SIDE, etc.)
};

// Define your event remappings here
static const struct event_remap event_remaps[] = {
    // Button swaps
    {BTN_RIGHT, ANY, BTN_EXTRA},
    {BTN_EXTRA, ANY, BTN_RIGHT},

    // Example: Horizontal scroll to buttons (uncomment to use)
    {REL_HWHEEL, RIGHT, BTN_EXTRA}, // Scroll right → BTN_FORWARD
    {REL_HWHEEL, LEFT, BTN_SIDE},   // Scroll left → BTN_BACK

    // Example: Vertical scroll to buttons (uncomment to use)
    // {REL_WHEEL, UP, BTN_SIDE},         // Scroll up → BTN_SIDE
    // {REL_WHEEL, DOWN, BTN_EXTRA},      // Scroll down → BTN_EXTRA
};

// Calculate number of remappings automatically
#define NUM_EVENT_REMAPS (sizeof(event_remaps) / sizeof(event_remaps[0]))

#endif // CONFIG_H
