# bad-apple-pcspkr

Plays music through your PC speaker (the little motherboard buzzer, same one that does the BIOS beep) on modern Linux. Built this to run Bad Apple through pure beeps, but it'll play any melody you throw at it.

Two parts:

- **`pcspkr_player.c`** — the actual player. Feeds notes to the pcspkr device.
- **`midi_to_notes.py`** — turns a MIDI file into the note format the player reads.

I didn't hardcode the Bad Apple melody in here since that's someone else's composition — instead you grab a MIDI of it yourself and convert it, which also means this repo works for any song, not just one.
'Update:' I did actually upload the notes, but still you can still upload any midi file you want.

## Setup

```bash
sudo modprobe pcspkr
ls -l /dev/input/by-path/platform-pcspkr-event-spkr
```

If that file doesn't show up, or you're on a laptop with no physical buzzer, check `alsamixer` for a "Beep" channel and unmute it.

If you want to skip `sudo` every time, add a udev rule:

```
# /etc/udev/rules.d/99-pcspkr.rules
KERNEL=="event*", SUBSYSTEM=="input", ATTRS{name}=="PC Speaker", MODE="0660", GROUP="audio"
```
then add yourself to the `audio` group and reboot (or unplug/replug).

## Build

```bash
gcc -O2 -Wall -o pcspkr_player pcspkr_player.c
```

## Test it

```bash
sudo ./pcspkr_player test_scale.txt
```
Should hear a scale play. If not, go back to the setup step.

## Playing an actual song

```bash
pip install mido --break-system-packages

python3 midi_to_notes.py bad_apple.mid --list-tracks
python3 midi_to_notes.py bad_apple.mid notes.txt --track 2

sudo ./pcspkr_player notes.txt
```

The speaker can only play one note at a time, so pick the actual melody track, not a merge of the whole song, or it'll sound like garbage. `--speed` and `--min-ms` are there to tweak timing if it sounds off.
