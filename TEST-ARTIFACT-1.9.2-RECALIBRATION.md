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
