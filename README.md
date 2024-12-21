# Project Name

SPC_2024_project_embed

## Table of Contents

- [Project Name](#project-name)
  - [Table of Contents](#table-of-contents)
  - [Overview](#overview)
  - [Features](#features)
  - [Installation](#installation)
  - [Usage](#usage)
  - [Libraries](#libraries)
  - [License](#license)

## Overview

This project is part of a school project, including the setting of the PC volume.
Second part of this project can be found here: https://github.com/MartinStieber/SPC_2024_project.
This project demonstrates the use of the TM1637 display, serial communication, and ADC functionality on an Arduino UNO R3 (with ATmega328p) board. It includes features such as mute/unmute functionality, median filtering of ADC values, and more.

## Features

- TM1637 display driver
- Serial communication with median filtering
- ADC initialization and interrupt handling
- Mute/unmute functionality

## Installation

1. Clone the repository:
    ```sh
    git clone https://github.com/MartinStieber/SPC_2024_project_embed.git
    ```
2. Navigate to the project directory:
    ```sh
    cd SPC_2024_project_embed
    ```
3. Install PlatformIO:
    ```sh
    pip install platformio
    ```
4. Build the project:
    ```sh
    pio run
    ```

## Usage

1. Connect your Arduino board.
2. Upload the firmware:
    ```sh
    pio run --target upload
    ```
3. Continue with SPC_2024_project (https://github.com/MartinStieber/SPC_2024_project).

## Libraries

This project uses the following libraries:

- [TM1637](https://github.com/avishorp/TM1637) - A library for controlling the TM1637 7-segment display (licensed under LGPL-3.0).
  - Modifications made to the original TM1637 library:
    - Removed unnecessary methods for this project
    - Some methods were renamed and changed
    - Removed examples
- Custom `Serial` library for serial communication with median filtering.
- Custom `TQueue` library for queue management.

## License

This project is licensed under the GNU General Public License (GPL) version 3.0. See the [LICENSE.md](LICENSE.md) file for details.

The TM1637 library included in this project is licensed under the GNU Lesser General Public License (LGPL) version 3.0. See the [LICENSE-LGPL-3.0.txt](LICENSE-LGPL-3.0.txt) file for details. The original library can be found at https://github.com/avishorp/TM1637.
