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

#include <Toolbox/argument.h>
#include <suanPan.h>
#include <lis/lislib.h>

#ifdef SUANPAN_WIN
#include <Windows.h>
#endif

#ifdef SUANPAN_MKL
extern "C" void mkl_free_buffers();
#else
void mkl_free_buffers() {}
#endif

// ReSharper disable once CppParameterMayBeConst
int main(int argc, char** argv) {
#ifdef SUANPAN_WIN
#if defined(SUANPAN_DEBUG) && defined(SUANPAN_MSVC)
    _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif
    SetConsoleCP(CP_UTF8);
    SetConsoleOutputCP(CP_UTF8);
#endif

    lis_initialize(nullptr, nullptr);

#ifdef SUANPAN_DEBUG
    argument_parser(argc, argv);
#else
    try { argument_parser(argc, argv); }
    catch(const std::exception& e) { suanpan_fatal("Some unexpected error happens: {}, please file a bug report via https://github.com/TLCFEM/suanPan/issues.\n", e.what()); }
#endif

    lis_finalize();

    return std::atexit(mkl_free_buffers);
}
