#include "main.h"

void readConfig()
{
	//printf("Read CFG\n");
	MPI_Comm_size(MPI_COMM_WORLD,&numtasks);
	MPI_Comm_rank(MPI_COMM_WORLD,&rank);
	nativeX=2;
	nativeY=4;
}

int Native(int x,int y)
{
	assert((x<nativeX) && (y<nativeY) && (x>=0) && (y>=0));//?
	return x*nativeY+y;
}

void reverseNative(int rank)
{
	X=(int)(rank/nativeY);
	Y=rank%nativeY;
}

int init(int *argc, char ***argv)
{
	assert(argc && argv);
	if(MPI_Init(argc,argv))
		return 1;
	readConfig();
	reverseNative(rank);
	return 0;
}

void testmdaA()
{
	int *test=NULL;
	MPI_Win win;
	MPI_Group group;
	MPI_Comm_group(MPI_COMM_WORLD,&group);
	int numNodes;
	MPI_Comm_size(MPI_COMM_WORLD,&numNodes);
	
	if(!rank)printf("\n\ntestmdaA\n\n");
	MPI_Barrier(MPI_COMM_WORLD);
	
	MPI_Alloc_mem( sizeof(int) *3, MPI_INFO_NULL, &(test) );
	MPI_Win_create(test,sizeof(int)*3,sizeof(int),MPI_INFO_NULL,MPI_COMM_WORLD,&win);
	test[0]=rank;
	test[1]=-1;
	test[2]=-1;
	
	MPI_Win_fence(0, win);
	MPI_Put(test, 1, MPI_INT, (rank+1)%numNodes, 1, 1, MPI_INT, win);
	MPI_Get(test+2, 1, MPI_INT, (rank+1)%numNodes, 0, 1, MPI_INT, win);
	MPI_Win_fence(0, win);
	
	printf("In MDA (%d) snd=%d rcv=%d backrecv=%d\n",rank,test[0],test[1],test[2]);
	MPI_Win_free(&win);
	MPI_Free_mem(test);
	
	MPI_Barrier(MPI_COMM_WORLD);
}

void testfgrid2A(fgrid2 * fg2, fgrid2 * slfg2)
{
	assert(fg2);
	assert(slfg2);
	
	if(fg2->id==0)printf("\n\ntestfgrid2A\n\n");
	MPI_Barrier(fg2->comm);
	
        printf("rank=%d | (%d,%d)[%d,%d] | (%d,%d)[%d,%d]\n",rank, fg2->x, fg2->y, fg2->height, fg2->width    , slfg2->x, slfg2->y, slfg2->height, slfg2->width);
	
	MPI_Barrier(fg2->comm);
}

void testarray1A(array1 *a)
{
	assert(a);
	int i,magic=1000+a->id;
	int *recv=NULL;
	pointer p;
	
	((int*)(a->data))[0]=a->id;
	((int*)(a->data))[1]=-1;
	
	if(a->id==0)printf("\n\ntestarray1A\n\n");
	MPI_Barrier(a->comm);
	
	
	if(rank==0)
		for(i=0;i<a->size;i++)
	{
		p=a->map(i,a);
		printf("pointer   i=%d  id=%d  index=%d\n",i,p.id,p.index);
	}
	
	MPI_Barrier(a->comm);
	
	array1Fence(a);
	array1Put(a,((a->id+1)%a->nodes) * a->thisSize + 1 ,&magic);
	array1Fence(a);
	array1Get(a,((a->id+1)%a->nodes) * a->thisSize  ,(void**) &recv);
	array1Fence(a);
	
	printf("array1 (%d)  %d,%d,%d\n",a->id,((int*)(a->data))[0],((int*)(a->data))[1],*recv);
	
	MPI_Barrier(a->comm);
	
}

void testarray2A(array2 *a)
{
	assert(a);
	
	//int nodes=4;
	//int w=4;
	//int h=4;
	//int x;
	//int y;
	
	//factorizeWithRatio(nodes, w, h, &x, &y);
	
	//printf("array2 nodes=%d wh=(%d,%d) nodeXY=[%d,%d]\n",nodes,w,h,x,y);
	
	
	if(a->id==0)printf("\n\ntestarray2A\n\n");
	MPI_Barrier(a->comm);
	
	printf("array2 (%d) size[%d,%d] thisSize[%d,%d]\n",a->id,a->sizeX,a->sizeY,a->thisSizeX,a->thisSizeY);
	
	MPI_Barrier(a->comm);
	
	if(a->id==0)
	{
		pointer p;
		int x=0;
		int y=0;
		for(x=0;x<a->sizeX;x++)
			for(y=0;y<a->sizeY;y++)
			{
				p=a->map(x,y,a);
				printf("array2-map (%d,%d) = (%d,%d)\n",x,y,p.id,p.index);
			}
	}
	
	MPI_Barrier(a->comm);
	
	int magic=a->id;
	int* recv=NULL;
	
	int xdif=1;
	int ydif=1;
	
	pointer p=a->map((a->thisStartX+xdif)%a->sizeX,        (a->thisStartY+ydif)%a->sizeY,  a);
	
	array2Fence(a);
	array2Put(a,(a->thisStartX+xdif)%a->sizeX,        (a->thisStartY+ydif)%a->sizeY,&magic);
	array2Fence(a);
	array2Get(a,(a->thisStartX+xdif)%a->sizeX,        (a->thisStartY+ydif)%a->sizeY,(void**) &recv);
	array2Fence(a);
	
	printf("array2   put=%d local=%d get=%d\n",a->id,((int*)(a->data))[p.index],*recv);
	
	free(recv);
	
	MPI_Barrier(a->comm);
}

void testarray21A(array21 *a)
{
	if(a->arr1->id==0)printf("\n\ntestarray21A\n\n");
	MPI_Barrier(a->arr1->comm);
	
	int magic=a->arr1->id;
	int* recv=NULL;
	
	int x=(a->arr1->id)/a->sizeY;
	int y=(a->arr1->id)%a->sizeY;
	
	array21Fence(a);
	array21Put(a, x, y, &magic);
	array21Fence(a);
	array21Get(a, x, y, (void**) &recv);
	array21Fence(a);
	
	printf("array21   put=%d get=%d\n",a->arr1->id,*recv);
	
	free(recv);
	
	MPI_Barrier(a->arr1->comm);
}

void testntreeA(ntree *t)
{
	assert(t);
	int treegatherchar;
	int treescatterchar;
	int treegathersize=0;
	int *treegather=NULL;
	int i;
	
	if(ntreeIsMegaRoot(t))printf("\n\ntestntreeA\n\n");
	MPI_Barrier(t->comm);
	
	treegatherchar=(char)('A'+(rank)%(nativeX*nativeY));
	
	treegathersize=ntreeGather(t,&treegatherchar,1,MPI_INT,(void**)(&treegather));
	ntreeScatter(t,treegather,1,MPI_INT,&treescatterchar);
	
	if(treescatterchar!=treegatherchar)
		printf("%d / %d\n",treegatherchar,treescatterchar);
	
	if(ntreeIsMegaRoot(t))
	{
		/*for(i=0;i<nativeX;i++)
		for(j=0;j<nativeY;j++)
		printf("native[%d,%d]=%d\n",i,j,fg.map(i,j));*/
		//printf("%d %d %d %d\n",g.up,g.down,g.right,g.left);
		for(i=0;i<treegathersize;i++)
			printf("%d:%c ",i,(char)treegather[i]);
		printf("\nNumTasks=%d\n\n",numtasks);
	}
	
	ntreeBarrier(t);
}

void testfgrid3A()
{
	int i;
	fgrid3 grid;
	grid.height = 3;
	grid.width = 4;
	grid.length = 2;
	
	if(!rank)printf("\n\ntestfgrid3A\n\n");
	MPI_Barrier(MPI_COMM_WORLD);
	
	if(!rank)
		for(i=0;i<grid.width*grid.height*grid.length;i++)
	{
		grid.id = i;
		fgrid3_reverse(&grid);
		printf("fg3test %d?%d %d %d %d\n", i, fgrid3_native(grid.x, grid.y, grid.z, &grid), grid.x, grid.y, grid.z);
	}
}

void testfgrid3B(fgrid3 * fg3)
{
	assert(fg3);
	int xt=-1,yt=-1,zt=-1,xyt=-1,yzt=-1,xzt=-1;
	
	if(fg3->id==0)printf("\n\ntestfgrid3B\n\n");
	MPI_Barrier(fg3->comm);
	
	MPI_Comm_rank(fg3->xSide,&(xt));
	MPI_Comm_rank(fg3->ySide,&(yt));
	MPI_Comm_rank(fg3->zSide,&(zt));
	printf("hello (%d)[%d,%d,%d] xSideID=%d ySideID=%d zSideID=%d\n",fg3->id,fg3->x,fg3->y,fg3->z,xt,yt,zt);
	fgrid3Barrier(fg3);
	
	MPI_Comm_rank(fg3->xyLine ,&(xyt));
	MPI_Comm_rank(fg3->yzLine ,&(yzt));
	MPI_Comm_rank(fg3->xzLine ,&(xzt));
	printf("HELLO (%d)[%d,%d,%d] xySideID=%d yzSideID=%d xzSideID=%d\n",fg3->id,fg3->x,fg3->y,fg3->z,xyt,yzt,xzt);
	fgrid3Barrier(fg3);
	
}

void testfgrid3C(fgrid3 * fg3, fgrid3 * slfg3)
{
	assert(fg3);
	assert(slfg3);
	
	if(fg3->id==0)printf("\n\ntestfgrid3C\n\n");
	MPI_Barrier(fg3->comm);
	
	printf("rank=%d  |  (%d,%d,%d)[%d,%d,%d]  |  (%d,%d,%d)[%d,%d,%d]\n",rank, fg3->x, fg3->y, fg3->z, fg3->height, fg3->width, fg3->length    , slfg3->x, slfg3->y, slfg3->z, slfg3->height, slfg3->width, slfg3->length);
	
	MPI_Barrier(fg3->comm);
}

void testfgridnA()
{
	fgridn* g = malloc(sizeof(fgridn));
	fgridn* sg = malloc(sizeof(fgridn));
	
	int sizes[] = {2, 4};
	int dimsCount = 2;
	int i;
	
	int index1[] = {0,0};
	int index2[] = {1,0};
	int index3[] = {0,1};
	int index4[] = {1,3};
	int steps[] = {1, 2};
	
	int steps1[] = {1,1};
	int steps2[] = {1,2,1};
	int* stepsA[] = {steps1, steps2};
	int stepSizes[] = {2,3};
	
	fgridnFromRange(g, MPI_COMM_WORLD, dimsCount, sizes, (int(*)(int*, fgridn *)) (fgridn_native), (void(*)(fgridn *)) (fgridn_reverse));
	//fgridnSlice(g, sg, 0, 1);
	//fgridnSliceLinear(g, sg, steps);
	fgridnSliceParts(g, sg, stepsA, stepSizes);
	
	if(g->id == 0)
	{
		printf("rank=%d  |  index(%d,%d) = %d\n", rank, index1[0], index1[1], g->map(index1, g));
		printf("rank=%d  |  index(%d,%d) = %d\n", rank, index2[0], index2[1], g->map(index2, g));
		printf("rank=%d  |  index(%d,%d) = %d\n", rank, index3[0], index3[1], g->map(index3, g));
		printf("rank=%d  |  index(%d,%d) = %d\n", rank, index4[0], index4[1], g->map(index4, g));
	}
	
	fgridnBarrierSubMask(g, 0);
	
	int mask = 2;
	printf("rank=%2d  |  self(full-zero) = %2d   |  self(%d) = %2d\n", rank, g->subDimsSelf[0], mask, g->subDimsSelf[mask]);
	fgridnCreateSubDimMask(g, mask);
	printf(" rank=%2d  -> self(%d) = %2d\n", rank, mask, g->subDimsSelf[mask]);
	
	fgridnBarrierSubMask(g, 0);
	
	printf("  rank=%2d  |  self(full-zero) = %2d   |  sliced = %2d\n", rank, g->subDimsSelf[0], sg->subDimsSelf[0]);
	
	fgridnBarrierSubMask(g, 0);
	
	printf("   rank=%2d  |  coords = (%2d,%2d)   |  sliced = (%2d,%2d)\n", rank, g->index[0], g->index[1]
		, sg->index[0], sg->index[1]);
	
	fgridnFree(sg);
	free(sg);
	fgridnFree(g);
	free(g);
}

void testgrouping()
{
	MPI_Group ogroup;
	MPI_Group newgroup;
	MPI_Comm comm,comm2,comm3;
	MPI_Comm_group(MPI_COMM_WORLD,&ogroup);
	int size,t=-1;
	MPI_Comm_size(MPI_COMM_WORLD,&size);
	if(size<3)
		return;
	int *ranks;
	ranks=malloc(sizeof(int)*size);
	
	if(ranks==NULL)
		printf("ERROR1\n");
	
	printf("size(%d)=%d\n",rank,size);
	int i;
	
	for(i=0;i<size;i++)
		ranks[i]=i;
	
	MPI_Group_incl(ogroup,size-1,ranks+1,&newgroup);
	MPI_Comm_create(MPI_COMM_WORLD,newgroup,&comm);
	
	if(rank)//try to comment this and you will see error. comm was not assigned for 0
		MPI_Comm_size(comm,&t);
	printf("size_2(%d)=%d\n",rank,t);
	
	if(rank==1)
		MPI_Send( &comm, 1, MPI_INT,0,777, MPI_COMM_WORLD );
	
	if(rank==2)
		MPI_Send( &comm, 1, MPI_INT,0,777, MPI_COMM_WORLD );
	
	if(rank==0)
	{
		MPI_Recv( &comm2, 1, MPI_INT,1, 777, MPI_COMM_WORLD ,NULL);
		MPI_Recv( &comm3, 1, MPI_INT,2, 777, MPI_COMM_WORLD ,NULL);
		printf("(%d,%d,%d)\n",comm,comm2,comm3);
		//as you see actualy comm was assigned for 0 but with trash abd every task has unique comm
		//MPI_Comm_size(comm2,&t);//won't do any good
	}
	MPI_Barrier(MPI_COMM_WORLD);
	printf("comm(%d)=%d\n",rank,comm);
	
	//printf("after(%d)\n",rank);
	free(ranks);
}

void testboxA()
{
	
	
	BoxHeader b;
	BoxHeader b2;
	
	if(!rank)printf("\n\ntestboxA\n\n");
	MPI_Barrier(MPI_COMM_WORLD);
	
	
	boxFromNative(&b,sizeof(int));
	boxFromNative(&b2,sizeof(int) * 400);
	
	int test;
	int test2[400];
	int *res=NULL;
	
	test=777;
	boxPut(&b,256 + 25,&test);
	test=999;
	boxPut(&b,25,&test);
	
	test=100500;
	boxPut(&b,256 + 256 + 25,&test);
	test=100600;
	boxPut(&b,256 + 256 + 256 + 25,&test);
	
	int i;
	for(i=1;i<80000 ;i++)
	{
		test2[0]=i;
		boxPut(&b2,i,&test2);
	}
	
	for(i=1;i<80000 ;i++)
	{
		res=boxGet(&b2,i);
		if((res==NULL) || (*res!=i))
		{
			printf("box next error %d\n",i);
			break;
		}
	}
	
	res=boxGet(&b,25);
	if(res==NULL)
		printf("box error\n");
	else
		printf("box res=%d\n",*res);
	
	boxFree(&b);
	boxFree(&b2);
	n:MPI_Barrier(MPI_COMM_WORLD);
}

void mapFunc_my(void* obj, long long index, BoxHeader *bh, void* ext)
{
	printf("## [%d] == %d  \t|  ext == %d\n", index, *((int*)obj), *((int*)ext));
}

int ifFunc_my(long long index, void* ext)
{
	return (index % 10)!=5; 
}

void testboxB()
{
	BoxHeader b;
	
	if(!rank)printf("\n\ntestboxB\n\nSize of BoxNode = %d\n",sizeof(BoxNode));
	MPI_Barrier(MPI_COMM_WORLD);
	
	int test;
	int ext=235813;
	
	boxFromNative(&b,sizeof(int));
	
	test=778;
	boxPut(&b,256 + 25,&test);
	test=777;
	boxPut(&b,256 + 25,&test);
	test=999;
	boxPut(&b,25,&test);
	
	test=100600;
	boxPut(&b,256 + 256 + 25,&test);
	test=100500;
	boxPut(&b,256 + 256 + 256 + 25,&test);
	
	boxDel(&b, 256 + 256 + 25);
	boxMapSome(&b, &mapFunc_my, &ifFunc_my, &ext);
	
	boxFree(&b);
	MPI_Barrier(MPI_COMM_WORLD);
}

void testcacheA(array1 *a)
{
	assert(a);
	int i,magic=1000+a->id;
	int *recv=NULL;
	Cache *c = malloc(sizeof(Cache));
	
	((int*)(a->data))[0]=a->id;
	((int*)(a->data))[1]=-1;
	
	cacheBoxFromArray1(c,a);
	
	if(a->id==0)printf("\n\ntestcacheA\n\n");
	MPI_Barrier(a->comm);
	
	//void (*put)(struct cachetemp *c, long long i, void* send, unsigned flags);
	//void (*get)(struct cachetemp *c, long long i, void** recv, unsigned flags);
	
	cacheFence(c);
	c->put(c, newP1D(((a->id+1)%a->nodes) * a->thisSize + 1) ,&magic, F_INCACHE | F_COPYCACHE);
	cacheFence(c);
	magic+=2000;
	c->put(c, newP1D(((a->id+1)%a->nodes) * a->thisSize + 1) ,&magic,  F_INCACHE | F_COPYCACHE);
	c->flush(c);
	
	cacheFence(c);
	
	//array1Get(a,((a->id+1)%a->nodes) * a->thisSize  ,(void**) &recv);
	c->get(c, newP1D(((a->id+1)%a->nodes) * a->thisSize), (void**) &recv, F_THROUGH | F_INCACHE | F_COPYCACHE);
	cacheFence(c);
	
	printf("array1 (%d)  %d,%d,%d\n",a->id,((int*)(a->data))[0],((int*)(a->data))[1],*recv);
	
	cacheFree(c);
	free(c);
	
	MPI_Barrier(a->comm);
	
}

void testcacheB(array2 *a)
{
	assert(a);
	
	Cache *c = malloc(sizeof(Cache));
	
	((int*)(a->data))[0]=a->id;
	((int*)(a->data))[1]=-1;
	
	cacheBoxFromArray2(c,a);
	
	if(a->id==0)printf("\n\ntestcacheB\n\n");
	MPI_Barrier(a->comm);
	
	//void (*put)(struct cachetemp *c, long long i, void* send, unsigned flags);
	//void (*get)(struct cachetemp *c, long long i, void** recv, unsigned flags);
	
	int magic=a->id;
	int* recv=NULL;
	
	int xdif=1;
	int ydif=1;
	
	//pointer p=a->map((a->thisStartX+xdif)%a->sizeX,        (a->thisStartY+ydif)%a->sizeY,  a);
	
	cacheFence(c);
	c->put(c, newP2D((a->thisStartX+xdif)%a->sizeX,        (a->thisStartY+ydif)%a->sizeY), &magic, F_INCACHE | F_COPYCACHE);
	//c->flush(c);
	cacheFence(c);
	c->get(c, newP2D((a->thisStartX+xdif)%a->sizeX,        (a->thisStartY+ydif)%a->sizeY), (void**)&recv, F_COPYCACHE);
	cacheFence(c);
	
	printf("array2   put=%d get=%d\n",a->id,*recv);
	
	free(recv);
	
	
	cacheFree(c);
	free(c);
	
	MPI_Barrier(a->comm);
	
}

void taskFunc_my(int taskIndex, void *data, void **result, int *resultSize)
{
	(*result)=malloc(sizeof(int));
	(*resultSize)=sizeof(int);
	int temp = *((int*)data);
	printf("taskFunc_my  :  taskIndex=%d data=%d\n", taskIndex, temp);
	(**((int**)result))=taskIndex + temp;
	
}

void taskFunc_my2(int taskIndex, void *data, void **result, int *resultSize)
{
	(*result)=malloc(sizeof(int));
	(*resultSize)=sizeof(int);
	int temp = *((int*)data);
	printf("taskFunc_my2  :  taskIndex=%d data=%d\n", taskIndex, temp);
	(**((int**)result))=taskIndex - 2*temp;
	
}

void testtasksA(Tasks *t)
{
	assert(t);
	
	//2-groups 5-tasks
	if(rank==0)printf("\n\ntesttasksA\n\n");
	MPI_Barrier(MPI_COMM_WORLD);
	
	int gr1 = tasksAddGroup(t, &taskFunc_my);
	int gr2 = tasksAddGroup(t, &taskFunc_my2);
	
	int *(datas[5]);
	int i;
	for(i=0; i<5;i++)
	{
		datas[i]=malloc(sizeof(int));
		*(datas[i])=i;
	}
	
	int taskIndexes[5];
	
	
	for(i=0;i<5;i++)
		taskIndexes[i]=tasksAddTask(t,datas[i], (i<2)?gr1:gr2, 1, 1);
	
	tasksAddTaskDep(t,taskIndexes[0],taskIndexes[4]);
	tasksAddTaskDep(t,taskIndexes[1],taskIndexes[4]);
	tasksAddGroupDep(t,taskIndexes[1],gr2);
	
	MPI_Barrier(MPI_COMM_WORLD);
	
	printf("Ready-Map ");
	for(i=0; i<5; i++)
		printf(" %d", tasksIsReady(t,taskIndexes[i]));
	printf("\n");
	
	for(i=0;i<2;i++)
		tasksRun(t,taskIndexes[i]);
	//tasksRunMany(t, taskIndexes, 5);
	
	int results[5];
	for(i=0;i<5;i++)
		results[i]=*((int*)(t->tasks[i].result));
	
	MPI_Barrier(MPI_COMM_WORLD);
	
	printf("tasks   gr1=%d(%d/%d) gr2=%d(%d/%d) :",gr1, t->groups[gr1].finished, t->groups[gr1].size,gr2, t->groups[gr2].finished, t->groups[gr2].size);
	
	for(i=0;i<5;i++)
		printf(" %d",results[i]);
	
	printf("\n");
	
	/*int *members, memsize;
	printf("members of %d : ", gr2);
	memsize = tasksGetGroupMembers(t, gr2, &members);
	for(i=0;i<memsize;i++)
		printf(" %d",members[i]);
	printf("\n");*/
	
	MPI_Barrier(MPI_COMM_WORLD);
	
}

int main(int argc, char **argv)
{
	int snd,rcv;
	int i,j,magic=777;
	int treecast=-1;
	int treereduce=0;
	
	grid2 g;
	
	fgrid2 fg;
	fgrid2 slfg;
	fgrid2 sllfg;
	fgrid2 slpfg;
	
	fgrid3 fg3;
	fgrid3 slfg3;
	fgrid3 sllfg3;
	fgrid3 slpfg3;
	
	ntree t;
	ntree t2;
	ntree t3;
	
	array1 ar;
	array2 ar2;
	array21 ar21;
	//Cache c;
	
	Tasks tasks;
	
	if(init(&argc,&argv))
		return 1;
	
	fgrid2FromNative(&fg);
// 	fgrid3FromNative(&fg3);
	fgrid3FromRange(&fg3,MPI_COMM_WORLD,2,2,2);
	
	int xParts[]={2};
	int yParts[]={1,2,1};
	
	int xParts3[]={2};
	int yParts3[]={1,1};
	int zParts3[]={1,1};
	
	fgrid2Slice(&fg,&slfg, 1, 1);
	fgrid3Slice(&fg3,&slfg3, 2, 1);
	fgrid2SliceLinear(&fg,&sllfg, 1, 2);
	fgrid3SliceLinear(&fg3,&sllfg3, 2, 1, 2);
	fgrid2SliceParts(&fg,&slpfg, xParts, yParts, 1, 3);
	fgrid3SliceParts(&fg3,&slpfg3, xParts3, yParts3, yParts3, 1, 2, 2);
	
	grid2Fromfgrid2(&g,&fg);
	ntreeFromNative(&t);
	//ntreeFromfgrid2(&t2,&fg);
	ntreeFromfgrid2Param(&t2,&fg,0,1,1);
	ntreeFromfgrid3(&t3,&fg3);
	array1FromRange(&ar, MPI_COMM_WORLD, 16, sizeof(int));
	array2FromRange(&ar2, MPI_COMM_WORLD, 8, 4, sizeof(int));
	array21FromArray1(&ar21, &ar, 4, 4);
	//cacheBoxFromArray1(&c, &ar);
	tasksFromNative(&tasks, 2, 5);
	
	treecast=(rank)%(nativeX*nativeY);
	
	ntreeReduce(&t3,&treecast,&treereduce,1,MPI_INT,MPI_SUM);
	ntreeBCast(&t3,&treecast,1,MPI_INT);
	
	
	snd=rank;
	MPI_Sendrecv (&snd,1,MPI_INT,g.up,2,&rcv,1,MPI_INT,g.down,2,g.comm, NULL);
	//MPI_Bcast(&magic,1,MPI_INT,t.pid,t.comm);
	
	MPI_Barrier(MPI_COMM_WORLD);
	int temp=-1;
	
		MPI_Comm_rank(fg.yLine,&temp);
		printf("hello (%d/%d)[%d,%d][%d,%d] : result=%d : treebcast=%d : reduce=%d\n",slfg.id,rank,X,Y,slfg.x,slfg.y,temp,treecast,treereduce);
	
	
	MPI_Barrier(MPI_COMM_WORLD);
	
	//testntreeA(&t3);
	//testfgrid2A(&fg, &sllfg);
	//testfgrid2A(&fg, &slpfg);
	
	//testfgrid3A();
	//testfgrid3B(&fg3);
	//testfgrid3C(&fg3, &slfg3);
	//testfgrid3C(&fg3, &sllfg3);
	//testfgrid3C(&fg3, &slpfg3);
	testfgridnA();
	MPI_Barrier(MPI_COMM_WORLD);
	
	//testmdaA();
	//testarray1A(&ar);
	//testcacheA(&ar);
	//testarray2A(&ar2);
	//testcacheB(&ar2);
	//testarray21A(&ar21);
	
	//testboxA();//be careful: this test req ~1.5 gig of ram
	//testboxB();
	
	//testgrouping();
	//testtasksA(&tasks);
	
	MPI_Barrier(MPI_COMM_WORLD);
	
	ntreeFree(&t);
	ntreeFree(&t2);
	ntreeFree(&t3);
	
	grid2Free(&g);
	
	fgrid2Free(&fg);
	fgrid2Free(&slfg);
	fgrid2Free(&sllfg);
	fgrid2Free(&slpfg);
	
	fgrid3Free(&fg3);
	fgrid3Free(&slfg3);
	fgrid3Free(&sllfg3);
	fgrid3Free(&slpfg3);
	
	array1Free(&ar);
	array2Free(&ar2);
	array21Free(&ar21);
	//cacheFree(&c);
	tasksFree(&tasks);
	
	MPI_Finalize();
	return 0;
}
