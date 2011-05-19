set( HEADERS 
    array.h
    box.h
    cache.h
    defs.h
    fgrid2.h
    fgrid3.h
    fgridn.h
    grid2.h
    narray.h
    nloop.h
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
    narray.c
    nloop.c
    ntree.c
    range.c
    tasks.c
)

if( MPI_LIBRARY_PATH )
    link_directories( ${MPI_LIBRARY_PATH} )
endif()

if( MPI_INCLUDE_PATH )
    include_directories( ${MPI_INCLUDE_PATH} )
endif()

add_library( libnex ${HEADERS} ${SOURCES} )
target_link_libraries( libnex mpi m )
