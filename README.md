# Gameball Remap

For years I've been using a [Gameball](https://www.gamingtrackball.com/products/gameball-standard-edition-black-ball) as my daily pointing device, however the default buttons on the right side never suited me.

I ran [Input Remapper](https://github.com/sezanzeb/input-remapper) to remap those buttons, however a GTK app on top of Python bindings wrapping libevdev was a little heavy for my needs, and I'd occasionally see it eating over a GiB of memory, plus occasionally dealing with Python dependency conflicts.

It also didn't work on FreeBSD, and I'd like to give that a spin again.

So, here we go. A simple C program to map mouse buttons. It doesn't handle remapping the scroll wheel, but I can live without that.

## What it does

This creates a virtual input device that mirrors your mouse, but with button remappings applied. The original device gets "grabbed" so you don't get duplicate events, and the virtual device sends the remapped buttons to your system. Simple as that.

## Building

You'll need `libevdev` installed:

**Linux:**

```bash
# Debian/Ubuntu
sudo apt install libevdev-dev

# Arch
sudo pacman -S libevdev

# Fedora
sudo dnf install libevdev-devel
```

**FreeBSD:**

```bash
pkg install libevdev
```

Then build it:

```bash
make
```

That's it. If you want to install it system-wide:

```bash
sudo make install
```

## Configuration

Edit `config.h` before building. You'll need to:

1. **Set your mouse device path** - Find yours with `evtest` or by poking around `/dev/input/`. Mine's at `/dev/input/event5` but yours is probably different.

2. **Configure your button mappings** - The default config swaps `BTN_RIGHT` and `BTN_EXTRA` (right-click and the side button). Change the `button_remaps` array to whatever you want. There's a list of available button codes in the comments.

Example config:

```c
static const struct button_remap button_remaps[] = {
    {BTN_RIGHT, BTN_EXTRA},    // Right button → Side button
    {BTN_EXTRA, BTN_RIGHT},    // Side button → Right button
    {BTN_MIDDLE, BTN_FORWARD}, // Middle → Forward
};
```

## Running

After building, run it with:

```bash
sudo ./gameball-remap
```

Or if you don't want to use `sudo` every time, add yourself to the `input` group on Linux:

```bash
sudo usermod -aG input $USER
```

Then log out and back in. On FreeBSD you'll need to load the evdev module first:

```bash
kldload evdev
```

You'll see output showing your remappings when it starts up. Leave it running, and your buttons will be remapped. Kill it with Ctrl+C when you're done. I start mine as a systemd user service in `graphical-session.target.wants`.

## Why not just use Input Remapper?

Input Remapper is great if you need all the features, but I didn't. This is under 100 lines of C, compiles to a ~20KB binary, uses minimal memory, works on FreeBSD, and does exactly what I need. That's good enough for me.

## License

MIT - see LICENSE file
