project(nex)

cmake_minimum_required( VERSION 2.6 )

find_package( MPI REQUIRED )

add_subdirectory( lib )
add_subdirectory( examples )

enable_testing()

if(NOT NUM_PROCS )
    set( NUM_PROCS 8 )
endif()

add_custom_target( run mpirun -n ${NUM_PROCS} examples/nex )
