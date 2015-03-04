# SparkRobot
Control a robot through a TCP server

## Protocol
By default, messages end with `'\n'`

`D0_H`
sets digital pin 0 HIGH

`D0_L`
sets digital pin 0 LOW

`D1_255`
sets digital pin 1 to PWM 255

`A0_0`
sets analog pin 0 to PWM 0
