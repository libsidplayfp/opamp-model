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
  consists of two building blocks; a saturated load NMOS inverter (on the
  right hand side of the schematics) with a buffer / bias input stage
  consisting of a variable saturated load NMOS inverter (on the left hand
  side of the schematics).

  Provided a reasonably high input impedance and a reasonably low output
  impedance, the "op-amp" can be modeled as a voltage transfer function
  mapping input voltage to output voltage.


W/L

T1a (top left)      ~ 20/80
T2a (bottom left)   ~ 70/25
T1b (top right)     ~ 20/40
T2b (bottom right)  ~ 20/1000



Quadratic model - assuming triode/saturation mode

Ids = uCox/2 * W/L * (Vgst^2 - Vgdt^2) [Vgdt=0 in saturation]


Applying Kirchoff's current law

IdsT1 + IdsT2 = 0


Left side

W/L*((Vi - Vx - Vt)^2 - (Vi - Vdd - Vt)^2) + W/L*((Vo - Vx - Vt)^2 - (Vo - Vt)^2) = 0

W/L*((Vit - Vx)^2 - (Vit - Vdd)^2) + W/L*((Vot - Vx)^2 - Vot^2) = 0

by setting

x = Vx
k = W/L T1
m = W/L T2
a = Vit
b = Vot
c = Vdd

we can rewrite the above as

k((a-x)^2 - (a - c)^2) + m((b - x)^2 - b^2) = 0

which have a solution in

x = (2ka + 2mb - sqrt((-2ka - 2mb)^2 - 4(k + m)(-kc^2 + 2kac))) / 2(k + m)


https://www.symbolab.com/solver/equation-calculator/k%5Cleft(%5Cleft(a-x%5Cright)%5E%7B2%7D%20-%20%5Cleft(a%20-%20c%5Cright)%5E%7B2%7D%5Cright)%20%2B%20m%5Cleft(%5Cleft(b%20-%20x%5Cright)%5E%7B2%7D%20-%20b%5E%7B2%7D%5Cright)%20%3D%200?or=input


Right side

W/L*((Vddt - Vo)^2) + W/L*((Vxt - Vo)^2 - Vxt^2) = 0

by setting

x = Vo
k = W/L T1
m = W/L T2
a = Vxt
b = Vddt

we can rewrite the above as

k(b - x)^2 + m((a - x)^2 - a^2) = 0

which have a solution in

x = (kb + ma + sqrt(m(-kb^2 + 2kab + ma^2)) / (k + m)


https://www.symbolab.com/solver/equation-calculator/k%5Cleft(b%20-%20x%5Cright)%5E%7B2%7D%20%2B%20m%5Cleft(%5Cleft(a%20-%20x%5Cright)%5E%7B2%7D%20-%20a%5E%7B2%7D%5Cright)%20%3D%200?or=input

*/

#include <iostream>
#include <cassert>
#include <cmath>

int main() {

    constexpr double VOLTAGE_SKEW = 1.015;

    constexpr double Vdd = 12. * VOLTAGE_SKEW;
    constexpr double Vt = 1.31;

    double Vi = 4.54;
    double Vo = 4.54;

    double Vx;

    for (;;)
    {
        {
            // Vx = (2*WL1*Vit + 2*WL2*Vot + sqrt((-2*WL1*Vit-2*WL2*Vot)^2 - 4(WL1 + WL2)(-Vdd + 2*Vit)*WL1*Vdd))/2(WL1 + WL2)
            constexpr double WL1 = 20./80.;
            constexpr double WL2 = 70./25.;

            const double Vit = Vi - Vt;
            assert(Vit > 0.);
            const double Vot = Vo - Vt;
            assert(Vot > 0.);

            const double term = -2.*(WL1*Vit + WL2*Vot);
            const double wl_sum = WL1 + WL2;
            Vx = (-term + std::sqrt(term*term - 4. * wl_sum * (2.*Vit - Vdd)*WL1*Vdd)) / (2. * wl_sum);
            std::cout << "Vx: " << Vx << std::endl;
        }

        double oldVo = Vo;

        {
            // Vo = (WL1*Vddt + WL2*Vxt + sqrt(WL2*(-WL1*Vddt^2 + 2*WL1*Vddt*Vxt + WL2*Vxt^2)) / (WL1 + WL2)
            constexpr double WL1 = 20./40.;
            constexpr double WL2 = 20./1000.;

            const double Vddt = Vdd - Vt;
            const double Vxt = Vx - Vt;
            assert(Vxt > 0.);

            const double wl_sum = WL1 + WL2;
            Vo = (WL1*Vddt + WL2*Vxt + std::sqrt(WL2*(-WL1*Vddt*Vddt + 2.*WL1*Vddt*Vxt + WL2*Vxt*Vxt))) / wl_sum;
            std::cout << "Vo: " << Vo << std::endl;
        }

        if (Vo - oldVo < 1e-6)
            break;
    }
}
