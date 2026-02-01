# Slamtec RPLIDAR C1 driver class for the Rock5 and RPI

![alt tag](lidar_on_robot.jpg)

This project describes how to connect a Slamtech RPLIDAR C1 directly
to the serial port of the Rock5. It should also work with the
RPI but hasn't been tested yet.

This is a re-worked version of the official [Slamtec SDK](https://github.com/Slamtec/rplidar_sdk) to support cmake and wraps it all into a C++ class.

A 360 degree scan is provided by a callback at the sampling rate
of the LIDAR at 600RPM (10Hz).

## Hardware setup

Connect the LIDAR to the UART pins:

![alt tag](wiring.png)

## Software

This howto is for ARMbian.

### Prerequisites

Enable the UART for serial communication. Start `sudo nano /boot/armbianEnv.txt`, identify these lines and add/edit them that
they look like these:

```
console=display
overlay_prefix=
overlays=rk3588-uart2-m0
```

This enables the UART.

Add yourself to the group `dialout` to access /dev/tty*.

### Installation

`cmake .`

`make`

`sudo make install`

This installs the library and can then be used in your application.

## C1Lidar class

The class has `start()` and `stop()` functions which start and
stop the data acquisition and also start and stop the motor of
the range finder.

A full scan is handed over via `DataInterface` where the abstract function
`newScanAvail(A1LidarData (&)[A1Lidar::nDistance]) = 0` needs to be implemented
which then receives both the polar and Cartesian coordinates after
a successful 360 degree scan. Register your `DataInterface` with
`registerInterface`.

## Example program
`printdata` prints tab separated distance data as
`x <tab> y <tab> r <tab> phi <tab> strength` until a key is pressed.

Pipe the data into a textfile and plot it with `gnuplot`:
```
./printdata > tt2.tsv
gnuplot> plot "tt2.tsv"
```
![alt tag](map.png)

## Using the library in your own program

Include `c1lidarrpi.h` in your C++ program and add to your cmakefile the line:
```
target_link_libraries(myexecutable c1lidarrpi Threads::Threads)
```

## Credits

The `rplidarsdk` folder is the `sdk` folder
from the official Slamtec git:

https://github.com/Slamtec/rplidar_sdk

re-working of the SDK and c1lidarrpi class: Bernd Porr.
