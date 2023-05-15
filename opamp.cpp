/*
 * This file is part of libsidplayfp, a SID player engine.
 *
 * Copyright 2023 Leandro Nini <drfiemost@users.sourceforge.net>
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

#include <cassert>
#include <ctime>
#include <cstdlib>

#include <iostream>
#include <sstream>
#include <fstream>
#include <string>
#include <vector>
#include <limits>
#include <random>

#include "parameters.h"

/**
 * This is the SID 6581 op-amp voltage transfer function, measured on
 * CAP1B/CAP1A on a chip marked MOS 6581R4AR 0687 14.
 * All measured chips have op-amps with output voltages (and thus input
 * voltages) within the range of 0.81V - 10.31V.
 */
const std::vector<data_t> opamp_voltage6581 =
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

/**
 * This is the SID 8580 op-amp voltage transfer function, measured on
 * CAP1B/CAP1A on a chip marked CSG 8580R5 1690 25.
 */
const std::vector<data_t> opamp_voltage8580 =
{
    {  1.30,  8.91 },  // Approximate start of actual range
    {  4.76,  8.91 },
    {  4.77,  8.90 },
    {  4.78,  8.88 },
    {  4.785, 8.86 },
    {  4.79,  8.80 },
    {  4.795, 8.60 },
    {  4.80,  8.25 },
    {  4.805, 7.50 },
    {  4.81,  6.10 },
    {  4.815, 4.05 },  // Change of curvature
    {  4.82,  2.27 },
    {  4.825, 1.65 },
    {  4.83,  1.55 },
    {  4.84,  1.47 },
    {  4.85,  1.43 },
    {  4.87,  1.37 },
    {  4.90,  1.34 },
    {  5.00,  1.30 },
    {  5.10,  1.30 },
    {  8.91,  1.30 },  // Approximate end of actual range
};

static const double EPSILON = 1e-6;

#ifdef __MINGW32__
// MinGW's std::random_device is a PRNG seeded with a constant value
// so we use system time as a random seed.
#include <chrono>
inline long getSeed()
{
    using namespace std::chrono;
    const auto now_ms = time_point_cast<std::chrono::milliseconds>(system_clock::now());
    return now_ms.time_since_epoch().count();
}
#else
inline long getSeed()
{
    return std::random_device{}();
}
#endif

static std::default_random_engine prng(getSeed());
static std::normal_distribution<> normal_dist(1.0, 0.0001);
static std::normal_distribution<> normal_dist2(0.5, 0.2);

static double GetRandomValue()
{
    return normal_dist(prng);
}

static double GetNewRandomValue()
{
    return static_cast<double>(normal_dist2(prng));
}

static void Optimize(const ref_vector_t &reference, int chip)
{
    Parameters bestparams;

    switch (chip)
    {
    case 6581:
        // current score 1.2889417569511381
        bestparams.q = 5.5285312141864937e-05;
        bestparams.b = 2.1608922897100533;
        bestparams.v = 0.67181935418132133;
        break;
    case 8580:
        // current score 0.47707935622794362
        bestparams.q = 2.4325259082487039e-310;
        bestparams.b = 147.10522534153901;
        bestparams.v = 0.010293750527798712;
        break;
    default:
        break;
    }

    // Calculate current score
    score_t bestscore = bestparams.Score(chip, reference, true, 999999999);
    std::cout << "# initial score " << std::dec
        << bestscore << std::endl
        << bestparams.toString() << std::endl << std::endl;

    if (bestscore.error == 0)
        exit(EXIT_SUCCESS);

    /*
     * Start the Monte Carlo loop: we randomly alter parameters
     * and calculate the new score until we find the best fitting
     * function compared to the sampled data.
     */
    Parameters p = bestparams;
    for (;;)
    {
        // loop until at least one parameter has changed
        bool changed = false;
        while (!changed)
        {
            for (Param_t i = Param_t::Q; i <= Param_t::V; i++)
            {
                // change a parameter with 50% proability
                if (GetRandomValue() > 1.)
                {
                    const double oldValue = bestparams.GetValue(i);

                    //std::cout << newValue << " -> ";
                    double newValue = static_cast<double>(GetRandomValue()*oldValue);
                    //double newValue = oldValue + GetRandomValue();
                    //std::cout << newValue << std::endl;

                    // avoid negative values
                    if (i != Param_t::B && newValue <= 0.f)
                    {
                        newValue = EPSILON;
                    }
                    // try to avoid too small values
                    /*else if (newValue < EPSILON)
                        newValue += GetNewRandomValue();*/

                    p.SetValue(i, newValue);
                    changed = changed || oldValue != newValue;
                }
            }
        }

        // check new score
        const score_t score = p.Score(chip, reference, false, bestscore.error);
        if (bestscore.isBetter(score))
        {
            // accept if improvement
            std::cout << "# current score " << std::dec
                << score << std::endl
                << p.toString() << std::endl << std::endl;
            if (score.error == 0)
                exit(EXIT_SUCCESS);
            //p.reset();
            bestparams = p;
            bestscore = score;
        }
        else if (score.error == bestscore.error)
        {
            // no improvement but use new parameters as base to increase the "entropy"
            bestparams = p;
        }
    }
}

/**
 * Read sampled values for specific waveform and chip.
 */
static ref_vector_t ReadChip(int chip)
{
    std::cout << "Reading chip: " << chip << std::endl;

    const std::vector<data_t>* data;
    switch (chip) {
    case 6581:
        data = &opamp_voltage6581;
        break;
    case 8580:
        data = &opamp_voltage8580;
        break;
    default:
        std::cout << "Error!" << std::endl;
        exit(EXIT_FAILURE);
    }

    ref_vector_t result;
    for (data_t d: *data)
    {
        result.push_back(d);
    }
    return result;
}

int main(int argc, const char* argv[])
{
    if (argc != 2)
    {
        std::cout << "Usage " << argv[0] << " <chip>" << std::endl;
        exit(EXIT_FAILURE);
    }

    const int chip = atoi(argv[1]);
    assert(chip == 6581 || chip == 8580);

    ref_vector_t reference = ReadChip(chip);

#ifndef NDEBUG
    for (data_t d: reference)
        std::cout << d.Vin << " -> " << d.Vout << std::endl;
    std::cout << "---" << std::endl;
#endif

    srand(time(0));

    Optimize(reference, chip);
}
