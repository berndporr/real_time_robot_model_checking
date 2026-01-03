# Modelchecking on the deltabot

## Prerequisites

Install the C1 lidar library: https://github.com/berndporr/c1lidar

## How to compile

```
cmake .
make
```

## How to run

Note the program logs into /data at the moment. You need to
change:
```
logger.startLogging("/data/deltabot/autoctrl.txt", true);
logger.startResourceLogging("/data/deltabot/usage.txt");
```
in `autoctrl.cpp`.

Then do:
```
cmake .
make
./autoctrl
```
