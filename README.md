# waybar-bits
Collection of small programs for use with [waybar](https://github.com/Alexays/Waybar) and [sway](https://github.com/swaywm/sway). The programs are written in C and are intended to be used in custom modules for waybar.

## Installation
### Dependencies
```
json-c
```

### Building from source
```bash
git clone git@github.com:MStarvik/waybar-bits.git
cd waybar-bits
make
```

## Usage
### network-traffic
```bash
network-traffic <interface>...
```
Periodically prints the amount of data sent and received on the given interfaces to stdout. The output is formatted as JSON and can be used directly by waybar.
```json
"custom/network_traffic": {
    "exec": "/path/to/network-traffic wlan0",
    "return-type": "json",
    "tooltip": false,
}
```

### keyboard-layout
```bash
keyboard-layout
```
Prints the current keyboard layout to stdout. Uses `swaymsg -t subscribe` to listen for changes in the keyboard layout and prints the new layout to stdout when it changes. The output is formatted as JSON and can be used directly by waybar.
```json
"custom/keyboard_layout": {
    "exec": "/path/to/keyboard-layout",
    "return-type": "json",
    "tooltip": false,
    "format": " {}",
    "on-click": "swaymsg input type:keyboard xkb_switch_layout next"
},
```