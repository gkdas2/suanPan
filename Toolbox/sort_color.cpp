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

// ReSharper disable IdentifierTypo
#include "sort_color.h"
#include <set>
#include "metis/metis.h"
#include "utility.h"

auto sort_color_metis(suanpan_register& element_register, const int num_color, const char method) {
#ifdef SUANPAN_DEBUG
    wall_clock T;
    T.tic();
#endif

    const auto element_size = element_register.size();

    std::atomic num_edges = 0llu;
    suanpan_for_each(element_register.begin(), element_register.end(), [&](const suanpan_set& element) { num_edges += element.size(); });

    vector<idx_t> xadj;
    xadj.reserve(element_size + 1llu);
    xadj.emplace_back(0);
    vector<idx_t> adjncy;
    adjncy.reserve(num_edges);

    for(const auto& element : element_register) {
        adjncy.insert(adjncy.end(), element.begin(), element.end());
        xadj.emplace_back(xadj.back() + static_cast<idx_t>(element.size()));
    }

    auto nvtxs = static_cast<idx_t>(element_size);
    idx_t ncon = 1;
    auto nparts = static_cast<idx_t>(num_color);
    idx_t edgecut = 0;

    vector<int> part(nvtxs);

    idx_t* vsize = nullptr;
    real_t* tpwgts = nullptr;
    real_t* ubvec = nullptr;

    idx_t options[METIS_NOPTIONS];
    METIS_SetDefaultOptions(options);
#ifdef SUANPAN_DEBUG
    options[METIS_OPTION_DBGLVL] = METIS_DBG_INFO | METIS_DBG_TIME;
#endif
    options[METIS_OPTION_OBJTYPE] = METIS_OBJTYPE_VOL;

    if('K' == method) METIS_PartGraphKway(&nvtxs, &ncon, xadj.data(), adjncy.data(), nullptr, vsize, nullptr, &nparts, tpwgts, ubvec, options, &edgecut, part.data());
    else METIS_PartGraphRecursive(&nvtxs, &ncon, xadj.data(), adjncy.data(), nullptr, vsize, nullptr, &nparts, tpwgts, ubvec, options, &edgecut, part.data());

#ifdef SUANPAN_DEBUG
    suanpan_debug("Coloring algorithm takes %.5E seconds.\n", T.toc());
#endif

    return part;
}

vector<vector<unsigned>> sort_color_wp(const suanpan_register& node_register) {
#ifdef SUANPAN_DEBUG
    wall_clock T;
    T.tic();
#endif

    const auto num_node = node_register.size();

    uvec weight(num_node, fill::none);

    suanpan_for(static_cast<size_t>(0), node_register.size(), [&](const size_t I) { weight(I) = node_register[I].size(); });

    auto comparator = [&](const unsigned A, const unsigned B) { return weight[A] > weight[B]; };

    std::multiset<unsigned, decltype(comparator)> degree(comparator);
    for(auto I = 0u; I < num_node; ++I) degree.insert(I);

    vector<vector<unsigned>> color_map;

    while(!degree.empty()) {
        color_map.emplace_back();
        auto& color = color_map.back();

        color.emplace_back(*degree.begin());
        for(auto node = degree.erase(degree.begin()); node != degree.end();) {
            auto flag = false;
            for(const auto colored : color) {
                if(colored == *node) continue;
                if(node_register[colored].contains(*node)) {
                    flag = true;
                    break;
                }
            }
            if(flag) ++node;
            else {
                color.emplace_back(*node);
                node = degree.erase(node);
            }
        }
    }

    for(auto& color : color_map) color.shrink_to_fit();
    color_map.shrink_to_fit();

#ifdef SUANPAN_DEBUG
    suanpan_debug("Coloring algorithm takes %.5E seconds.\n", T.toc());
#endif

    return color_map;
}

vector<vector<unsigned>> sort_color_mis(const suanpan_register& node_register) {
#ifdef SUANPAN_DEBUG
    wall_clock T;
    T.tic();
#endif

    const auto num_node = node_register.size();

    uvec weight(num_node, fill::none);

    suanpan_for(static_cast<size_t>(0), node_register.size(), [&](const size_t I) { weight(I) = node_register[I].size(); });

    uword counter = num_node;
    for(const auto I : sort_index(weight, "descend").eval()) weight[I] = --counter;

    auto comparator = [&](const unsigned A, const unsigned B) { return weight[A] > weight[B]; };

    std::multiset<unsigned, decltype(comparator)> degree(comparator);
    for(auto I = 0u; I < num_node; ++I) degree.insert(I);

    vector<vector<unsigned>> color_map;

    while(!degree.empty()) {
        color_map.emplace_back();
        auto& color = color_map.back();

        auto degree_copy = degree;
        do {
            const auto node_to_color = *degree_copy.begin();
            color.emplace_back(node_to_color);
            degree.erase(node_to_color);
            for(auto neighbour : node_register[node_to_color]) degree_copy.erase(neighbour);
        }
        while(!degree_copy.empty());
    }

    for(auto& color : color_map) color.shrink_to_fit();
    color_map.shrink_to_fit();

#ifdef SUANPAN_DEBUG
    suanpan_debug("Coloring algorithm takes %.5E seconds.\n", T.toc());
#endif

    return color_map;
}
