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

1) [Description](#description)
2) [Commands](#commands)
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

  </ol>

</details>

# Description

It is list of Continuous Controllers. For each function you can find:
1) Description of controller
2) Input
   1) Number of controller
   2) Type of input:
      1) Progression - You will send regular value
      2) Percent - You will send percent (convert 0-127 value to 0-100%)
      3) Range - You will send some value from enumeration of value range
      4) Switch - You will send "turn off" command if value < 63 and "turn on" if value >= 63
   3) Command have different actions on different channels, or not

# Commands

## Sensitivity (fib) 

### Description

Fibonacci parameter responsible for the note distribution curve.

### Input

Controlled by two variables:

1) Note Distance (Num: CC 22; Type: Percent)
2) First Value (Num CC 23; Type: Progression )


## Smoothness

### Description

The smoothness of the notes played, where 0 is an instant change in notes,
99 is a smooth change (notes change over time)

### Input
- Num: CC 3
- Type: Percent (from 0% to 99%)


## Scale

### Description

Scale played from the device

### Input
- Num: CC 24
- Type: Range

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
- Num: CC 9
- Type: Progression
- Different channels



## Min Velocity

### Description

Min pressing force for each channel. (From plant and from light sensor)

### Input
- Num: CC 25
- Type: Progression
- Different channels



## Humanize

### Description

Velocity randomization at a controlled interval for each channel.
(From plant and from light sensor)

### Input
- Num: CC 26
- Type: Switch
- Different channels



## Ultra sensitivity

### Description

Increases the sensitivity of generation from a plant

### Input
- Num: CC 15
- Type: Switch


## Same Note

### Description

Notes that are played only when changing notes with a customizable step,
where 1 - produces a note if the notes have changed by 1 note,
10 if there has been a shift by 10 notes

### Input
- Num: CC 20
- Type: Progression



## Note Off Percent

### Description

How many percent of the time the note will sound,
where 100 is the full sound before the note changes,
50 is half the time the note is played

### Input
- Num: CC 21
- Type: Percent



## Light Notes Range

### Description

Setting the range of notes played from the light sensor
(lower and upper limits are set)

### Input
- Num: CC 28
- Type: Progression



## Light Pitch Bend

### Description

Instead of playing notes on second channel,
light sensor will change pitch band on first channel (Plant)

### Input
- Num: CC 27
- Type: Progression



