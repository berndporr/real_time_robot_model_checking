# Modelchecking on the deltabot

## Prerequisites

Install the C1 lidar library: https://github.com/berndporr/c1lidar

## How to compile

```
cmake .
make
```

## How to run

Note the program logs into /tmp at the moment. 

Change:
```
const char loggingFilename[] = "/tmp/autoctrl.txt";
const char resourceLoggingFilename[] = "/tmp/usage.txt";
```
in `autoctrl.cpp` to log to a different location.

Then do:
```
cmake .
make
./autoctrl
```
