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

#include "ConcreteTsai.h"

podarray<double> ConcreteTsai::compute_compression_initial_reverse() const {
    podarray<double> response(2);

    response(1) = compute_compression_backbone(response(0) = middle_point * c_strain)(0);

    return response;
}

podarray<double> ConcreteTsai::compute_tension_initial_reverse() const {
    podarray<double> response(2);

    response(1) = compute_tension_backbone(response(0) = middle_point * t_strain)(0);

    return response;
}

podarray<double> ConcreteTsai::compute_compression_backbone(const double n_strain) const {
    podarray<double> response(2);

    const auto normal_strain = std::max(datum::eps, n_strain / c_strain);
    const auto tmp_a = pow(normal_strain, c_n);
    const auto tmp_b = c_n == 1. ? 1. + (c_m - 1. + log(normal_strain)) * normal_strain : 1. + (c_m - c_n / (c_n - 1.)) * normal_strain + tmp_a / (c_n - 1.);
    response(0) = c_stress * c_m * normal_strain / tmp_b;
    response(1) = initial_stiffness(0) * (1. - tmp_a) / tmp_b / tmp_b;

    return response;
}

podarray<double> ConcreteTsai::compute_tension_backbone(const double n_strain) const {
    podarray<double> response(2);

    const auto normal_strain = std::max(datum::eps, n_strain / t_strain);
    const auto tmp_a = pow(normal_strain, t_n);
    const auto tmp_b = t_n == 1. ? 1. + (t_m - 1. + log(normal_strain)) * normal_strain : 1. + (t_m - t_n / (t_n - 1.)) * normal_strain + tmp_a / (t_n - 1.);
    response(0) = t_stress * t_m * normal_strain / tmp_b;
    response(1) = initial_stiffness(0) * (1. - tmp_a) / tmp_b / tmp_b;

    return response;
}

double ConcreteTsai::compute_compression_residual(const double reverse_c_strain, const double reverse_c_stress) const { return std::min(0., reverse_c_strain - reverse_c_stress * (reverse_c_strain / c_strain + .57) / (reverse_c_stress / c_strain + .57 * initial_stiffness(0))); }

double ConcreteTsai::compute_tension_residual(const double reverse_t_strain, const double reverse_t_stress) const { return std::max(0., reverse_t_strain - reverse_t_stress * (reverse_t_strain / t_strain + .67) / (reverse_t_stress / t_strain + .67 * initial_stiffness(0))); }

ConcreteTsai::ConcreteTsai(const unsigned T, const double CS, const double TS, const double MC, const double NC, const double MT, const double NT, const double MP, const double CE, const double TE, const double R)
    : SimpleHysteresis(T, MP, R)
    , c_stress(-fabs(CS))
    , c_strain(-fabs(CE) + datum::eps * 1E8)
    , c_m(std::max(1.1, MC))
    , c_n(std::max(1.1 * c_m / (c_m - 1.), NC))
    , t_stress(fabs(TS))
    , t_strain(fabs(TE) + datum::eps * 1E8)
    , t_m(std::max(1.1, MT))
    , t_n(std::max(1.1 * t_m / (t_m - 1.), NT)) {}

ConcreteTsai::ConcreteTsai(const unsigned T, const double CS, const double TS, const double MCT, const double NCT, const double MP, const double CE, const double TE, const double R)
    : ConcreteTsai(T, CS, TS, MCT, NCT, MCT, NCT, MP, CE, TE, R) {}

int ConcreteTsai::initialize(const shared_ptr<DomainBase>&) {
    trial_stiffness = current_stiffness = initial_stiffness = std::max(t_stress / t_strain * t_m, c_stress / c_strain * c_m);
    access::rw(c_m) = initial_stiffness(0) * c_strain / c_stress;
    access::rw(t_m) = initial_stiffness(0) * t_strain / t_stress;
    access::rw(c_n) = std::max(1.1 * c_m / (c_m - 1.), c_n);
    access::rw(t_n) = std::max(1.1 * t_m / (t_m - 1.), t_n);

    return SUANPAN_SUCCESS;
}

double ConcreteTsai::get_parameter(const ParameterType P) const {
    if(ParameterType::ELASTICMODULUS == P) return initial_stiffness(0);
    if(ParameterType::PEAKSTRAIN == P) return c_strain;
    if(ParameterType::CRACKSTRAIN == P) return t_strain;
    return 0.;
}

unique_ptr<Material> ConcreteTsai::get_copy() { return make_unique<ConcreteTsai>(*this); }
