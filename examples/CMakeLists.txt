set( HEADERS
    main.h
)
set( SOURCES
    main.c
)

include_directories( ../lib )

add_executable( nex ${HEADERS} ${SOURCES} )
target_link_libraries( nex mpi m libnex )
