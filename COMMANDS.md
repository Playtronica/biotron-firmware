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
        <li><a href="#change-octave">Change octave</a></li>
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

Function that create longer distance between notes. Working by fiboncci algorithm

Has two parameters:
1) First is influence on main algorithm (from 0 to 100%). 
2) Second is first value of fibonacci (from 0 to 100, but it converts in 0 to 1).

### Change smoothness
(Function Number Bite = 2)

It changes the smoothness of note changes.

Has one parameter.
The parameter is responsible for smoothness (from 0 to 99%)

### Change octave
(Function Number Bite = 3)

Changes octave.

Has one parameter.
The parameter is responsible for octave. (from 0 to 11)

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

### Stop all notes
(CC command number = 120)

Stop all notes.
Don't need any parameters/
