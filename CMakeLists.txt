project(nex)

cmake_minimum_required( VERSION 2.6 )

find_package( MPI REQUIRED )

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

add_executable( nex ${HEADERS} ${SOURCES} )
target_link_libraries( nex mpi m )

enable_testing()

add_custom_target( run mpirun -n 8 ./nex)
