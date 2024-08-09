<div id="top"></div>

<!-- PROJECT SHIELDS -->
<!--
*** I'm using markdown "reference style" links for readability.
*** Reference links are enclosed in brackets [ ] instead of parentheses ( ).
*** See the bottom of this document for the declaration of the reference variables
*** for contributors-url, forks-url, etc. This is an optional, concise syntax you may use.
*** https://www.markdownguide.org/basic-syntax/#reference-style-links
-->
<!--
[![Contributors][contributors-shield]][contributors-url]
[![Forks][forks-shield]][forks-url]
[![Stargazers][stars-shield]][stars-url]
[![Issues][issues-shield]][issues-url]
[![MIT License][license-shield]][license-url]
-->


<!-- PROJECT LOGO -->
<br />
<div align="center">
  <a href="https://github.com/Playtronica">
    <img src="static/logo.png" alt="Logo" width="100">
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
        <li><a href="#build-firmware-file">Build firmware file</a></li>
        <li><a href="#load-firmware">Load Firmware</a></li>
      </ul>
    </li>
    <li><a href="#biotrons-continuous-controllers-">Biotron's Continuous Controllers</a></li>
    <li><a href="#contributing">Contributing</a></li>
    <li><a href="#license">License</a></li>
    <li><a href="#contact">Contact</a></li>
  </ol>

</details>


## About The Project

### Biotron: A Plant-Responsive MIDI Device

Biotron is a unique MIDI device that transforms the internal processes of plants into musical compositions. By connecting to a plant via electrodes, Biotron captures the plant's natural electrical resistance and translates it into MIDI signals, allowing you to create dynamic and evolving music that reflects the plant's interaction with its environment.

### Key Features:

- Plant Interaction: Biotron measures the plant's active resistance, capacitive resistance, and minimal inductive resistance to generate MIDI notes. The music changes in real-time as the plant's internal conditions, such as moisture and environmental factors, fluctuate.
- Theremin-Like Playability: In addition to responding to the plant's internal state, Biotron also reacts to movements around the plant, similar to a theremin. This feature allows you to "play" the plant by moving your hands nearby, influencing the music.
- Light Sensor Integration: Biotron includes a second MIDI channel connected to a light sensor. By adjusting light and shadows over the sensor, you can add another layer of musical interaction and depth to your compositions.
- Customizable Generative Logic: With the help of Playtronica's online tool, you can adjust and fine-tune Biotron’s generative music logic, tailoring the device to your creative needs and exploring various soundscapes.
- Versatile Connectivity: Biotron connects via USB and is compatible with DAWs on phones, computers, and hardware synthesizers, making it a flexible addition to any music production setup.

Biotron provides a unique fusion of nature and technology, turning the life processes of plants into a living, evolving musical experience. Ideal for musicians, sound artists, and nature enthusiasts, Biotron offers an innovative way to explore the intersection of biology and music.


### Built With

* [Raspberry Pi Pico SDK](https://github.com/raspberrypi/pico-sdk)
* [Docker](https://www.docker.com/)

<p align="right">(<a href="#top">back to top</a>)</p>

<!-- GETTING STARTED -->
## Getting Started

### Build firmware file


1. Install docker, example for Ubuntu 20.04

2. Clone the repo
   ```sh
   git clone https://github.com/Playtronica/Biotron.git
   ```
3. Setup build image and build binary
   ```sh
   make build
   ```
4. Get your firmware file in output directory (file with extension uf2)


### Load Firmware

For loading device you need firmware file. You can get it in different ways:
1) Build firmware by yourself. How to do it, you can read [here](#build-firmware-file)
2) Load firmware from [releases](https://github.com/Playtronica/biotron-firmware/releases/latest)
3) Open [WebMidi](https://playtronica.github.io/WebMidiVue/#/biotron) and press "Update Firmware". 
(It also change device state in boot mode)

After that you need to turn on boot mode on device:

1) If your device is already have one of the latest firmwares - 
Open [WebMidi](https://playtronica.github.io/WebMidiVue/#/biotron) and press "Update Firmware" 
(You also get the latest firmware)
2) If your firmware version is not latest, or you have problems with first method -
while you are connecting device to PC, lock boot pins

The device will be displayed as removable media (like a USB flash drive).
You should transfer the resulting .uf2 file to the removable media that appeared.


<p align="right">(<a href="#top">back to top</a>)</p>

<!-- COMMANDS -->
## Biotron's Continuous Controllers 
**Continuous Controllers** - MIDI messages, 
which are used to patch data for parameters.

Get list of Biotron's CC [here](./CC.md)

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

* Project Link: [https://github.com/Playtronica/biotron-firmware](https://github.com/Playtronica/biotron-firmware)
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

