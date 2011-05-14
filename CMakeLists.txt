project(nex)

cmake_minimum_required( VERSION 2.6 )

find_package( MPI REQUIRED )

#include( CMakeConfig.txt )

set( HEADERS 
    array.h
    box.h
    cache.h
    defs.h
    fgrid2.h
    fgrid3.h
    fgridn.h
    grid2.h
    main.h
    narray.h
    ntree.h
    range.h
    tasks.h
)

set( SOURCES
    array.c
    box.c
    cache.c
    fgrid2.c
    fgrid3.c
    fgridn.c
    grid2.c
    main.c
    narray.c
    ntree.c
    range.c
    tasks.c
)

if(NOT NUM_PROCS )
    set( NUM_PROCS 8 )
endif()

if( MPI_LIBRARY_PATH )
    link_directories( ${MPI_LIBRARY_PATH} )
endif()

if( MPI_INCLUDE_PATH )
    include_directories( ${MPI_INCLUDE_PATH} )
endif()

add_executable( nex ${HEADERS} ${SOURCES} )
target_link_libraries( nex mpi m )

enable_testing()

add_custom_target( run mpirun -n ${NUM_PROCS} ./nex )
