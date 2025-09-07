/*
 * This file is part of libsidplayfp, a SID player engine.
 *
 * Copyright 2025 Leandro Nini <drfiemost@users.sourceforge.net>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

/*
  "Op-amp" (self-biased NMOS inverter)
  ------------------------------------
  ~~~

             12V         12V

              ┬           ┬
              │           │
              │           │
              │    ┌──────o
              │    │      │
              │    │  │├──┘
              │    └──┤│
              │       │├──┐
          │├──┘           │
  Vi ─────┤│              o───o───── Vo
          │├──┐           │   │
              │   Vx  │├──┘   │
              o───────┤│      │
              │       │├──┐   │
          │├──┘           │   │
       ┌──┤│              │   │
       │  │├──┐           │   │
       │      │           │   │
       │      │           │   │
       │      V           V   │
       │                      │
       │     GND         GND  │
       │                      │
       └──────────────────────┘


  Vi  - input voltage
  Vo  - output voltage
  ~~~
  Notes:

  The schematics above are laid out to show that the "op-amp" logically
  consists of two building blocks; a common source amplifier with
  enhancement load (on the right hand side of the schematics)
  and a common drain input stage biased by the output voltage
  (on the left hand side of the schematics).

  Provided a reasonably high input impedance and a reasonably low output
  impedance, the "op-amp" can be modeled as a voltage transfer function
  mapping input voltage to output voltage.


W/L

M1a (top left)      ~ 80/20
M2a (bottom left)   ~ 25/70
M1b (top right)     ~ 40/20
M2b (bottom right)  ~ 650/20

---

# Common drain source follower

https://www.allaboutcircuits.com/technical-articles/introduction-to-the-common-drain-amplifier-large-signal-behavior/

---

# Enhancement-load common source amplifier

https://ittc.ku.edu/~jstiles/412/handouts/6.5%20The%20Common%20Source%20Amp%20with%20Active%20loads/section%206_5%20The%20Common%20Source%20Amp%20with%20Active%20Loads%20lecture.pdf

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

---

Transistor EKV model

Id = Is * (if - ir)

Is = 2*n*uCox*W/L*Ut^2

if = ln(1 + exp((Vp-Vs)/(2*Ut)))^2
ir = ln(1 + exp((Vp-Vd)/(2*Ut)))^2

Vp ~ (Vg - Vt)/n

*/
#include <stdio.h>
#include <gsl/gsl_errno.h>
#include <gsl/gsl_math.h>
#include <gsl/gsl_roots.h>

#include <iostream>
#include <iomanip>

#include <cassert>
#include <cmath>


// Boltzmann Constant
constexpr double k = 1.380649e-23;
// charge of an electron
constexpr double q = 1.602176634e-19;

// temperature in °C
constexpr double temp = 27.; // ?

// thermal voltage Ut = kT/q
constexpr double Ut = k * (temp + 273.15) / q;

constexpr double uCox = 20e-6;

struct transistor_params
{
    double Vg, Vd, Vs;
    double Vt, WL;
};

using model_params = transistor_params[2];

double ids(transistor_params *p)
{
    double Vg = p->Vg;
    double Vd = p->Vd;
    double Vs = p->Vs;
    double Vt = p->Vt;
    double WL = p->WL;

    double Vp = Vg - Vt;

    double if_tmp = std::log1p(std::exp((Vp - Vs)/(2.*Ut)));
    double ir_tmp = std::log1p(std::exp((Vp - Vd)/(2.*Ut)));
    double is = 2 * uCox * WL * Ut*Ut;
    return if_tmp*if_tmp - ir_tmp*ir_tmp;
}

double ekv(double x, void *params)
{
    model_params *p = (model_params*)params;
    transistor_params *p1 = p[0];
    transistor_params *p2 = p[1];

    return ids(p1) + ids(p2);
}

double findRoot()
{
    constexpr int max_iter = 100;
    int iter = 0;
    double x_lo = 1.0, x_hi = 12.0;
    model_params params = { 0 };

    gsl_function F;
    F.function = &ekv;
    F.params = &params;

    const gsl_root_fsolver_type *T = gsl_root_fsolver_brent;
    gsl_root_fsolver *s = gsl_root_fsolver_alloc (T);
    gsl_root_fsolver_set (s, &F, x_lo, x_hi);

    printf ("using %s method\n", 
            gsl_root_fsolver_name (s));

    printf ("%5s [%9s, %9s] %9s %9s\n",
            "iter", "lower", "upper", "root", 
            "err(est)");

    int status;
    double r;
    do
        {
        iter++;
        status = gsl_root_fsolver_iterate (s);
        r = gsl_root_fsolver_root (s);
        x_lo = gsl_root_fsolver_x_lower (s);
        x_hi = gsl_root_fsolver_x_upper (s);
        status = gsl_root_test_interval (x_lo, x_hi,
                                        0, 0.001);

        if (status == GSL_SUCCESS)
            printf ("Converged:\n");

        printf ("%5d [%.7f, %.7f] %.7f %.7f\n",
                iter, x_lo, x_hi,
                r,
                x_hi - x_lo);
        }
    while (status == GSL_CONTINUE && iter < max_iter);
    return r;
}

int main() {

    constexpr double VOLTAGE_SKEW = 1.015;

    constexpr double Vdd = 12. * VOLTAGE_SKEW;
    constexpr double Vt = 1.31;
    constexpr double uCox = 20e-6;

    double Vo = 1.33;

    double Vx;

    for (double Vi = 5.60; Vi > 2.4; Vi -= 0.1)
    {
        for (;;)
        {
            {
                // Vx = ((K1*Vit + K2*Vt) + sqrt((K1*Vit + K2*Vt)^2 - (K1+K2)*(K1*Vit^2 - K2*Vot^2 + K2*Vt^2)) / (K1+K2)
                constexpr double WL1 = 73./19.;
                constexpr double WL2 = 26./58.;

                constexpr double K1 = uCox/2. * WL1;
                constexpr double K2 = uCox/2. * WL2;

                assert(Vi - Vx > Vt); // M1 is in subthreshold mode
                const double Vit = Vi - Vt;
                assert(Vo > Vt); // M2 is in subthreshold mode
                const double Vot = Vo - Vt;

                const double a = K1+K2;
                const double b = K1*Vit + K2*Vt;
                const double c = K1*Vit*Vit - K2*Vo*Vo + 2.*K2*Vo*Vt;//K1*Vit*Vit - K2*Vot*Vot + K2*Vt*Vt;
                Vx = (b + std::sqrt(b*b - a*c))/a;
                std::cout << "Vx: " << Vx << std::endl;
            }

            double oldVo = Vo;

            {
                // Vo = ((K1*Vddt + K2*Vxt) + sqrt((K1*Vddt + K2*Vxt)^2 - (K1+K2)*K1*Vddt^2))/(K1+K2)
                constexpr double WL1 = 40./19.;
                constexpr double WL2 = 664./19.;

                constexpr double K1 = uCox/2. * WL1;
                constexpr double K2 = uCox/2. * WL2;

                const double Vddt = Vdd - Vt;
                assert(Vx > Vt); // M2 is in subthreshold mode
                const double Vxt = Vx - Vt;

                const double a = K1+K2;
                const double b = K1*Vddt + K2*Vxt;
                const double c = K1*Vddt*Vddt;
                Vo = (b + std::sqrt(b*b - a*c))/a;
                std::cout << "Vo: " << Vo << std::endl;
            }

            if (std::abs(Vo - oldVo) < 1e-6)
                break;
        }
        std::cout << std::fixed << std::setprecision(1) << Vi << " -> " << std::setprecision(3) << Vo << std::endl;
    }
}
