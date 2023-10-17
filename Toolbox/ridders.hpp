/*******************************************************************************
 * Copyright (C) 2017-2023 Theodore Chang
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 ******************************************************************************/

#ifndef RIDDERS_HPP
#define RIDDERS_HPP

#include <suanPan.h>

template<std::invocable<double> T> double ridders(const T& func, double x1, double f1, double x2, double f2, const double tolerance) {
    double target;

    auto counter = 0u;
    while(true) {
        counter += 2u;

        const auto x3 = .5 * (x1 + x2);
        const auto f3 = func(x3);
        if(std::fabs(f3) < tolerance || fabs(x2 - x1) < tolerance) {
            target = x3;
            break;
        }

        const auto dx = (x3 - x1) * f3 / std::sqrt(f3 * f3 - f1 * f2);

        const auto x4 = f1 > f2 ? x3 + dx : x3 - dx;
        const auto f4 = func(x4);
        if(std::fabs(f4) < tolerance) {
            target = x4;
            break;
        }

        // one end is x4
        // pick the other from x3, x2, x1
        if(f4 * f3 < 0.) {
            x1 = x3;
            f1 = f3;
        }
        else if(f4 * f2 < 0.) {
            x1 = x2;
            f1 = f2;
        }

        x2 = x4;
        f2 = f4;
    }

    suanpan_debug("Ridders' method initial guess {:.5E} with {} iterations.\n", target, counter);

    return target;
}

#endif

//! @}
