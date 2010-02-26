#include "grid2.h"

void grid2FromNative(grid2 * g)
{
	assert(g);
	
	MPI_Comm_dup(MPI_COMM_WORLD, &(g->comm));
	MPI_Comm_rank(g->comm,&(g->id));
	g->x=X;
	g->y=Y;
	
	g->up=Native(    (g->x+nativeX-1)%nativeX  ,    g->y);
	g->down=Native(  (g->x+1)%nativeX          ,    g->y);
	g->left=Native(  g->x                      ,    (g->y+nativeY-1)%nativeY);
	g->right=Native( g->x                      ,    (g->y+1)%nativeY);
	
	MPI_Group ogroup;
	MPI_Group ngroup;
	MPI_Comm_group(g->comm,&ogroup);
	int ranks[5]={g->id,g->up,g->right,g->down,g->left};
	MPI_Group_incl(ogroup,5,ranks,&(ngroup));
	MPI_Comm_create(g->comm,ngroup,&(g->near));
	MPI_Comm_rank(g->near,&(g->nearSelf));
	
	g->topo.obj=NULL;
	g->topo.type=Tnone;
}

void grid2Fromfgrid2(grid2 * g,fgrid2 * fg)
{
	assert(g);
	assert(fg);
	
	MPI_Comm_dup(fg->comm, &(g->comm));
	g->id=fg->id;
	g->x=fg->x;
	g->y=fg->y;
	
	g->up=fg->map( (g->x+fg->h-1)%fg->h , g->y , fg);
	g->down=fg->map( (g->x+1)%fg->h , g->y , fg);
	g->left=fg->map( g->x , (g->y+fg->w-1)%fg->w , fg);
	g->right=fg->map( g->x , (g->y+1)%fg->w , fg);
	
	MPI_Group ogroup;
	MPI_Group ngroup;
	MPI_Comm_group(g->comm,&ogroup);
	int ranks[5]={g->id,g->up,g->right,g->down,g->left};
	MPI_Group_incl(ogroup,5,ranks,&(ngroup));
	MPI_Comm_create(g->comm,ngroup,&(g->near));
	MPI_Comm_rank(g->near,&(g->nearSelf));
	
	g->topo.obj=fg;
	g->topo.type=Tfgrid2;
}

void grid2Free(grid2 * g)
{
	assert(g);
	
	MPI_Comm_free(&(g->comm));
	MPI_Comm_free(&(g->near));
	g->left=-1;
	g->right=-1;
	g->up=-1;
	g->down=-1;
	g->id=-1;
	g->nearSelf=-1;
	g->x=-1;
	g->y=-1;
	g->topo.type=Tnone;
	g->topo.obj=NULL;
}
