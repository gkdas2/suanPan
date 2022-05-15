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

#include "PenaltyBC.h"
#include <Domain/DomainBase.h>
#include <Domain/Factory.hpp>
#include <Toolbox/utility.h>

/**
 * \brief default constructor.
 * \param T `unique_tag`
 * \param S `start_step`
 * \param N `nodes`
 * \param D `dofs`
 */
PenaltyBC::PenaltyBC(const unsigned T, const unsigned S, uvec&& N, const unsigned D)
    : Constraint(T, S, 0, std::forward<uvec>(N), {D}, 0) {}

/**
 * \brief the constructor uses DoF vector.
 * \param T `unique_tag`
 * \param S `start_step`
 * \param N `nodes`
 * \param D `dofs`
 */
PenaltyBC::PenaltyBC(const unsigned T, const unsigned S, uvec&& N, uvec&& D)
    : Constraint(T, S, 0, std::forward<uvec>(N), std::forward<uvec>(D), 0) {}

/**
 * \brief the constructor uses predefined TYPE: "XSYMM", "YSYMM", "ZSYMM", "ENCASTRE", "PINNED".
 * \param T `unique_tag`
 * \param S `start_step`
 * \param N `nodes`
 * \param TP PenaltyBC TYPE
 */
PenaltyBC::PenaltyBC(const unsigned T, const unsigned S, uvec&& N, const char* TP)
    : Constraint(T, S, 0, std::forward<uvec>(N), {}, 0) {
    if(is_equal(TP[0], 'X')) dof_reference = uvec{1, 5, 6};
    else if(is_equal(TP[0], 'Y')) dof_reference = uvec{2, 4, 6};
    else if(is_equal(TP[0], 'Z')) dof_reference = uvec{3, 4, 5};
    else if(is_equal(TP[0], 'E')) dof_reference = uvec{1, 2, 3, 4, 5, 6};
    else if(is_equal(TP[0], 'P')) dof_reference = uvec{1, 2, 3};
    else if(is_equal(TP[0], '1')) dof_reference = uvec{1};
    else if(is_equal(TP[0], '2')) dof_reference = uvec{2};
    else if(is_equal(TP[0], '3')) dof_reference = uvec{3};
    else if(is_equal(TP[0], '4')) dof_reference = uvec{4};
    else if(is_equal(TP[0], '5')) dof_reference = uvec{5};
    else if(is_equal(TP[0], '6')) dof_reference = uvec{6};

    dof_reference -= 1;
}

/**
 * \brief method to apply the PenaltyBC to the system.
 * \param D `Domain`
 * \return 0
 */
int PenaltyBC::process(const shared_ptr<DomainBase>& D) {
    dof_encoding = get_nodal_active_dof(D);

    for(const auto I : dof_encoding) D->insert_restrained_dof(I);

    stiffness.zeros(dof_encoding.n_elem, dof_encoding.n_elem);

    stiffness.diag().fill(multiplier * D->get_factory()->get_stiffness()->max());

    return SUANPAN_SUCCESS;
}

int PenaltyBC::process_resistance(const shared_ptr<DomainBase>& D) {
    for(const auto I : get_nodal_active_dof(D)) D->insert_restrained_dof(I);

    return SUANPAN_SUCCESS;
}
