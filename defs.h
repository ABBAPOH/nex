#ifndef __DEFS
#define __DEFS
#define AHAHAHAHA

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <math.h>
#include <mpi.h>

typedef enum {Tnone=0,Tgrid2,Tfgrid2,Tfgrid3,Tntree} topoType;
typedef struct {topoType type; void* obj;} Topo;

int nativeX,nativeY,X,Y;

#endif
