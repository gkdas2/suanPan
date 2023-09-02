include_directories(${ROOT})
include_directories(Include)
include_directories(Include/metis)
include_directories(Include/fmt/include)

set(CMAKE_BUILD_TYPE "Release" CACHE STRING "Debug Release RelWithDebInfo MinSizeRel")

message(STATUS "CMAKE_BUILD_TYPE: ${CMAKE_BUILD_TYPE}")

set(BUILD_PACKAGE "" CACHE STRING "DEB OR RPM")

option(BUILD_DLL_EXAMPLE "Build dynamic linked library examples." OFF)
option(BUILD_MULTITHREAD "Build with multi-threaded support via TBB." OFF)
option(BUILD_SHARED "Build all libraries as shared ones." OFF)
option(USE_SUPERLUMT "Use multi-threaded SuperLU. Note the performance may not be better than the sequential version." OFF)
option(USE_EXTERNAL_VTK "Enable visualisation via VTK. Note external VTK libraries need to be compiled in advance." OFF)
option(USE_HDF5 "Enable recording results in HDF5 format." ON)
option(USE_AVX "Enable AVX support." ON)
option(USE_AVX2 "Enable AVX2 support." OFF)
option(USE_AVX512 "Enable AVX512 support." OFF)
option(USE_MKL "Use Intel MKL instead of OpenBLAS." OFF)
option(USE_MIMALLOC "Use mimalloc instead of default memory allocator." OFF)
if (USE_MKL)
    option(USE_INTEL_OPENMP "Use Intel OpenMP implementation on Linux and macOS" ON)
    option(LINK_DYNAMIC_MKL "Link dynamic Intel MKL libraries." ON)
    option(USE_MPI "Enable MPI based global solvers." OFF)
    if (USE_MPI)
        option(USE_INTEL_MPI "Use Intel MPI implementation on Linux and macOS." ON)
        if (NOT USE_INTEL_MPI)
            set(MPI_INC "OpenMPI include path." CACHE PATH "")
            set(MPI_LIB "OpenMPI lib path." CACHE PATH "")
        endif ()
    endif ()
else ()
    set(USE_MPI OFF CACHE BOOL "" FORCE)
endif ()

set(COMPILER_IDENTIFIER "unknown")
set(SP_EXTERNAL_LIB_PATH "unknown")
if (CMAKE_SYSTEM_NAME MATCHES "Windows") # WINDOWS PLATFORM
    if (CMAKE_CXX_COMPILER_ID MATCHES "GNU") # GNU GCC COMPILER
        set(COMPILER_IDENTIFIER "gcc-win")
        set(SP_EXTERNAL_LIB_PATH "win")
    elseif (CMAKE_CXX_COMPILER_ID MATCHES "MSVC" OR CMAKE_CXX_COMPILER_ID MATCHES "Intel") # MSVC COMPILER
        set(COMPILER_IDENTIFIER "vs")
        set(SP_EXTERNAL_LIB_PATH "vs")
        add_compile_definitions(_SILENCE_CXX17_ITERATOR_BASE_CLASS_DEPRECATION_WARNING)
        if (FORTRAN_STATUS)
            set(BUILD_SHARED OFF CACHE BOOL "" FORCE)
        endif ()
        option(USE_EXTERNAL_CUDA "Enable GPU based global solvers via CUDA." OFF)
    endif ()
elseif (CMAKE_SYSTEM_NAME MATCHES "Linux") # LINUX PLATFORM
    set(SP_EXTERNAL_LIB_PATH "linux")
    if (CMAKE_CXX_COMPILER_ID MATCHES "GNU") # GNU GCC COMPILER
        set(COMPILER_IDENTIFIER "gcc-linux")
    elseif (CMAKE_CXX_COMPILER_ID MATCHES "IntelLLVM") # Intel COMPILER icpx
        set(COMPILER_IDENTIFIER "clang-linux")
    elseif (CMAKE_CXX_COMPILER_ID MATCHES "Intel") # Intel COMPILER Classic icc
        set(COMPILER_IDENTIFIER "gcc-linux")
        message(STATUS "Classic Intel compiler icc has incomplete CPP20 support, if it fails to compile please use another compiler.")
    elseif (CMAKE_CXX_COMPILER_ID MATCHES "Clang") # Clang COMPILER
        set(COMPILER_IDENTIFIER "clang-linux")
    endif ()
    option(USE_EXTERNAL_CUDA "Enable GPU based global solvers via CUDA." OFF)
elseif (CMAKE_SYSTEM_NAME MATCHES "Darwin") # MAC PLATFORM
    set(SP_EXTERNAL_LIB_PATH "mac")
    if (CMAKE_CXX_COMPILER_ID MATCHES "GNU") # GNU GCC COMPILER
        set(COMPILER_IDENTIFIER "gcc-mac")
    elseif (CMAKE_CXX_COMPILER_ID MATCHES "Clang")
        set(COMPILER_IDENTIFIER "clang-mac")
        link_directories(/usr/local/opt/libomp/lib/)
        message(STATUS "On macOS, make sure llvm and libomp are installed.")
        message(STATUS "brew install llvm libomp")
    endif ()
endif ()

if (COMPILER_IDENTIFIER MATCHES "unknown")
    message(FATAL_ERROR "Cannot identify the compiler available, please use GCC or MSVC or Intel.")
endif ()

if (USE_MKL AND USE_EXTERNAL_CUDA)
    option(USE_MAGMA "Enable GPU based global solvers via MAGMA." OFF)
endif ()

link_directories(Libs/${SP_EXTERNAL_LIB_PATH})

if (USE_SUPERLUMT)
    message(WARNING "Current SuperLU MT library may contain bugs. Disabling it.")
    set(USE_SUPERLUMT OFF CACHE BOOL "" FORCE)
    # add_compile_definitions(SUANPAN_SUPERLUMT)
endif ()

if (USE_MKL)
    set(MKLROOT "" CACHE PATH "MKL library path which contains /include and /lib folders.")
    find_file(MKL_HEADER NAMES mkl.h PATHS ${MKLROOT}/include)
    if (MKL_HEADER MATCHES "MKL_HEADER-NOTFOUND")
        message(FATAL_ERROR "The <mkl.h> is not found under the path: ${MKLROOT}/include.")
    endif ()
    add_compile_definitions(SUANPAN_MKL)
    # add_compile_definitions(ARMA_USE_MKL_ALLOC)
    include_directories(${MKLROOT}/include)
    link_directories(${MKLROOT}/lib/intel64)
    if (USE_MPI)
        add_compile_definitions(SUANPAN_MPI)
        if (USE_INTEL_MPI)
            add_compile_definitions(MPICH_SKIP_MPICXX)
            include_directories(${MKLROOT}/../../mpi/latest/include)
            link_directories(${MKLROOT}/../../mpi/latest/lib/release)
        elseif (MPI_INC STREQUAL "" OR MPI_LIB STREQUAL "")
            message(FATAL_ERROR "Please specify the MPI_INC and MPI_LIB paths.")
        else ()
            include_directories(${MPI_INC})
            link_directories(${MPI_LIB})
        endif ()
    endif ()
    if (COMPILER_IDENTIFIER MATCHES "IntelLLVM")
        set(USE_INTEL_OPENMP ON CACHE BOOL "" FORCE)
    endif ()
    if (USE_INTEL_OPENMP)
        if (MKLROOT MATCHES "(oneapi|oneAPI)")
            if (COMPILER_IDENTIFIER MATCHES "linux")
                find_library(IOMPPATH iomp5 ${MKLROOT}/../../compiler/latest/linux/compiler/lib/intel64_lin)
                get_filename_component(IOMPPATH ${IOMPPATH} DIRECTORY)
                link_directories(${IOMPPATH})
            elseif (COMPILER_IDENTIFIER MATCHES "(win|vs)")
                find_library(IOMPPATH libiomp5md ${MKLROOT}/../../compiler/latest/windows/compiler/lib/intel64_win)
                get_filename_component(IOMPPATH ${IOMPPATH} DIRECTORY)
                link_directories(${IOMPPATH})
            endif ()
        else ()
            message(FATAL_ERROR "Intel Parallel Studio is not supported, please install Intel oneAPI toolkits.")
        endif ()
    endif ()
endif ()

if (USE_EXTERNAL_CUDA)
    cmake_policy(SET CMP0146 OLD)
    find_package(CUDA)
    if (NOT CUDA_FOUND)
        set(CUDA_PATH "" CACHE PATH "CUDA library path which contains /include folder.")
        find_package(CUDA PATHS ${CUDA_PATH})
        if (NOT CUDA_FOUND)
            message(FATAL_ERROR "CUDA library is not found, please indicate its path.")
        endif ()
    endif ()
    add_compile_definitions(SUANPAN_CUDA)
    include_directories(${CUDA_INCLUDE_DIRS})
    link_libraries(${CUDA_LIBRARIES} ${CUDA_CUBLAS_LIBRARIES} ${CUDA_cusolver_LIBRARY} ${CUDA_cusparse_LIBRARY})
endif ()

if (USE_MAGMA)
    set(MAGMAROOT "" CACHE PATH "Magma library path which contains /include and /lib folders.")
    find_file(MAGMA_HEADER NAMES magma.h PATHS ${MAGMAROOT}/include)
    if (MAGMA_HEADER MATCHES "MAGMA_HEADER-NOTFOUND")
        message(FATAL_ERROR "The <magma.h> is not found under the path: ${MAGMAROOT}/include.")
    endif ()
    include_directories(${MAGMAROOT}/include)
    link_directories(${MAGMAROOT}/lib)
    link_libraries(magma magma_sparse)
    add_compile_definitions(SUANPAN_MAGMA)
endif ()

set(HAVE_VTK FALSE CACHE INTERNAL "")
if (USE_EXTERNAL_VTK)
    if (VTK_PATH MATCHES "")
        find_package(VTK)
    else ()
        find_package(VTK PATHS ${VTK_PATH})
    endif ()
    if (VTK_FOUND)
        add_compile_definitions(SUANPAN_VTK)
        set(HAVE_VTK TRUE CACHE INTERNAL "")
    else ()
        set(VTK_PATH "" CACHE PATH "VTK library path which contains /include folder.")
        find_package(VTK PATHS ${VTK_PATH})
        if (NOT VTK_FOUND)
            message(FATAL_ERROR "VTK library is not found, please indicate its path.")
        endif ()
    endif ()
endif ()

if (USE_HDF5)
    add_compile_definitions(SUANPAN_HDF5)
    if (HAVE_VTK)
        string(REGEX REPLACE "/lib6?4?/cmake/vtk" "/include/vtk" VTK_INCLUDE ${VTK_DIR})
        include_directories(${VTK_INCLUDE}/vtkhdf5)
        include_directories(${VTK_INCLUDE}/vtkhdf5/src)
        include_directories(${VTK_INCLUDE}/vtkhdf5/hl/src)
    else ()
        include_directories(Include/hdf5)
        include_directories(Include/hdf5-${SP_EXTERNAL_LIB_PATH})
        if (COMPILER_IDENTIFIER MATCHES "vs")
            link_libraries(libhdf5_hl libhdf5)
        else ()
            link_libraries(hdf5_hl hdf5)
        endif ()
    endif ()
else ()
    add_compile_definitions(ARMA_DONT_USE_HDF5)
endif ()

if (USE_MIMALLOC)
    message(STATUS "USING MIMALLOC LIBRARY")
    include(FetchContent)
    FetchContent_Declare(mimalloc
            GIT_REPOSITORY https://github.com/microsoft/mimalloc
            GIT_TAG v2.1.2)
    FetchContent_MakeAvailable(mimalloc)
endif ()

if (BUILD_MULTITHREAD)
    message(STATUS "USING TBB LIBRARY")
    add_compile_definitions(SUANPAN_MT)
    if (COMPILER_IDENTIFIER MATCHES "gcc-win")
        link_libraries(tbb12)
    else ()
        link_libraries(tbb)
    endif ()
    option(USE_TBB_ALLOC "Use tbb memory allocator. Enable if no other allocators will be used." OFF)
    if (USE_TBB_ALLOC)
        # for armadillo to use tbb allocator
        add_compile_definitions(ARMA_USE_TBB_ALLOC)
        include_directories(Include/oneapi) # because armadillo assumes oneapi be in the include path
        link_libraries(tbbmalloc tbbmalloc_proxy)
    endif ()
endif ()

if (BUILD_SHARED)
    message(STATUS "BUILD SHARED LIBRARY")
    set(LIBRARY_TYPE SHARED)
else ()
    message(STATUS "BUILD STATIC LIBRARY")
    set(LIBRARY_TYPE STATIC)
endif ()

if (COMPILER_IDENTIFIER MATCHES "vs")
    unset(TEST_COVERAGE CACHE)

    link_directories(Libs/win)

    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} /W4 /MP /openmp")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /W4 /MP /openmp /EHsc")
    set(CMAKE_Fortran_FLAGS "${CMAKE_Fortran_FLAGS} /nowarn /MP /Qopenmp /Qparallel /fpp /names:lowercase /assume:underscore /libs:dll /threads")

    if (USE_AVX512)
        set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} /arch:AVX512")
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /arch:AVX512")
        set(CMAKE_Fortran_FLAGS "${CMAKE_Fortran_FLAGS} /arch:AVX")
    elseif (USE_AVX2)
        set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} /arch:AVX2")
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /arch:AVX2")
        set(CMAKE_Fortran_FLAGS "${CMAKE_Fortran_FLAGS} /arch:AVX")
    elseif (USE_AVX)
        set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} /arch:AVX")
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /arch:AVX")
        set(CMAKE_Fortran_FLAGS "${CMAKE_Fortran_FLAGS} /arch:AVX")
    endif ()
else ()
    if (BUILD_SHARED)
        set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fPIC")
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC")
        set(CMAKE_Fortran_FLAGS "${CMAKE_Fortran_FLAGS} -fPIC")
    endif ()

    link_libraries(dl pthread gfortran quadmath)

    if (CMAKE_CXX_COMPILER_ID MATCHES "IntelLLVM")
        link_libraries(stdc++)
        set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -ffp-model=precise -fexceptions -fiopenmp")
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -ffp-model=precise -fexceptions -fiopenmp")
    elseif (COMPILER_IDENTIFIER MATCHES "clang-mac")
        include_directories("/usr/local/include" "/usr/local/opt/llvm/include")
        link_directories("/usr/local/lib" "/usr/local/opt/llvm/lib")
        link_libraries(omp)
        set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fexceptions -Xpreprocessor -fopenmp")
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fexceptions -Xpreprocessor -fopenmp")
    else ()
        set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fexceptions -fopenmp")
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fexceptions -fopenmp")
    endif ()

    if (USE_AVX512)
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -mavx512f")
        set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -mavx512f")
    elseif (USE_AVX2)
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -mavx2")
        set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -mavx2")
    elseif (USE_AVX)
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -mavx")
        set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -mavx")
    endif ()

    option(TEST_COVERAGE "TEST CODE COVERAGE USING GCOV" OFF)

    if (TEST_COVERAGE AND COMPILER_IDENTIFIER MATCHES "gcc") # only report coverage with gcc
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fprofile-arcs -ftest-coverage")
        link_libraries(gcov)
    endif ()

    if (CMAKE_BUILD_TYPE MATCHES "Debug")
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O1")
        option(USE_ASAN "USE ADDRESS SANITIZER" OFF)
        if (USE_ASAN)
            message(STATUS "Using the address sanitizer with flags: -fsanitize=address,leak,undefined")
            set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=address,leak,undefined")
            set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fsanitize=address,leak,undefined")
        endif ()
    endif ()

    set(CMAKE_Fortran_FLAGS "${CMAKE_Fortran_FLAGS} -w -fallow-argument-mismatch")
    if (CMAKE_Fortran_COMPILER_ID MATCHES "Intel")
        set(CMAKE_Fortran_FLAGS "${CMAKE_Fortran_FLAGS} -fpp -qopenmp")
    else ()
        set(CMAKE_Fortran_FLAGS "${CMAKE_Fortran_FLAGS} -cpp -fopenmp")
    endif ()
endif ()
