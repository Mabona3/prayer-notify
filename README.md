# Prayer Notify
A command-line application that calculates Islamic prayer times and sends notifications when it's time to pray.

## Features
- Automatic prayer time calculations based on geographic location without network connection
- Real-time notifications for each prayer time
- Support for multiple calculation methods
- Configurable timezone, latitude, and longitude
- Customizable prayer time adjustments (angles and minutes)
- Multiple juristic methods for Asr calculation
- High latitude adjustment methods

## Installation

```bash
make release
sudo make install
systemctl --user start prayer-notify
```

if you want to auto start prayer-notify on startup
```bash
systemctl --user enable prayer-notify
```

Configure the location through ~/.config/prayer-notify and the method of calculation based on your location.
for more information use `prayer-notify -h`

## Waybar
The next prayer is updated on each prayer in /tmp/prayer-notify.json to make waybar read it just add a custom module as follows:

```json
    "custom/prayer": {
        "format": "{text}",
        "tooltip": true,
        "exec": "cat /tmp/prayer-notify.json",
        "return-type": "json",
        "interval": 1
    }
```
