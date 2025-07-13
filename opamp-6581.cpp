/*
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
              |   Vx  ||--+   |
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
  Notes:

  The schematics above are laid out to show that the "op-amp" logically
  consists of two building blocks; an enhancement load NMOS inverter (on the
  right hand side of the schematics) with a common drain input stage biased
  by the output voltage (on the left hand side of the schematics).

  Provided a reasonably high input impedance and a reasonably low output
  impedance, the "op-amp" can be modeled as a voltage transfer function
  mapping input voltage to output voltage.


W/L

M1a (top left)      ~ 80/20
M2a (bottom left)   ~ 25/70
M1b (top right)     ~ 40/20
M2b (bottom right)  ~ 650/20

---

Transistor quadratic model - assuming triode/saturation mode

Ids = uCox/2 * W/L * (Vgst^2 - Vgdt^2) [Vgdt=0 in saturation]

---

Source follower

https://www.allaboutcircuits.com/technical-articles/introduction-to-the-common-drain-amplifier-large-signal-behavior/

Vo = Vi - (sqrt(2*Ibias/uCox*WL) + Vt)

M1 is always in saturation
M2 is in saturation if Vx >= Vo - Vt

so

Vx = Vi - (sqrt(2*IdsM2/(uCox*WL1)) + Vt)


Vx = Vi - (sqrt(2*uCox/2 * WL2 * (Vgst^2 - Vgdt^2)/(uCox*WL1)) + Vt)

Vx = Vi - (sqrt(uCox * WL2 * (Vgst^2 - Vgdt^2)/(uCox*WL1)) + Vt)

Vx = Vi - (sqrt(WL2/WL1 * (Vgst^2 - Vgdt^2)) + Vt)

Vx = (Vi - Vt) - sqrt(WL2/WL1 * ((Vo-Vt)^2 - (Vo-Vx-Vt)^2))


x = a - sqrt(c * (b^2 - (b-x)^2))

https://www.symbolab.com/solver/equation-calculator/x%20%3D%20a%20-%20sqrt%5Cleft(c%20%5Ccdot%20%5Cleft(b%5E%7B2%7D%20-%20%5Cleft(b-x%5Cright)%5E%7B2%7D%5Cright)%5Cright)?or=input

x = (a + cb + sqrt(c(-a^2 + 2ab + cb^2)))/(1+c)

Vx = ((Vi - Vt) + WL2/WL1*(Vo - Vt) + sqrt(WL2/WL1*(-(Vi - Vt)^2 + 2*(Vi - Vt)*(Vo - Vt) + WL2/WL1*(Vo - Vt)^2)))/(1+WL2/WL1)

---

Enhancement-load inverter

M1 = l (load)
M2 = d (driver)

IdsM1 = Kn(Vdd-Vt)^2 (saturated load)

Vx<Vt => Idl = 0

Vx>Vt => Vo = Vddt - sqrt(Kd/Kl*(Vx - Vt))

---

Reference values, measured on CAP1B/CAP1A on a chip marked MOS 6581R4AR 0687 14:

 0.81 -> 10.31
 2.40 -> 10.31
 2.60 -> 10.30
 2.70 -> 10.29
 2.80 -> 10.26
 2.90 -> 10.17
 3.00 -> 10.04
 3.10 ->  9.83
 3.20 ->  9.58
 3.30 ->  9.32
 3.50 ->  8.69
 3.70 ->  8.00
 4.00 ->  6.89
 4.40 ->  5.21
 4.54 ->  4.54
 4.60 ->  4.19
 4.80 ->  3.00
 4.90 ->  2.30
 4.95 ->  2.03
 5.00 ->  1.88
 5.05 ->  1.77
 5.10 ->  1.69
 5.20 ->  1.58
 5.40 ->  1.44
 5.60 ->  1.33
 5.80 ->  1.26
 6.00 ->  1.21
 6.40 ->  1.12
 7.00 ->  1.02
 7.50 ->  0.97
 8.50 ->  0.89
10.00 ->  0.81
10.31 ->  0.81

*/

#include <iostream>
#include <iomanip>

#include <cassert>
#include <cmath>

int main() {

    constexpr double VOLTAGE_SKEW = 1.015;

    constexpr double Vdd = 12. * VOLTAGE_SKEW;
    constexpr double Vt = 1.31;

    //double Vi = 4.54;
    double Vo = 5.;

    double Vx;

    for (double Vi = 2.; Vi < 7.; Vi += 0.1)
    {
        for (;;)
        {
            {
                // Vx = ((Vi - Vt) + WL2/WL1*(Vo - Vt) + sqrt(WL2/WL1*(-(Vi - Vt)^2 + 2*(Vi - Vt)*(Vo - Vt) + WL2/WL1*(Vo - Vt)^2)))/(1+WL2/WL1)
                constexpr double WL1 = 73./19.;
                constexpr double WL2 = 26./58.;

                assert(Vi > Vt); // M1 is in subthreshold mode
                const double Vit = Vi - Vt;
                assert(Vo > Vt); // M2 is in subthreshold mode
                const double Vot = Vo - Vt;

                const double c = WL2/WL1;
                Vx = (Vit + c*Vot + std::sqrt(c*(-(Vit*Vit) + 2*Vit*Vot + c*Vot*Vot)))/(1. + c);
                //std::cout << "Vx: " << Vx << std::endl;
            }

            double oldVo = Vo;

            {
                // Vo = Vddt - sqrt(Kd/Kl*(Vx - Vt))
                constexpr double WL1 = 40./19.;
                constexpr double WL2 = 664./19.;

                const double Vddt = Vdd - Vt;
                assert(Vx > Vt); // M2 is in subthreshold mode
                const double Vxt = Vx - Vt;

                const double c = WL2/WL1;
                Vo = Vddt - std::sqrt(c*Vxt);
                //std::cout << "Vo: " << Vo << std::endl;
            }

            if (std::abs(Vo - oldVo) < 1e-6)
                break;
        }
        std::cout << std::fixed << std::setprecision(1) << Vi << " -> " << std::setprecision(3) << Vo << std::endl;
    }
}
