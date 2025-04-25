# Rudder Angle Sensor

A complete, working example of using an inexpensive resistive rudder angle sensor to get `steering.rudderAngle` values into SignalK.

See the signalk [spec for /vessels/<RegExp>/steering/rudderAngle](https://signalk.org/specification/1.7.0/doc/vesselsBranch.html#vesselsregexpsteeringrudderangle)).

This is the primary chain of responsibility:

* The sensor is connected to the esp32 through a voltage divider using the 3.3v rail.
* The voltage is read repeatedly via `analogReadMilliVolts` and a `sensesp::RepeatSensor`
* A `sensesp::VoltageDividerR2` is used to calculate the value of the variable resistor in the sensor.
* The calculated resistance goes through a simple linear transform, such as 0 Ohms to -45 degrees and 190 Ohms to +45 degrees.
  * *Note that the ADC on ESP32 has some accuracy problems and isn't completely linear, but it works well enough for rudder angle if the min and max resistance as seen after the voltage divider transform are fed back into the min/max for the linear transform.*
* The angle is converted from degrees to radians
* That radians value is sent to signalk

There is also a status page item for each of the intermediate values, so you can easily validate everything or make adjustments if you need to tweak the resistor values, angles, etc.  In addition, a listener shows the `steering.rudderAngle` value received back from signalk to fully confirm it works.

Code from these files could potentially be folded back into the main SensESP project:
* [src/linear.h](src/linear.h) 
* [src/radians.h](src/radians.h)

`linear.h` just contains a convenience method to get an instance of `sensesp::Linear` transform (which is defined using slope intercept format, `f(x) = mx + b`), by deriving that from two points.  The points could anywhere on the line mark start and end range for the transform (i.e. `x1, y1` -> `x2, y2`, or in English "Get a linear transform where resistance value 2.11 Ohms maps to angle value -50 degrees, and 223 Ohms maps to +50 degrees")

`radians.h` is simply a `sensesp::LambdaTransform` to convert from degrees to readians (signalk spec uses radians).

# Hardware
Simple, common rudder angle sensors might look like this:

![Stock image of a rudder angle sensor made of white plastic enclosing a potentiometer, attached to a metal swing arm.  One sensor terminal with a hex nut is also visible](images/basic_rudder_angle_sensor.png)

Then, a voltage divider can be used to measure the resistance.  For a sensor in the range of 0-190 Ohms a second resistor of 47 Ohms worked fine.  The ESP32 ADC is not particularly acurrate so checking the *reported* min/max resistance values from the sensesp voltage divider and then pluging those back into the min/max for the linear transform is recommended.  In the case of 0-190 Ohms and 47 Ohms on the divider, the calculated resistances were coming out in the range of ~2 Ohms to ~223 Ohms.  Feeding that back into the linear transform makes a big difference in at least getting a correct min angle, max angle, and midpoint/zero.  To get more accuracy, it would probably be neccessary to use the `sensesp::CurveInterpolator` with a number of samples, as in the example for [temperature_sender.cpp](https://github.com/SignalK/SensESP/blob/main/examples/temperature_sender.cpp).

# SensESP Project Template

This project was generated using the template for [SensESP](https://github.com/SignalK/SensESP/) projects. A living example adapted for S/V Symphony's aft sensesp unit lives at [Dark-Star-LLC/symphony-sensesp-aft](https://github.com/Dark-Star-LLC/symphony-sensesp-aft)

Comprehensive documentation for SensESP, including how to get started with your own project, is available at the [SensESP documentation site](https://signalk.org/SensESP/).