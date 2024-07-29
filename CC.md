<!-- PROJECT LOGO -->
<br />
<div align="center">
  <a href="https://github.com/Playtronica">
    <img src="static/logo.png" alt="Logo" width="100">
  </a>

<h3 align="center">Biotron Continuous Controllers</h3>

  <p align="center">
    BODY IS AN INSTRUMENT
    <br />
    <a href="https://github.com/Playtronica/Biotron"><strong>Explore the docs »</strong></a>
    <br />
    <br />
  </p>
</div>

<!-- TABLE OF CONTENTS -->

<details >

  <summary>Table of Continuous Controllers</summary>

  <ol>

[//]: # (1&#41; [Description]&#40;#description&#41;)
1)[Commands](#commands)
   1) [Sensitivity (fib)](#sensitivity--fib-)
   2) [Smoothness](#smoothness)
   3) [Scale](#scale)
   4) [Max Velocity](#max-velocity)
   5) [Min Velocity](#min-velocity)
   6) [Humanize](#humanize)
   7) [Ultra sensitivity](#ultra-sensitivity)
   8) [Same Note](#same-note)
   9) [Note Off Percent](#note-off-percent)
   10) [Light Notes Range](#light-notes-range)
   11) [Light Pitch Bend](#light-pitch-bend)
   12) [Performance Mode](#performance-mode) 

  </ol>

</details>

# Description

It is list of Continuous Controllers. For each function you can find:

# Commands

## Sensitivity (fib) 

### Description

Fibonacci parameter responsible for the note distribution curve.

### Input

Controlled by two variables:

1) Note Distance (Num: CC 22) - Influence on main algorithm (from 0 to 100%).
2) First Value (Num CC 23) - First value of Fibonacci

Then bigger value, then distance between notes would be bigger


## Smoothness

### Description

The smoothness of the notes played, where 0 is an instant change in notes,
99 is a smooth change (notes change over time)

### Input
Num: CC 3

The value is responsible for smoothness (from 0 to 99%)


## Scale

### Description

Scale played from the device

### Input
- Num: CC 24


| Range Value | Scale      |
|-------------|------------|
| 0 - 10      | MAJOR      |
| 10 - 21     | MINOR      |
| 21 - 31     | CHROM      |
| 31 - 42     | DORIAN     |
| 42 - 52     | MIXOLYDIAN |
| 52 - 63     | LYDIAN     |
| 63 - 73     | WHOLETONE  |
| 73 - 84     | MINBLUES   |
| 84 - 95     | MAJBLUES   |
| 95 - 105    | MINPEN     |
| 105 - 116   | MAJPEN     |
| 116 - 127   | DIMINISHED |



## Max Velocity

### Description

Max pressing force for each channel. (From plant and from light sensor)

### Input
Num: CC 9

Change max velocity of channel 



## Min Velocity

### Description

Min pressing force for each channel. (From plant and from light sensor)

### Input
Num: CC 25

Change min velocity of channel


## Humanize

### Description

Velocity randomization at a controlled interval for each channel.
(From plant and from light sensor)

### Input
Num: CC 26

Turn off humanize if value < 63, else turn it on


## Ultra sensitivity

### Description

Increases the sensitivity of generation from a plant

### Input
Num: CC 15

Decrease sensitivity of device if value < 63, else increase it


## Same Note

### Description

Notes that are played only when changing notes with a customizable step,
where 1 - produces a note if the notes have changed by 1 note,
10 if there has been a shift by 10 notes

### Input
Num: CC 20

Then bigger value, then bigger step between notes. When value is equal 0,
then all notes are playing



## Note Off Percent

### Description

How many percent of the time the note will sound,
where 100 is the full sound before the note changes,
50 is half the time the note is played

### Input
Num: CC 21

The value is responsible for percent of note duration (from 0 to 100%)




## Light Notes Range

### Description

Setting the range of notes played from the light sensor
(lower and upper limits are set)

### Input
Num: CC 28

Then bigger value, then bigger playable range of light notes


## Light Pitch Bend

### Description

Instead of playing notes on second channel,
light sensor will change pitch band on first channel (Plant)

### Input
Num: CC 27

Turn off this mode if value < 63, else turn it on


## Performance Mode

### Description

Mode for better manual control of the device

### Input

Num: CC: 30

Turn off this mode if value < 63, else turn it on

