# opamp-model
An attempt to model the SID opamp transfer functions


6581 OpAmp
===

"Op-amp" (self-biased NMOS inverter)
------------------------------------
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

_Notes_:

The schematics above are laid out to show that the "op-amp" logically
consists of two building blocks; an enhancement load NMOS inverter (on the
right hand side of the schematics) with a common drain input stage biased
by the output voltage (on the left hand side of the schematics).


W/L
---
* M1a (top left)      ~ 80/20
* M2a (bottom left)   ~ 25/70
* M1b (top right)     ~ 40/20
* M2b (bottom right)  ~ 650/20


Reference values, measured on CAP1B/CAP1A on a chip marked MOS 6581R4AR 0687 14
---

![screenshot compact view](https://github.com/libsidplayfp/opamp-model/opamp-6581.png)
