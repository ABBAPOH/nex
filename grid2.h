#ifndef __GRID2
#define __GRID2


#include "defs.h"
#include "fgrid2.h"


typedef struct {
	int left,right,up,down; 
	int id,nearSelf; 
	MPI_Comm comm,near; 
	int x,y; 
	Topology topo;} grid2;

void grid2FromNative(grid2 * g);
void grid2Fromfgrid2(grid2 * g,fgrid2 * fg);

void grid2Free(grid2 * g);

#endif
