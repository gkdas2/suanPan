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

#include "NodalDisplacement.h"
#include <Domain/DomainBase.h>
#include <Domain/Factory.hpp>
#include <Load/Amplitude/Amplitude.h>

NodalDisplacement::NodalDisplacement(const unsigned T, const unsigned ST, const double L, uvec&& N, const unsigned D, const unsigned AT)
    : Load(T, ST, AT, std::forward<uvec>(N), uvec{D}, L) { enable_displacement_control(); }

NodalDisplacement::NodalDisplacement(const unsigned T, const unsigned ST, const double L, uvec&& N, uvec&& D, const unsigned AT)
    : Load(T, ST, AT, std::forward<uvec>(N), std::forward<uvec>(D), L) { enable_displacement_control(); }

int NodalDisplacement::initialize(const shared_ptr<DomainBase>& D) {
    set_end_step(start_step + 1);

    D->get_factory()->update_reference_dof(encoding = get_nodal_active_dof(D));

    return Load::initialize(D);
}

int NodalDisplacement::process(const shared_ptr<DomainBase>& D) {
    const auto& W = D->get_factory();

    trial_settlement.zeros(W->get_size());

    trial_settlement(encoding).fill(pattern * magnitude->get_amplitude(W->get_trial_time()));

    return SUANPAN_SUCCESS;
}
