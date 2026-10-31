# SATB Checker

A C++ program that proofreads four-part harmony (Soprano, Alto, Tenor, Bass) and tells you which chord broke which voice-leading rule. My passion project, mixing music theory and coding :)

## How to use it

Write a score file:

```
key: C major
chords: I V I
S: E5 D5 C5
A: C5 B4 G4
T: G4 G4 E4
B: C3 G2 C3
```

Build and run (Windows users can use the included `satb.exe`):

```
g++ -std=c++17 -o satb SATB04.cpp
./satb cleanSATB.txt
```

Example output (`parallelsSATB.txt`):

```
chord 2 (ii) Parallel octaves A-T
chord 2 (ii) Parallel fifths  A-B
chord 2 (ii) Parallel fifths  T-B
3 issue(s)
```

## What it checks

Range, spacing, voice crossing and overlap, notes not in the chord, unraised leading tone in minor, doubled leading tone, leading tone resolution, augmented seconds, and parallel fifths/octaves.

## How I built it

I wrote it in chunks (`SATB_1c` to `SATB5_c`) because one giant file felt overwhelming. Then I merged them into `SATB.cpp`, which had tons of typos and errors, and spent days fixing them through `SATB01`, `SATB03`, and finally `SATB04.cpp`.

Hardest parts:
- **`find` vs `substr`:** `find` gives a position (a number), `substr` actually cuts out the text. I kept mixing them up while parsing lines like `S: E5 D5 C5`.
- **Order matters:** my parser used `Key`, `Chord`, and `Pitch` before they were defined, so I had to reorder everything.
- **Showing errors:** with so many nested `if`s and loops, I made a `flag()` function that saves each problem to a list, then `main()` prints it all at the end.

## Limitations and next steps

Fancy chords (like secondary dominants) get skipped with a warning, and inversions are only partly supported. Next I want to add more rules, MIDI playback, and suggested fixes.
