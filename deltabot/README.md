# Deltabot

![alt tag](c1lidar/lidar_on_robot.jpg)

## Hardware

- Deltabot: https://github.com/berndporr/deltabot
- 360 degree 2D laser scanner, RPLIDAR C1MB by Slamtec: https://www.slamtec.com/en/c1

## Software

### Prerequisites

This software requires Armbian/Debian trixie.

Install the following SDKs:

- Lidar - [c1lidar](c1lidar)

### Compile 

Generate the build files

```cmake .```

Compile the program

```make```

### Run

```./autoctrl```
