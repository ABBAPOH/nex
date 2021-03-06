#ifndef __NLOOP
#define __NLOOP

#include "defs.h"
#include "box.h"
#include "fgridn.h"
#include "narray.h"
#include "box.h"

typedef struct nlooptemp nloop;
typedef int (*nloop_work_func)(nloop * loop, int index[], void* data);
typedef int (*nloop_depend_func)(nloop * loop, int index[], int indexDep[], void* data);

struct nlooptemp
{
	narray* status;
	nloop_work_func func;
	nloop_depend_func depend;
	int localDone;
	BoxHeader* remoteStatuses;
	Topology topo;
};

void nloopFromTopo(nloop *loop, Topology topo, int sizes[], int dimsCount, npointer (*map)(int[], narray*), void (*alloc)(narray*), nloop_work_func func, nloop_depend_func depend);
void nloopFree(nloop *loop);

void nloopOffsetToIndex(nloop *loop, int offset, int index[]);
long long nloopIndexToGlobalOffset(nloop *loop, int index[]);

int nloopDo(nloop *loop, int index[], void* data);
int nloopDoWithDeps(nloop *loop, int index[], void* data, int askRemote);
int nloopDoLocal(nloop *loop, void* data);
int nloopCheckStatus(nloop *loop, int index[]);
int nloopClearStatus(nloop *loop);

void nloopStatusFence(nloop* loop);
void nloopBarrier(nloop* loop);

#endif
