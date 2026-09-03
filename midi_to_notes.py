import sys
import argparse

try:
    import mido
except ImportError:
    sys.exit("Please install mido first:  pip install mido --break-system-packages")


def note_to_freq(note):
    return 440.0 * (2.0 ** ((note - 69) / 12.0))


def list_tracks(mid):
    for i, track in enumerate(mid.tracks):
        n_notes = sum(1 for m in track if m.type == "note_on" and m.velocity > 0)
        name = ""
        for m in track:
            if m.type == "track_name":
                name = m.name
                break
        print(f"  track {i}: {n_notes} notes  {('(' + name + ')') if name else ''}")


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("midi_in")
    ap.add_argument("notes_out", nargs="?")
    ap.add_argument("--track", type=int, default=None)
    ap.add_argument("--speed", type=float, default=1.0)
    ap.add_argument("--min-ms", type=int, default=30)
    ap.add_argument("--list-tracks", action="store_true")
    args = ap.parse_args()

    mid = mido.MidiFile(args.midi_in)

    if args.list_tracks:
        print(f"{args.midi_in}: {len(mid.tracks)} tracks")
        list_tracks(mid)
        return

    if not args.notes_out:
        sys.exit("notes_out is required unless --list-tracks is given")

    events = []

    for i, track in enumerate(mid.tracks):
        if args.track is not None and i != args.track:
            continue
        t = 0.0
        cur_tempo = 500000
        for msg in track:
            t += mido.tick2second(msg.time, mid.ticks_per_beat, cur_tempo)
            if msg.type == "set_tempo":
                cur_tempo = msg.tempo
            elif msg.type == "note_on" and msg.velocity > 0:
                events.append((t, "on", msg.note))
            elif msg.type == "note_off" or (msg.type == "note_on" and msg.velocity == 0):
                events.append((t, "off", msg.note))

    if not events:
        sys.exit("No note events found. Try a different --track (see --list-tracks).")

    events.sort(key=lambda e: e[0])

    notes = []
    active = None
    active_start = 0.0
    last_t = 0.0

    for t, typ, note in events:
        if typ == "on":
            gap = t - last_t
            if active is None and gap > 0.01:
                notes.append((0, gap))
            active = note
            active_start = t
        elif typ == "off" and note == active:
            dur = t - active_start
            notes.append((note, dur))
            active = None
        last_t = t

    with open(args.notes_out, "w") as f:
        for note, dur in notes:
            dur_ms = max(args.min_ms, int((dur * 1000) / args.speed))
            freq = 0 if note == 0 else round(note_to_freq(note))
            f.write(f"{freq} {dur_ms}\n")

    print(f"Wrote {len(notes)} notes to {args.notes_out}")


if __name__ == "__main__":
    main()
