<!-- PROJECT LOGO -->
<br />
<div align="center">
  <a href="https://github.com/Playtronica">
    <img src="images/logo.png" alt="Logo" width="100">
  </a>

<h3 align="center">Biotron Commands</h3>

  <p align="center">
    BODY IS AN INSTRUMENT
    <br />
    <a href="https://github.com/Playtronica/Biotron"><strong>Explore the docs »</strong></a>
    <br />
    <br />
  </p>
</div>

<!-- TABLE OF CONTENTS -->

<details>

  <summary>Table of Contents</summary>

  <ol>
    <li>
      <a href="#introduce-info">Introduce Info</a>
    </li>
    <li>
      <a href="#byte-commands">Byte commands</a>
      <ul>
        <li><a href="#change-bpm">Change BPM</a></li>
        <li><a href="#change-sensitivity">Change sensitivity</a></li>
        <li><a href="#change-smoothness">Change smoothness</a></li>
        <li><a href="#change-scale">Change scale</a></li>
        <li><a href="#change-velocity">Change velocity</a></li>
      </ul>
    </li>
    <li>
      <a href="#cc-commands">CC commands</a>
      <ul>
        <li><a href="#smoothness-change">Smoothness change</a></li>
        <li><a href="#velocity-change">Velocity change</a></li>
        <li><a href="#stop-all-notes">Stop all notes</a></li>
      </ul>
    </li>
  </ol>

</details>

## Introduce Info

Biotron has custom MIDI commands. Part of them can be controlled by CC commands,
but most of them can be used only by byte commands. 

## Byte commands

Byte commands it is a raw bytes. All bytes commands start with byte F0 and end on byte F7.
For separate user's commands from system's, every command contains second key byte, which is equal to B.
The third bite is number function.
And the other bytes (up to a byte F7) it is parameters. All parameters described for each function described lower.
Short syntax you can find lower in the table.


| Number of Byte  | 1          | 2        | 3                    | 4               | 5              | ..  | 6        |
|-----------------|------------|----------|----------------------|-----------------|----------------|-----|----------|
| Possible Values | F0         | B        | 0-3                  | 0-7F            | 0-7F           | ..  | F7       |
| Info            | Start Byte | Key Byte | Function Number Byte | Parameters Byte | Parametrs Byte | ..  | End Byte |


### Change BPM
(Function Number Bite = 0)

Change BPM of Biotron.

Can have infinite parameters.
Sum of the parameters -> bps. **(Warning. With very big value, device can crash)**.

### Change sensitivity
(Function Number Bite = 1)

Function that create longer distance between notes. Working by fibonacci algorithm

Has two parameters:
1) First is influence on main algorithm (from 0 to 100%). 
2) Second is first value of fibonacci (from 0 to 100, but it converts in 0 to 1).

### Change smoothness
(Function Number Bite = 2)

It changes the smoothness of note changes.

Has one parameter.
The parameter is responsible for smoothness (from 0 to 99%)

### Change scale
(Function Number Bite = 3)

Changes scale.

Has one parameter.
The parameter is responsible for scale. (from 0 to 11)

| Byte | Octave     |
|------|------------|
| 0    | MAJOR      |
| 1    | MINOR      |
| 2    | CHROM      |
| 3    | DORIAN     |
| 4    | MIXOLYDIAN |
| 5    | LYDIAN     |
| 6    | WHOLETONE  |
| 7    | MINBLUES   |
| 8    | MAJBLUES   |
| 9    | MINPEN     |
| A    | MAJPEN     |
| B    | DIMINISHED |


### Change velocity
(Function Number Bite = 4)

Change velocity of one channel

Has two parameters:
1) Channel (if equal 1 change plant velocity, else if equal 2 change light velocity)
2) Velocity (from 0 to 127)

### Return default settings
(Function Number Bite = 5)

Return initial settings.

Don't need any parameters

### Log current settings
(Function Number Bite = 6)

Display all settings in serial.

Don't need any parameters

### Change BPM of the light sensor
(Function Number Bite = 7)

Change speed of the second channel (light sensor). Depends on BPM of first channel.

Has one parameter:
1) Light BPM - how many Plant notes must play, before new Light note would be played. (from 0 to 127)

### Set randomness of notes
(Functional Number Byte = 8)

Controls the game mode. If it is enabled, random is enabled, otherwise is disabled.
Has one parameter:
1) 0 if it is disabled
2) 1 if it is enabled


### Disable same notes play
(Function Number Byte = 9)

Controls the game mode. If it is enabled, all the notes will be played, otherwise the same notes will be skipped.
Has 1 switch parameter
1) 0 if it is disabled
2) 1 if it is enabled

### Set note off control
(Function Number Byte = 10)

Controls time to note off plant note. If it is lower 100, last notes will be turned off faster, then next note.
Has 1 parameter

1) Percent of current plant BPM. (from 1 to 100)

## CC commands
Its more simplified type of commands than bytes commands. You can send it by terminal, 
by more often they send automatically by sequencer. All of them have the same syntax.
Choose channel, choose CC command and send parameter (from 0 to 127).


### Smoothness change
(CC command number = 3)

It changes the smoothness of note changes.

The parameter is responsible for smoothness (from 0% to 99% (from 0 to 127))

### Velocity change
(CC command number = 9)

Change velocity of current channel

The parameter is responsible for velocity (from 0 to 127)

### Change BPM of the light sensor (only for second channel)
(CC command number = 14)
Change speed of the second channel (light sensor). Depends on BPM of first channel.

The parameter is Light BPM - how many Plant notes must play, before new Light note would be played. (from 0 to 127)

### Set randomness of notes
(CC command number = 15)

Controls the game mode. If it is enabled, random is enabled, otherwise is disabled.

The parameter is Switch:
1) (0 - 62) - turn off
2) (63 - 127) - turn on

### Disable same notes play
(CC command number = 20)

Controls the game mode. If it is enabled, all the notes will be played, otherwise the same notes will be skipped.
The parameter is Switch:
1) (0 - 62) - turn off
2) (63 - 127) - turn on

### Set note off control
(CC command number = 21)

Controls time to note off plant note. If it is lower 100, last notes will be turned off faster, then next note.
The parameter is percent of current plant BPM.

### Stop all notes
(CC command number = 120)

Stop all notes.
Don't need any parameters





