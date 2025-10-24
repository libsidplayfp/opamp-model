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

             Vdd         Vdd

              ┬           ┬
              │           │
              │           │
              │    ┌──────o
              │    │      │D
              │    │  │├──┘
              │    └──┤│
              │D     G│├──┐
          │├──┘           │S
  Vi ─────┤│              o───o───── Vo
         G│├──┐           │D  │
              │S  Vx  │├──┘   │
              o───────┤│      │
              │D     G│├──┐   │
          │├──┘           │S  │
       ┌──┤│              │   │
       │ G│├──┐           │   │
       │      │S          │   │
       │      │           │   │
       │      V           V   │
       │                      │
       │     GND         GND  │
       │                      │
       └──────────────────────┘


  Vdd - 12V
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

Transistor EKV model

Id = Is * (if - ir)

Is = 2*n*uCox*W/L*Ut^2

if = ln(1 + exp((Vp-Vs)/(2*Ut)))^2
ir = ln(1 + exp((Vp-Vd)/(2*Ut)))^2

Vp ~ (Vg - Vt)/n

*/
#include <gsl/gsl_errno.h>
#include <gsl/gsl_math.h>
#include <gsl/gsl_roots.h>

#include <iostream>
#include <iomanip>

#include <cassert>
#include <cmath>
#include <cstdio>

//#define DEBUG

using Point = struct
{
double x;
double y;
};

constexpr unsigned int OPAMP_SIZE = 33;

// Reference values, measured on CAP1B/CAP1A on a chip marked MOS 6581R4AR 0687 14:
constexpr Point opamp_voltage[OPAMP_SIZE] =
{
  {  0.81, 10.31 },  // Approximate start of actual range
  {  2.40, 10.31 },
  {  2.60, 10.30 },
  {  2.70, 10.29 },
  {  2.80, 10.26 },
  {  2.90, 10.17 },
  {  3.00, 10.04 },
  {  3.10,  9.83 },
  {  3.20,  9.58 },
  {  3.30,  9.32 },
  {  3.50,  8.69 },
  {  3.70,  8.00 },
  {  4.00,  6.89 },
  {  4.40,  5.21 },
  {  4.54,  4.54 },  // Working point (vi = vo)
  {  4.60,  4.19 },
  {  4.80,  3.00 },
  {  4.90,  2.30 },  // Change of curvature
  {  4.95,  2.03 },
  {  5.00,  1.88 },
  {  5.05,  1.77 },
  {  5.10,  1.69 },
  {  5.20,  1.58 },
  {  5.40,  1.44 },
  {  5.60,  1.33 },
  {  5.80,  1.26 },
  {  6.00,  1.21 },
  {  6.40,  1.12 },
  {  7.00,  1.02 },
  {  7.50,  0.97 },
  {  8.50,  0.89 },
  { 10.00,  0.81 },
  { 10.31,  0.81 },  // Approximate end of actual range
};

// Boltzmann Constant
constexpr double k = 1.380649e-23;
// charge of an electron
constexpr double q = 1.602176634e-19;

// temperature in °C
constexpr double temp = 60.;

// thermal voltage Ut = kT/q
constexpr double Ut = k * (temp + 273.15) / q;

// Transconductance coefficient
constexpr double uCox = 20e-6;

constexpr double VOLTAGE_SKEW = 1.015;

constexpr double Vdd = 12. * VOLTAGE_SKEW;

// Threshold voltage
//constexpr double Vt = 1.31;
constexpr double Vt0 = 1.31;

constexpr double gam = 1.0;  // body effect factor
constexpr double phi = 0.8;  // bulk Fermi potential FIXME negative for nmos?

constexpr double n = 1.0;

struct transistor_params
{
    double Vg, Vd, Vs;
    double WL;
};

struct model_params
{
    transistor_params m1, m2;
};

struct opamp_params
{
    transistor_params m1a, m2a, m1b, m2b;
};

double ids(transistor_params *p)
{
    double Vg = p->Vg;
    double Vd = p->Vd;
    double Vs = p->Vs;
    double Vt = Vt0;// + gam * (std::sqrt(std::abs(Vs + phi)) - std::sqrt(std::abs(phi)));
    double WL = p->WL;

    double Vp = (Vg - Vt) / n;

    double if_tmp = std::log1p(std::exp((Vp - Vs)/(2.*Ut)));
    double ir_tmp = std::log1p(std::exp((Vp - Vd)/(2.*Ut)));
    double is = 2. * n * uCox * WL * Ut*Ut;
    return is * (if_tmp*if_tmp - ir_tmp*ir_tmp);
}

double common_drain(double x, void *params)
{
    model_params *p = (model_params*)params;
    p->m1.Vs = p->m2.Vd = x;

    return ids(&p->m1) - ids(&p->m2);
}

double findRoot(model_params* params)
{
    constexpr int max_iter = 100;
    int iter = 0;
    double x_lo = -1.0, x_hi = 13.0;

    gsl_function F;
    F.function = &common_drain;
    F.params = params;

    const gsl_root_fsolver_type *T = gsl_root_fsolver_brent;
    gsl_root_fsolver *s = gsl_root_fsolver_alloc (T);
    gsl_root_fsolver_set (s, &F, x_lo, x_hi);
#ifdef DEBUG
    printf ("using %s method\n", 
            gsl_root_fsolver_name (s));

    printf ("%5s [%9s, %9s] %9s %9s\n",
            "iter", "lower", "upper", "root", 
            "err(est)");
#endif
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
                                        0, 0.0001);
#ifdef DEBUG
        if (status == GSL_SUCCESS)
            printf ("Converged:\n");

        printf ("%5d [%.7f, %.7f] %.7f %.7f\n",
                iter, x_lo, x_hi,
                r,
                x_hi - x_lo);
#endif
    }
    while ((status == GSL_CONTINUE) && (iter < max_iter));

    return r;
}

int main() {
    //double Vi = 4.54;
    double Vo = 10.00; // random initial value

    for (Point p: opamp_voltage)
    {
        double Vi = p.x;
        for (;;)
        {
            double Vx;

            {
                model_params params_common_drain = { 0 };
                params_common_drain.m1.WL = 80./20.;
                params_common_drain.m2.WL = 25./70.;
                params_common_drain.m1.Vg = Vi;
                params_common_drain.m2.Vg = Vo;
                params_common_drain.m1.Vd = Vdd;
                params_common_drain.m2.Vs = 0.; // GND

                Vx = findRoot(&params_common_drain);
                //std::cout << "Vx: " << std::fixed << std::setprecision(3) << Vx << std::endl;
            }

            double oldVo = Vo;

            {
                model_params params_common_source = { 0 };
                params_common_source.m1.WL = 40./20.;
                params_common_source.m2.WL = 650./20.;
                params_common_source.m1.Vg = Vdd;
                params_common_source.m2.Vg = Vx;
                params_common_source.m1.Vd = Vdd;
                params_common_source.m2.Vs = 0.; // GND

                Vo = findRoot(&params_common_source);
                //std::cout << "Vo: " << std::fixed << std::setprecision(3) << Vo << std::endl;
            }

            if (std::abs(Vo - oldVo) < 1e-6)
                break;
        }
        std::cout << std::fixed << std::setprecision(2) << Vi
            << ", " << std::setprecision(3) << Vo
            << " (" << p.y << ")" << std::endl;
    }
}
