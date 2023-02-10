<div id="top"></div>

<!-- PROJECT SHIELDS -->
<!--
*** I'm using markdown "reference style" links for readability.
*** Reference links are enclosed in brackets [ ] instead of parentheses ( ).
*** See the bottom of this document for the declaration of the reference variables
*** for contributors-url, forks-url, etc. This is an optional, concise syntax you may use.
*** https://www.markdownguide.org/basic-syntax/#reference-style-links
-->
[![Contributors][contributors-shield]][contributors-url]
[![Forks][forks-shield]][forks-url]
[![Stargazers][stars-shield]][stars-url]
[![Issues][issues-shield]][issues-url]
[![MIT License][license-shield]][license-url]


<!-- PROJECT LOGO -->
<br />
<div align="center">
  <a href="https://github.com/Playtronica">
    <img src="images/logo.png" alt="Logo" width="100">
  </a>

<h3 align="center">Biotron</h3>

  <p align="center">
    BODY IS AN INSTRUMENT
    <br />
    <a href="https://github.com/Playtronica/Biotron"><strong>Explore the docs »</strong></a>
    <br />
    <br />
    <a href="#">View Demo</a>
    ·
    <a href="https://github.com/Playtronica/Biotron/issues">Report Bug</a>
    ·
    <a href="https://github.com/Playtronica/Biotron/issues">Request Feature</a>
  </p>
</div>



<!-- TABLE OF CONTENTS -->

<details>

  <summary>Table of Contents</summary>

  <ol>
    <li>
      <a href="#about-the-project">About The Project</a>
      <ul>
        <li><a href="#built-with">Built With</a></li>
      </ul>
    </li>
    <li>
      <a href="#getting-started">Getting Started</a>
      <ul>
        <li><a href="#prerequisites">Prerequisites</a></li>
        <li><a href="#installation">Installation</a></li>
      </ul>
    </li>
    <li><a href="#commands">MIDI Commands</a></li>
    <li><a href="#contributing">Contributing</li>
    <li><a href="#license">License</a></li>
    <li><a href="#contact">Contact</a></li>
  </ol>

</details>


## About The Project

TODO


### Built With

* [Raspberry Pi Pico SDK](https://github.com/raspberrypi/pico-sdk)

<p align="right">(<a href="#top">back to top</a>)</p>

<!-- GETTING STARTED -->
## Getting Started


### Prerequisites

Install docker, example for Ubuntu 20.04


### Build and flash with docker (Recomended way)

1. Clone the repo
   ```sh
   git clone https://github.com/Playtronica/Biotron.git
   ```
2. Setup build image
   ```sh
   make build_image
   ```
3. Build binary
   ```sh
   make build
   ```
4. Boot biotron in bootloader mode
 - TODO

5. Copy `biotron.uf2` from `output` dir to touchme mass storage device  
   Example:
    ```sh
    cp output/biotron.uf2 /media/user/RPI-RP2
    ```


<p align="right">(<a href="#top">back to top</a>)</p>

<!-- COMMANDS -->
## MIDI Commands

Biotron has some custom MIDI commands. 

### Syntax

| Number of Byte  | 1          | 2        | 3                    | 4               | 5              | ..  | 6        |
|-----------------|------------|----------|----------------------|-----------------|----------------|-----|----------|
| Possible Values | F0         | B        | 0-3                  | 0-7F            | 0-7F           | ..  | F7       |
| Info            | Start Byte | Key Byte | Function Number Byte | Parameters Byte | Parametrs Byte | ..  | End Byte |

### Commands
1) **BPM change (Function Number Bite = 0)** - Change BPM of Biotron. Can have infinite variables.
Sum of the variables -> bps. **(Warning. With very big value, device can crash)**.

2) **Change sensitivity (fibonacci algorithm) (Function Number Bite = 1)** - Function that create longer
distance between notes. Has two parameters. First is influence on main algorithm (from 0 to 100%). And Second is
first value of fibonacci (from 0 to 100, but it converts in 0 to 1).

3) **Change smoothness (Function Number Bite = 2)** - It changes the smoothness of note changes. Has one parameter.
The parameter is responsible for smoothness (from 0 to 99%). 

4) **Change scale (Major, minor, etc) (Function Number Bite = 3)** - Changes octave. Has one parameter.
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

<p align="right">(<a href="#top">back to top</a>)</p>

<!-- CONTRIBUTING -->
## Contributing

Contributions are what make the open source community such an amazing place to learn, inspire, and create. Any contributions you make are **greatly appreciated**.

If you have a suggestion that would make this better, please fork the repo and create a pull request. You can also simply open an issue with the tag "enhancement".
Don't forget to give the project a star! Thanks again!

1. Fork the Project
2. Create your Feature Branch (`git checkout -b feature/AmazingFeature`)
3. Commit your Changes (`git commit -m 'Add some AmazingFeature'`)
4. Push to the Branch (`git push origin feature/AmazingFeature`)
5. Open a Pull Request

<p align="right">(<a href="#top">back to top</a>)</p>



<!-- LICENSE -->
## License
TODO Add license

<!-- Distributed under the MIT License. See `LICENSE.txt` for more information. -->

<p align="right">(<a href="#top">back to top</a>)</p>



<!-- CONTACT -->
## Contact

* Project Link: [https://github.com/Playtronica/Biotron](https://github.com/Playtronica/Biotron)
* Our website: [https://playtronica.com/](https://playtronica.com/)

### Social media
* [Facebook](https://www.facebook.com/playtronica)
* [Instagram](http://instagram.com/playtronica)
* [Twitter](https://twitter.com/playtronica)
* [Youtube](https://www.youtube.com/playtronica)


<p align="right">(<a href="#top">back to top</a>)</p>



<!-- ACKNOWLEDGMENTS -->
<!-- ## Acknowledgments
* []()
* []()
* []()
<p align="right">(<a href="#top">back to top</a>)</p> -->


<!-- MARKDOWN LINKS & IMAGES -->
<!-- https://www.markdownguide.org/basic-syntax/#reference-style-links -->
[contributors-shield]: https://img.shields.io/github/contributors/Playtronica/Biotron.svg?style=for-the-badge
[contributors-url]: https://github.com/Playtronica/Biotron/graphs/contributors
[forks-shield]: https://img.shields.io/github/forks/Playtronica/Biotron.svg?style=for-the-badge
[forks-url]: https://github.com/Playtronica/Biotron/network/members
[stars-shield]: https://img.shields.io/github/stars/Playtronica/Biotron.svg?style=for-the-badge
[stars-url]: https://github.com/Playtronica/Biotron/stargazers
[issues-shield]: https://img.shields.io/github/issues/Playtronica/Biotron.svg?style=for-the-badge
[issues-url]: https://github.com/Playtronica/Biotron/issues
[license-shield]: https://img.shields.io/github/license/Playtronica/Biotron.svg?style=for-the-badge
[license-url]: https://github.com/Playtronica/Biotron/blob/master/LICENSE.txt
[linkedin-shield]: https://img.shields.io/badge/-LinkedIn-black.svg?style=for-the-badge&logo=linkedin&colorB=555
[linkedin-url]: https://linkedin.com/in/linkedin_username


