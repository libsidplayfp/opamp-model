# opamp-model
An attempt to model the SID opamp transfer functions


6581 OpAmp
===

## "Op-amp" (self-biased NMOS inverter)

~~~

               12V         12V

                T           T
                |           |
                |           |
                |    +------o
                |    |      |
                |    |  ||--+
                |    +--||
                |       ||--+
            ||--+           |
    Vi -----||              o---o----- Vo
            ||--+           |   |
                | Vx    ||--+   |
                o-------||      |
                |       ||--+   |
            ||--+           |   |
         +--||              |   |
         |  ||--+           |   |
         |      |           |   |
         |      |           |   |
         |      V           V   |
         |                      |
         |     GND         GND  |
         |                      |
         +----------------------+

    Vi  - input voltage
    Vo  - output voltage
~~~

The schematics above are laid out to show that the "op-amp" logically
consists of two building blocks; an enhancement load common source amplifier
(on the right hand side of the schematics) with a common drain input stage biased
by the output voltage (on the left hand side of the schematics).


### W/L values

* M1a (top left)      ~ 80/20
* M2a (bottom left)   ~ 25/70
* M1b (top right)     ~ 40/20
* M2b (bottom right)  ~ 650/20


### Reference values, measured on CAP1B/CAP1A on a chip marked MOS 6581R4AR 0687 14

![screenshot opamp transfer function](https://github.com/libsidplayfp/opamp-model/blob/main/opamp-6581.png)


### Circuit simulation

Assuming `uCox = 20uA/V²` and `Vt = 1.31V`

https://www.falstad.com/circuit/circuitjs.html?ctz=CQAgjCAMB0l3AWAnC1b0DYQGYBM0B2AViIA4i4jdIEKSRdsQjnJmBTAWjDACgAHcNiS4QnbNixgqbcZJxRFkXgHNwMsRKkbOuIljbKAZuGmi5U6rK3NRYaNggx4kAhlIJeAJXBXNVXwRSf1FDEAQ2MFClaCJeEx4A3QjfODFcFID7RyhYF1IkDFVU2VwMkrEwNyVeAHcK3X0G3FJg5XqotPFqZtaoXgA3cDhgi1MkmzYEcKnCFiJYlkNY4rARzXk1yFHsGhqEjNGW4KjC9L6shyc8yAwkAs8Ov10+qKDztrrx8yiTs3SUsZAqMIpFcGdOKDbOArrkXAhcHwnhCbKcsOI9u1fCjNuD0dgPP0fGixAgwFJsAhppDyUoZr46TA4sTDqS0p1ZFCwikooyVmoSboCHY8eltjVkfjNsJzJMvmAZRsKSJKi1+h1Fdw1YkfmrlAB7EDBDDTSLYIgIggMZYuJBufR6RSiYLYXiGtgmxQKi24LA9GAQQNu0wgT1mn1WtZw+B2jAOliiOwQV1AA


unfortunately this is quite off from the measured values.

Open questions:
* are all the transistors enhancement-mode?
* do all the transistors have the same Vt?
