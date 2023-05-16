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

#ifndef PARAMETERS_H
#define PARAMETERS_H

#include <cmath>

#include <vector>
#include <string>
#include <sstream>
#include <iomanip>
#include <limits>


// Model parameters
enum class Param_t
{
    Q,
    B,
    V
};

// define the postfix increment operator to allow looping over enum
inline Param_t& operator++(Param_t& x, int)
{
    return x = static_cast<Param_t>(static_cast<std::underlying_type<Param_t>::type>(x) + 1);
}

typedef struct
{
    double Vin;
    double Vout;
} data_t;

typedef std::vector<data_t> ref_vector_t;

struct score_t
{
    double error;

    score_t() :
        error(0.)
    {}

    bool isBetter(const score_t& newScore) const
    {
        return newScore.error < error;
    }
};

std::ostream & operator<<(std::ostream & os, const score_t & foo)
{
   os.precision(std::numeric_limits<double>::max_digits10);
   os << foo.error;
   return os;
}

class Parameters
{
public:
    double q, b, v;

public:
    Parameters() { reset(); }

    void reset()
    {
        q = 1.;
        b = 1.;
        v = 1.;
    }

    double GetValue(Param_t i)
    {
        switch (i)
        {
            case Param_t::Q: return q;
            case Param_t::B: return b;
            case Param_t::V: return v;
            default: return 0.; // Just to silence warning
        }
    }

    void SetValue(Param_t i, double d)
    {
        switch (i)
        {
            case Param_t::Q: q = d; break;
            case Param_t::B: b = d; break;
            case Param_t::V: v = d; break;
        }
    }

    std::string toString()
    {
        std::ostringstream ss;
        ss.precision(std::numeric_limits<double>::max_digits10);
        ss << "q = " << q << std::endl;
        ss << "b = " << b << std::endl;
        ss << "v = " << v << std::endl;
        return ss.str();
    }

private:
    double GetValue(double Vin) const
    {
        // https://en.wikipedia.org/wiki/Generalised_logistic_function
        // y = min + (max-min)/(1+Q*e^-B*x)^(1/v)
        return std::pow(1. + q*std::exp(b*Vin), 1./v);
    }

    double GetScore(double Vout, double Vref) const
    {
        double diff = (Vout - Vref)/Vref;
        return diff * diff;
    }

public:
    score_t Score(const ref_vector_t &reference, bool print, unsigned int bestscore)
    {
        score_t score;

        const double Vmin = reference[0].Vin;
        const double Vmax = reference[0].Vout;

        double error = 0.;

        for (data_t data: reference)
        {
            // Calculate score
            const double simval = Vmin + (Vmax-Vmin)/GetValue(data.Vin-Vmin);
            const double err = GetScore(simval, data.Vout);
            error += err;

            if (print)
            {
                std::cout
                          << simval << " "
                          << data.Vout << " ("
                          << err << ")"
                          << std::endl;
            }
        }

        score.error = std::sqrt(error);

        if (print)
        {
            std::cout << "Error: " << score.error << std::endl;
        }

        return score;
    }
};

#endif
