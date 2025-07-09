# opamp-model
An attempt to model the SID opamp transfer functions


6581 OpAmp
===

"Op-amp" (self-biased NMOS inverter)
------------------------------------
~~~
    
                12V         12V
    
                |           |
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
         |      |           |   |
         |                      |
         |     GND         GND  |
         |                      |
         +----------------------+

    Vi  - input voltage
    Vo  - output voltage
~~~

_Notes_:

The schematics above are laid out to show that the "op-amp" logically
consists of two building blocks; a saturated load NMOS inverter (on the
right hand side of the schematics) with a buffer / bias input stage
consisting of a variable saturated load NMOS inverter (on the left hand
side of the schematics).


W/L
---
T1a (top left)      ~ 20/80
T2a (bottom left)   ~ 70/25
T1b (top right)     ~ 20/40
T2b (bottom right)  ~ 20/1000
