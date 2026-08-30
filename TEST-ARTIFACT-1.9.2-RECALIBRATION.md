# Biotron 1.9.2 recalibration test artifact

Status: **LAB ONLY — do not release or send to customers.**

- Production source commit: `6361629`
- Matching Web beta commit: `635592d`
- Firmware identity: `1.9.2`
- Settings compatibility ID: `1765723554` (unchanged)
- Pico SDK: `2.3.0`
- ARM GCC: `13.2.1`
- UF2 SHA-256: `ba7c15a53dced73aaf96b85cf0132de8973006e1bf9ca1dde829215eee22f508`
- ELF SHA-256: `ada9123707ceef5e133ee111ef66ffb56d5b703a8a251c14f5bc01225a0d3737`
- Durable local copy: `~/ProjectData/playtronica-firmware/biotron/2026-08-30-6361629-recalibration/`

The Web button sends `F0 14 0D 7B <nonce> F7`. Firmware reports waiting,
measuring and ready on both logical cables. The command must preserve every
setting, avoid flash/USB/BOOT, stop active notes and restart only the transient
plant baseline.

Before any release, verify on a company Biotron: ten recalibration cycles,
settings equality before/after, internal and MIDI Clock modes, both logical
ports, unstable/disconnected clips, no stuck notes, USB remains enumerated and
the matching Web beta reaches `ready` only after the device reports state `3`.

## Mac physical result — 2026-08-30

Exact artifact `ba7c15a…` was flashed with picotool verification after a full
4 MiB backup of the company Biotron running lab firmware `1.9.1`.

- version `1.9.2` replied through both logical outputs and both inputs;
- 10/10 normal-mode cycles alternated Port 1/Port 2 and reported ordered
  `waiting → measuring → ready` on both inputs;
- cycle duration was 10.062–10.101 s (median 10.083 s);
- 2/2 MIDI Clock-mode cycles passed, with exactly 50 realtime messages parsed
  on each cable and no flash, USB, malformed, overflow, reject or recovery
  counter delta;
- the 4 KiB settings sector was byte-identical before and after all ten normal
  cycles: SHA-256 `01318193c809ae176c883733661e437e47c54837cecc31bc16f9b54ff93a29fd`;
- final reboot returned `1.9.2` through both ports.

Evidence is under
`~/ProjectData/playtronica-firmware/biotron/2026-08-30-6361629-recalibration/physical/`.

The matching Web beta passes its nonce/progress, build and persistent-offline
Chrome automation. Actual Web UI → physical-device progress display is still a
manual one-click gate because the browser-control surface could not claim the
localhost test page. Unstable/disconnected clips and human observation of
stuck-note/audio behaviour also remain physical gates.

## Release blocker discovered by the physical upgrade

Lab firmware `1.9.1` provisionally used vendor SysEx ID `123` for settings
readback. This `1.9.2` branch was based on the diagnostics head and reuses ID
`123` for recalibration, so it does not contain that readback feature. The two
branches must be reconciled with distinct owner-approved IDs before merge or
release. The successful recalibration test does not override this protocol
collision.
