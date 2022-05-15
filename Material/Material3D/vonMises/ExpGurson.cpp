/*******************************************************************************
 * Copyright (C) 2017-2022 Theodore Chang
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

#include "ExpGurson.h"

constexpr unsigned ExpGurson::max_iteration = 20;

ExpGurson::ExpGurson(const unsigned T, const double E, const double V, const double YS, const double N, const double Q1, const double Q2, const double FN, const double SN, const double EN, const double R)
    : DataExpGurson{fabs(YS), std::min(1., N)}
    , NonlinearGurson(T, E, V, Q1, Q2, FN, SN, EN, R) {}

vec ExpGurson::compute_hardening(const double plastic_strain) const {
    auto k = 1.;
    double pow_term;

    unsigned counter = 0;
    while(true) {
        if(max_iteration == ++counter) {
            suanpan_error("NonlinearGurson cannot converge within %u iterations.\n", max_iteration);
            k = pow_term = 1.;
            break;
        }

        const auto tmp_term = k + para_c * plastic_strain;
        pow_term = pow(tmp_term, n - 1.);
        const auto incre = (k - pow_term * tmp_term) / (1. - n * pow_term);
        if(fabs(incre) <= tolerance) break;

        k -= incre;
    }

    pow_term *= n;

    return vec{k, pow_term * para_c / (1. - pow_term)} * yield_stress;
}

unique_ptr<Material> ExpGurson::get_copy() { return make_unique<ExpGurson>(*this); }

void ExpGurson::print() { suanpan_info("The Gurson model.\n"); }
