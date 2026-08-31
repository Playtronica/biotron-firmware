# Biotron 1.9.4 calibration cue — internal physical result

Status: **BLOCKED — internal experiment only**  
Tested: 2026-08-31 on `rev.A08-Fibonacci`, USB serial
`E6639C754B87AF2A`.

## What passed

- Contact-free update from firmware 1.9.3 to 1.9.4.
- Flash verification and post-flash version/topology verification.
- Both MIDI inputs and outputs remained available.
- Web-triggered recalibration completed without a USB reset or stuck note.
- Host sanitizer/optimized suites and LED-OFF/LED-ON ARM builds passed.

## What failed the human gate

The eight-note calibration phrase was too loud relative to normal plant play
and was not expressive enough. It must not be shared with the team or users as
an accepted cue.

## Next iteration

Keep calibration as real MIDI for DAWs and external instruments. Before the
next physical flash, audition several lower-velocity phrases against measured
normal-play velocity, select one with a clear beginning and resolution, and
retain balanced Note On/Off plus legacy Web detection.

No merge, release or production deployment is approved by this result.
