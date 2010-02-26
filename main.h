#ifndef __MAIN
#define __MAIN

#include "defs.h"
#include "fgrid2.h"
#include "fgrid3.h"
#include "grid2.h"
#include "ntree.h"
#include "array.h"
#include "range.h"
#include "box.h"

int numtasks,rank;

void readConfig();
int Native(int x,int y);
void reverseNative(int rank);

#endif
