<!-- PROJECT LOGO -->
<br />
<div align="center">
  <a href="https://github.com/Playtronica">
    <img src="static/logo.png" alt="Logo" width="100">
  </a>

<h3 align="center">Biotron Settings Description</h3>

  <p align="center">
    BODY IS AN INSTRUMENT
    <br />
    <a href="https://github.com/Playtronica/Biotron"><strong>Explore the docs »</strong></a>
    <br />
    <br />
  </p>
</div>

<!-- TABLE OF CONTENTS -->



# Table of Settings


| WebMIDI Name          | Function                                                      | Sys Ex Number | CC Number |
|-----------------------|---------------------------------------------------------------|:--------------|-----------|
| The Beat (Plant)      | [set_plant_bpm](#set_plant_bpm)                               | 0             | -         |
| The Beat (Light)      | [set_light_bpm](#set_light_bpm)                               | 9             | -         |
| Step Size             | [set_fib_power](#set_fib_power)                               | 1             | 22        |
| Wake-Up               | [set_fib_first](#set_fib_first)                               | 2             | 23        |
| Delay                 | [set_filter](#set_filter)                                     | 3             | 3         |
| Scale                 | [set_scale](#set_scale)                                       | 4             | 24        |
| Note velocity (Plant) | [set_max_plant_vel](#set_max_plant_vel)                       | 5             | 9         |
| Note velocity (Light) | [set_max_light_vel](#set_max_light_vel)                       | 6             | 9         |
| Note velocity (Plant) | [set_min_plant_vel](#set_min_plant_vel)                       | 15            | 25        |
| Note velocity (Light) | [set_min_light_vel](#set_min_light_vel)                       | 17            | 25        |
| Humanize (Plant)      | [set_random_plant_vel](#set_random_plant_vel)                 | 16            | 26        |
| Humanize (Light)      | [set_random_light_vel](#set_random_light_vel)                 | 18            | 26        |
| Mute (Plant)          | [set_mute_plant_vel](#set_mute_plant_vel)                     | 22            | 31        |
| Mute (Light)          | [set_mute_light_vel](#set_mute_light_vel)                     | 23            | 31        |
| -                     | [set_default](#set_default)                                   | 7             | -         |
| Ultra sensitivity     | [set_random_note](#set_random_note)                           | 10            | 15        |
| Note Repeat (Plant)   | [set_same_note_plant](#set_same_note_plant)                   | 11            | 20        |
| Note Repeat (Light)   | [set_same_note_light](#set_same_note_light)                   | 24            | 20        |
| Note Hold             | [set_note_off_percent](#set_note_off_percent)                 | 12            | 21        |
| Range                 | [set_light_range](#set_light_range)                           | 13            | 28        |
| Pitch bend            | [set_light_pitch_mode](#set_light_pitch_mode)                 | 19            | 27        |
| Manual control        | [set_stuck_mode](#set_stuck_mode)                             | 21            | 30        |
| Home Note             | [set_middle_plant_note](#set_middle_plant_note)               | 25            | 85        |
| Swing Note            | [set_swing_first_note_percent](#set_swing_first_note_percent) | 26            | 86        |
| MIDI channel          | [set_channel](#set_channel)                                   | 127           | -         |


# Description

It is list of Continuous Controllers and Sys Ex Command.

All sys ex commands have the same format

`240 20 13 x [y1, y2, .., yn] 247`

Where x - sys ex number of command, y - parameters

# Commands

---
## set_plant_bpm

### Description

The `set_plant_bpm` function allows for the control of the beats per minute (bpm) on the Plant Midi Channel. This functionality is exclusive to Sys Ex messages and permits an unlimited count of parameters to be set. The final value of the bpm is determined by summing up all the specified parameters.

### Sys Ex Format

- Multiple parameters can be included, providing flexibility in adjusting the bpm.
- The cumulative total of all parameters corresponds to the final bpm value set for the Plant Midi Channel.


---

## set_light_bpm

### Description

The `set_light_bpm` feature governs the synchronization between plant notes and light notes, where for every 'n' plant notes played, one light note is triggered. The parameter value sets the ratio of plant notes to light notes played. The range of this parameter spans from 0 (indicating that the light channel is inactive) to 10 (denoting a light note played for every 10 plant notes).

### Sys Ex Format

- Accepted values range from 0 to 10, with 0 indicating no connection between plant and light notes, and 10 representing a one-to-ten ratio of plant notes to light notes.


---

## set_fib_power

### Description

The `set_fib_power` function governs the algorithm's power required to transition from one note to another within a sequence, with this power increasing for each subsequent note. The parameter value dictates the strength of this algorithm, ranging from 0% to 100%. In Sys Ex messages, a value of 55 corresponds to 55% power, while in CC messages, a value of 0 represents 0% power and 127 signifies 100% power.

### Sys Ex Format

- Values in Sys Ex messages directly correspond to the percentage of power allocated to the algorithm.

### CC Format

- In CC messages, a value of 0 indicates 0% power, while a value of 127 represents 100% power.

---

## set_fib_first

### Description

The `set_fib_first` function manages the initial power needed to transition from one note to another within a sequence. This initial power value ranges from 0 to 100 in Sys Ex messages and from 0 to 127 in CC messages (with 127 equivalent to 100 in Sys Ex). Modifying this parameter allows users to adjust the starting transition strength required between notes.

### Sys Ex Format

- The parameter value in Sys Ex messages ranges from 0 to 100, signifying the initial power for note transitions.

### CC Format

- In CC messages, the parameter value ranges from 0 to 127, with 127 representing 100% intensity.

---

## set_filter

### Description

The `set_filter` function configures a filter parameter that modulates raw data, enabling a smoothing effect. This parameter ranges from 0 to 0,99 , indicating the degree of data smoothing applied. In Sys Ex messages, the parameter value spans from 0 to 99, with 99 equivalent to 0,99. For CC messages, the range is from 0 to 127, where 127 corresponds to 0,99. Adjustment of this parameter influences the level of data smoothing implemented.

### Sys Ex Format


- Parameter values in Sys Ex messages vary from 0 to 99, with 99 representing full smoothing effect.
- This parameter allows users to regulate the degree of data smoothing within the system.

### CC Format

- In CC messages, the parameter value ranges from 0 to 127, with 127 resulting in the minimum smoothing effect (equivalent to 0,99).

---

## set_scale

### Description

The `set_scale` function is utilized to establish the scale for the device,
enabling the selection of specific musical scales. In Sys Ex format,
the system computes the value `n mod 12` to determine the scale number.
For CC messages, a predefined table is referenced to assign a particular
scale based on the parameter value provided.

### Sys Ex Format

- For Sys Ex messages, the system calculates the scale number using 'n mod 12'.

### CC Format

| Range Value | Scale      |
|-------------|------------|
| 0 - 10      | MAJOR      |
| 10 - 21     | MINOR      |
| 21 - 31     | CHROMATIC  |
| 31 - 42     | DORIAN     |
| 42 - 52     | MIXOLYDIAN |
| 52 - 63     | LYDIAN     |
| 63 - 73     | WHOLE TONE |
| 73 - 84     | MINOR BLUES|
| 84 - 95     | MAJOR BLUES|
| 95 - 105    | MINOR PENT  |
| 105 - 116   | MAJOR PENT  |
| 116 - 127   | DIMINISHED  |

---

## set_max_plant_vel

### Description

The `set_max_plant_vel` function determines the maximum (and default) velocity that can be assigned to the plant channel. Both CC and Sys Ex messages accept values ranging from 0 to 127. Additionally, the CC command for `set_max_plant_vel` is combined with the `set_max_light_vel` command.

### Sys Ex Format

- The `set_max_plant_vel` function in Sys Ex messages utilizes values from 0 to 127.

### CC Format

- In CC messages, the `set_max_plant_vel` function also values between 0 and 127.
- The CC command for `set_max_plant_vel` is merged with the `set_max_light_vel` command for streamlined control. Plant or light controlled by cc channel.

---

## set_max_light_vel

### Description

The `set_max_light_vel` function defines the maximum (and default) velocity assignable to the light channel. Both CC and Sys Ex messages accept values within the range of 0 to 127. Additionally, the CC command for `set_max_light_vel` is synchronized with the `set_max_plant_vel` command for unified control over velocity settings between the light and plant channels.

### Sys Ex Format

- The `set_max_light_vel` function in Sys Ex messages operates with values between 0 and 127.

### CC Format

- Utilizing CC messages, the `set_max_light_vel` function supports values ranging from 0 to 127.
- The CC command for `set_max_light_vel` is integrated with the `set_max_plant_vel` command for seamless velocity management. Plant or light controlled by cc channel.

---

## set_min_plant_vel

### Description

The `set_min_plant_vel` function sets the minimum velocity that can be assigned to the plant channel. This function is compatible with both CC and Sys Ex messages, accepting values from 0 to 127. Furthermore, the CC command for `set_min_plant_vel` is consolidated with the `set_min_light_vel` command to streamline velocity configurations across the plant and light channels.

### Sys Ex Format

- In Sys Ex messages, the `set_min_plant_vel` function responds to values within the range of 0 to 127.

### CC Format

- The `set_min_plant_vel` function in CC messages also operates with values between 0 and 127.
- Integration of the CC command for `set_min_plant_vel` with the `set_min_light_vel` command facilitates simplified velocity adjustments. Plant or light controlled by cc channel.

---

## set_min_light_vel

### Description

The `set_min_light_vel` function establishes the minimum velocity assignable to the light channel. This function supports values from 0 to 127 in both CC and Sys Ex messages. Moreover, the CC command for `set_min_light_vel` is harmonized with the `set_min_plant_vel` command, ensuring cohesive velocity control for the light and plant channels.

### Sys Ex Format

- The `set_min_light_vel` function in Sys Ex messages accommodates values within the range of 0 to 127.

### CC Format

- Using CC messages, the `set_min_light_vel` function operates with values ranging from 0 to 127.
- The CC command for `set_min_light_vel` is consolidated with the `set_min_plant_vel` command to provide unified velocity settings for comprehensive channel management. Plant or light controlled by cc channel.

---

## set_random_plant_vel

### Description

The `set_random_plant_vel` function facilitates the implementation of randomized velocity for the plant channel within the specified velocity range. This feature enables the assignment of random velocities within the set minimum and maximum velocity values.
Furthermore, the CC command for `set_random_plant_vel` is combined with the `set_random_light_vel` command to conveniently manage random velocity settings for both the plant and light channels.

### Sys Ex Format

- In Sys Ex messages, the `set_random_plant_vel` function uses a value greater than 0 to activate random velocity and 0 to deactivate it.

### CC Format

- For CC messages, values equal to or exceeding 64 turn on the random velocity feature, while values below 64 turn it off.
- The CC command for `set_random_plant_vel` is merged with the `set_random_light_vel` command for streamlined random velocity control across both channels.  Plant or light controlled by cc channel.

---

## set_random_light_vel

### Description

The `set_random_light_vel` function enables the randomization of velocity for the light channel within the specified velocity range.
The CC command for `set_random_light_vel` is combined with the `set_random_plant_vel` command to facilitate the coordinated management of random velocity settings for both the light and plant channels.

### Sys Ex Format

- The `set_random_light_vel` function in Sys Ex messages uses a value greater than 0 for enabling random velocity and 0 for disabling it.

### CC Format

- Utilizing CC messages, values at or above 64 activate the random velocity functionality, while values below 64 deactivate it.
- Integration of the CC command for `set_random_light_vel` with the `set_random_plant_vel` command ensures a harmonized approach to random velocity control across both channels.  Plant or light controlled by cc channel.

---

## set_mute_plant_vel

### Description

The `set_mute_plant_vel` function deactivates the music output from the plant channel.
This feature provides a method to mute the channel effectively.
The CC command for `set_mute_plant_vel` is integrated with the `set_mute_light_vel` command to facilitate unified control over muting settings for both the plant and light channels.

### Sys Ex Format

- When using Sys Ex messages, a value greater than 0 is employed to mute the plant channel, while 0 signifies an unmuted state.
### CC Format

- In CC messages, values above or equal to 64 indicate muting the channel, while values below 64 indicate an unmuted status.
- The CC command for `set_mute_plant_vel` is consolidated with the `set_mute_light_vel` command, ensuring consistent muting options for both channels.  Plant or light controlled by cc channel.

---

## set_mute_light_vel

### Description

The `set_mute_light_vel` function deactivates the music output from the light channel, providing a means to mute the channel effectively.
 The CC command for `set_mute_light_vel` is harmonized with the `set_mute_plant_vel` command, enabling streamlined control over muting settings for both the light and plant channels.

### Sys Ex Format

- The `set_mute_light_vel` function, in Sys Ex messages, uses a value greater than 0 to mute the light channel, with 0 indicating an unmuted state.

### CC Format

- For CC messages, values at or above 64 signify muting the light channel, while values below 64 represent an unmuted status.
- Integration of the CC command for `set_mute_light_vel` with the `set_mute_plant_vel` command ensures consistent muting options for both channels.  Plant or light controlled by cc channel.

---

## set_default

### Description

The `set_default` function establishes the default preset configuration without the need for any additional parameters. This function solely utilizes Sys Ex messages to set the default preset, providing a straightforward method to revert to a predefined configuration.

### Sys Ex Format

- The `set_default` function exclusively operates through Sys Ex messages.
- This function does not necessitate any parameter inputs for setting the default preset.


---

## set_random_note

### Description

The `set_random_note` function activates a mode that injects additional randomness to the raw value processing, enriching the musical output with unpredictability and variability. This feature introduces an element of spontaneity to the generation of notes, enhancing the creative possibilities within the composition.

### Sys Ex Format

- In Sys Ex messages, a value of 0 signifies that the mode is inactive, while any value greater than 1 activates the random note feature.

### CC Format

- For CC messages, values equal to or exceeding 64 turn on the random velocity feature, while values below 64 turn it off.

---

## set_same_note_plant

### Description

The `set_same_note_plant` mode prevents the generation of new Plant notes until the input value changes sufficiently, enhancing the musical output by ensuring variation in note repetition. When this mode is active, a value of 2 signifies that a new note will not be played until it differs from the previous note by at least ±2. Both CC and Sys Ex messages accept values ranging from 0 to 10 to control this mode.

### Sys Ex Format

- The `set_same_note_plant` function, through Sys Ex messages, operates with values between 0 and 127.

### CC Format

- In CC messages as well, values ranging from 0 to 127 are used to activate and control the `set_same_note_plant` mode.


---

## set_same_note_light

### Description

The `set_same_note_light` mode prevents the generation of new Light notes until the input value changes sufficiently, enhancing the musical output by ensuring variation in note repetition. When this mode is active, a value of 2 signifies that a new note will not be played until it differs from the previous note by at least ±2. Both CC and Sys Ex messages accept values ranging from 0 to 10 to control this mode.


### Sys Ex Format

- The `set_same_note_light` function via Sys Ex messages utilizes values between 0 and 10 to adjust the behavior of the light channel.

### CC Format

- In CC messages, values from 0 to 10 activate and regulate the `set_same_note_light` mode, influencing the generation of light notes based on input variance.

---

## set_note_off_percent

### Description

The `set_note_off_percent` function introduces the capability to end a note before its complete duration, enabling control over when a note stops playing. This function is designed to provide variations in note lengths, enhancing musical dynamics. Users can specify the note off fraction using a specific denominator value. The fraction values for CC messages are mapped as outlined in the table below:

| Range Value | Note Off Fraction |
|-------------|-------------------|
| 0 - 11      | 1/64              |
| 11 - 22     | 1/48              |
| 22 - 33     | 1/32              |
| 33 - 44     | 1/24              |
| 44 - 55     | 1/16              |
| 55 - 66     | 1/12              |
| 66 - 77     | 1/8               |
| 77 - 88     | 1/6               |
| 88 - 99     | 1/4               |
| 99 - 110    | 1/2               |
| 110 - 127   | 1                 |

Users can select the desired fraction for the note off function by specifying the denominator value in Sys Ex messages.

### Sys Ex Format

- The `set_note_off_percent` function in Sys Ex messages allows users to choose the note off fraction by inputting the denominator value directly, such as 24 for 1/24. Value outside the table won't work.

### CC Format

- CC messages utilize the predefined table to assign the corresponding note off fraction based on the provided range value.

---

## set_light_range

### Description

The `set_light_range` function manages the range of notes that can be obtained from the root note for the light channel, allowing users to define the note span within a musical composition. Both CC and Sys Ex messages accept values ranging from 0 to 127 to control this setting.

### Sys Ex Format

- In Sys Ex messages, the `set_light_range` function allows users to specify the note range from the root note for the light channel by inputting values between 0 and 127.

### CC Format

- Utilizing CC messages, users can adjust the range of notes available for the light channel, with values ranging from 0 to 127.

---

## set_light_pitch_mode

### Description

The `set_light_pitch_mode` function alters the pitch of the plant notes instead of adding extra light channel notes.
Activating this mode provides a distinct approach to manipulating musical output by focusing on pitch adjustments.

### Sys Ex Format

- Enabling the `set_light_pitch_mode` function through Sys Ex messages requires a value greater than 0 for activation and 0 for deactivation.

### CC Format

- Values at or above 64 in CC messages activate the `set_light_pitch_mode`, while those below 64 deactivate it.


---

## set_stuck_mode

### Description

The `set_stuck_mode` function enables a logic mechanism that introduces increased randomness to note generation when minimal activity is detected from the device. This feature enhances the musical output by infusing unpredictability into the composition during periods of inactivity from the user.

### Sys Ex Format

- A value greater than 0 in Sys Ex messages activates the `set_stuck_mode`, while 0 deactivates it.

### CC Format

- The `set_stuck_mode` can be activated in CC messages with values equal to or greater than 64, while values below 64 deactivate it.

---

## set_middle_plant_note

### Description

The `set_middle_plant_note` function determines the root note of the plant channel, allowing users to establish the central musical note around which compositions are built. This function enables users to set the primary note for the plant channel, influencing the key and tonal center of the musical composition.

### Sys Ex Format

- In Sys Ex messages, users can designate the middle plant note by sending values ranging from 0 to 127.

### CC Format

- CC messages support the setting of the middle plant note by transmitting values between 0 and 127.

---

## set_swing_first_note_percent

### Description

The `set_swing_first_note_percent` function adjusts the swing effect for the first and second notes in a musical sequence. By setting a percentage value, users can control the duration of the first and second notes relative to each other, creating rhythmic variations in the composition. For instance, setting the value to 70% would result in the first note playing for 70% of the time and the second note for 130% of that time, enhancing the rhythmic flow.

### Sys Ex Format

- Sys Ex messages accept values from 1 to 100 to adjust the swing effect for the first and second notes in the sequence.

### CC Format

- CC messages facilitate adjustments to the swing effect, with values ranging from 0 to 127 (127 representing 100% swing).


---

## set_channel

### Description

The `set_channel` function allows for the adjustment of channels for the plant and light channels exclusively through Sys Ex messages. The first parameter indicates whether the change applies to the plant (0) or the light (1) channel, while the second parameter, ranging from 0 to 15, specifies the new channel to be set.

### Sys Ex Format

- The first parameter distinguishes between the plant (0) and light (1) channels, while the second parameter designates the desired new channel (0 to 15).
