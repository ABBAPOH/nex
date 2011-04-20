/*
 *  fgrid3.c
 *  
 *
 *  Created by arch on 15.11.09. //hehe i'll let him be proud of himself
 *  Copyright 2009 МГУ. All rights reserved.
 *
 */

#include "fgrid3.h"

/*!
	\fn void fgrid3CreateXYLine(fgrid3 * g)
	\brief Creates communicator for lines with same X and Y coordinates as the current node

	Создает коммуникатор для текущего узла и всех узлов, имеющих с ним такие же X и Y.
*/
void fgrid3CreateXYLine(fgrid3 * g)
{
	assert(g);
	
	MPI_Comm_split(g->comm, g->x*g->width + g->y, 0, &(g->xyLine));
	MPI_Comm_rank(g->xyLine, &(g->xyLineSelf));
}

/*!
	\fn void fgrid3CreateYZLine(fgrid3 * g)
	\brief Creates communicator for lines with same Y and Z coordinates as the current node

	Создает коммуникатор для текущего узла и всех узлов, имеющих с ним такие же Y и Z.
*/
void fgrid3CreateYZLine(fgrid3 * g)
{
	assert(g);
	
	MPI_Comm_split(g->comm, g->y*g->length + g->z, 0, &(g->yzLine));
	MPI_Comm_rank(g->yzLine, &(g->yzLineSelf));
}

/*!
	\fn void fgrid3CreateXZLine(fgrid3 * g)
	\brief Creates communicator for lines with same X and Z coordinates as the current node

	Создает коммуникатор для текущего узла и всех узлов, имеющих с ним такие же X и Z.
*/
void fgrid3CreateXZLine(fgrid3 * g)
{
	assert(g);
	
	MPI_Comm_split(g->comm, g->x*g->length + g->z, 0, &(g->xzLine));
	MPI_Comm_rank(g->xzLine, &(g->xzLineSelf));
}

/*!
	\fn void fgrid3CreateXSide(fgrid3 * g)
	\brief Creates communicator for side with same X as the current node

	Создает коммуникатор для текущего узла и всех узлов, имеющих с ним такой же X.
*/
void fgrid3CreateXSide(fgrid3 * g)
{
	assert(g);
	
	MPI_Comm_split(g->comm, g->x, 0, &(g->xSide));
	MPI_Comm_rank(g->xSide, &(g->xSideSelf));	
}

/*!
	\fn void fgrid3CreateYSide(fgrid3 * g)
	\brief Creates communicator for side with same Y as the current node

	Создает коммуникатор для текущего узла и всех узлов, имеющих с ним такой Y.
*/
void fgrid3CreateYSide(fgrid3 * g)
{
	assert(g);
	
	MPI_Comm_split(g->comm, g->y, 0, &(g->ySide));
	MPI_Comm_rank(g->ySide, &(g->ySideSelf));
}

/*!
	\fn void fgrid3CreateZSide(fgrid3 * g)
	\brief Creates communicator for side with same Z as the current node

	Создает коммуникатор для текущего узла и всех узлов, имеющих с ним такой Z.
*/
void fgrid3CreateZSide(fgrid3 * g)
{
	assert(g);
	
	MPI_Comm_split(g->comm, g->z, 0, &(g->zSide));
	MPI_Comm_rank(g->zSide, &(g->zSideSelf));
}

/*!
	\fn void fgrid3_reverse(fgrid3 *g)
	\brief Writes current node coordinates to structure \g

	Current coordinates is calculated from node's rank

	\sa fgrid3_native
*/
void fgrid3_reverse(fgrid3 *g)
{
	assert(g);
	g->x = (g->id) / (g->width * g->length);
	g->y = (g->id / g->length) % g->width;
	g->z = (g->id) % (g->width * g->length) % (g->length);
}

/*!
	\fn int fgrid3_native(int x, int y, int z, fgrid3 *fg)
	\brief Simple map function. Calculates process rank from it's coordinates

	\sa fgrid3_reverse
*/
int fgrid3_native(int x, int y, int z, fgrid3 *fg)
{
	assert((x<fg->height) && (y<fg->width) && (z<fg->length) && (x>=0) && (y>=0) && (x>=0));//?
	//return z*fg->w*fg->h + x*fg->w+y;
	return x*fg->width*fg->length + y*fg->length + z;
}

/*!
	\fn void fgrid3FromRange(fgrid3 * g, MPI_Comm comm, int x, int y, int z)
	\brief Creates three-dimensional grid from communicator \a comm with size x*y*z

	Количество процессов в коммуникаторе должно быь в точности равно x*y*z.
	Размеры задаются так: \a x - height, \a y - width, \a z - length
	Коммуникатор дублируется, так что \a comm можно удалить извне
	В данной версии функции переменные reverse и map (функции маппирования) жестко заданы (используются fgrid3_reverse и fgrid3_native)
	Также создаются все line и side коммуникаторы.
*/
void fgrid3FromRange(fgrid3 * g, MPI_Comm comm, int x, int y, int z)
{
	assert(g);
	assert((x>0) && (y>0) && (z>0));

	int numNodes;
	MPI_Comm_size(comm,&numNodes);
	assert(x*y*z == numNodes);

	g->width = y;
	g->height = x;
	g->length = z;

	MPI_Comm_dup(comm, &(g->comm));
	MPI_Comm_rank(g->comm, &(g->id));

	g->topo.obj=NULL;
	g->topo.type=Tnone;

	g->reverse=(void(*)(fgrid3 *g)) (fgrid3_reverse);
	g->reverse(g);
	g->map=(int(*)(int, int, int, fgrid3 *)) (fgrid3_native);

	fgrid3CreateXYLine(g);
	fgrid3CreateYZLine(g);
	fgrid3CreateXZLine(g);

	fgrid3CreateXSide(g);
	fgrid3CreateYSide(g);
	fgrid3CreateZSide(g);
}

/*!
	\fn void fgrid3FromNative(fgrid3 * g)
	\brief Creates topology using MPI_COMM_WORLD communicator, and size nativeX*nativeY*1

	\sa fgrid3FromRange
*/
void fgrid3FromNative(fgrid3 * g)
{

	fgrid3FromRange(g, MPI_COMM_WORLD, nativeX, nativeY, 1);

	g->topo.obj = NULL;
	g->topo.type = Tnone;
}

/*!
 \fn void fgrid3Slice(fgrid3 * g, fgrid3 * ng, int dim, int sliceIndex)
 \brief Creates new grid from existing one by slicing it.
 \param[in] g Existing grid
 \param[out] ng Returned value of a new grid
 \param[in] dim If \a dim == 0 sliced by X coordinate, if \a dim == 1 grid is sliced by Y and  if \a dim == 2 grid is sliced by Z
 \param[in] sliceIndex Index by which grid is sliced (sliceIndex included into second topology)

 На самом деле, создается 2 топологии, но возвращается только одна - та, в кторой находится текущий процесс
 Элемент с номером sliceIndex вхожит во вторую топологию.
 Функция возвращает только одну из 2х топологий, в зависимости от того, в какой из них находится текущий процесс. В новую топологию устанавливается ссылка на старую.
 */
void fgrid3Slice(fgrid3 * g, fgrid3 * ng, int dim, int sliceIndex)
{
	assert(g);
	assert(ng);
	assert((dim >= 0) && (dim < 3));
	assert(sliceIndex >= 0);
	switch (dim)
	{
		case 0: assert(sliceIndex < g->height); break;
		case 1: assert(sliceIndex < g->width); break;
		case 2: assert(sliceIndex < g->length); break;
		default: assert(0);
	}
	
	switch(dim)
	{
		case 0:{
			//slice on x
			ng->width = g->width;
			ng->length = g->length;
			
			if(g->x<sliceIndex)
				ng->height = sliceIndex;//in first part
			else
				ng->height = g->height-sliceIndex;//in second part
			
			MPI_Comm_split(g->comm, g->x <sliceIndex, 0, &(ng->comm));
		} break;
		
		case 1:{
			//slice on y
			ng->height = g->height;
			ng->length = g->length;
			
			if(g->y<sliceIndex)
				ng->width = sliceIndex;//in first part
			else
				ng->width = g->width-sliceIndex;//in second part
			
			MPI_Comm_split(g->comm, g->y <sliceIndex, 0, &(ng->comm));
		} break;
		
		case 2:{
			//slice on y
			ng->height = g->height;
			ng->width = g->width;
			
			if(g->z<sliceIndex)
				ng->length = sliceIndex;//in first part
			else
				ng->length = g->length-sliceIndex;//in second part
			
			MPI_Comm_split(g->comm, g->z <sliceIndex, 0, &(ng->comm));
		}break;
		
	}
	
	MPI_Comm_rank(ng->comm, &(ng->id));
	
	ng->topo.obj = g;
	ng->topo.type = Tfgrid3;
	
	ng->reverse = g->reverse;
	ng->reverse(ng);
	ng->map = g->map;
	
	//!!!!!!!!!!lines
	fgrid3CreateXYLine(ng);
	fgrid3CreateYZLine(ng);
	fgrid3CreateXZLine(ng);
	fgrid3CreateXSide(ng);
	fgrid3CreateYSide(ng);
	fgrid3CreateZSide(ng);
	//!!!!!!!!!!untested
}

/*!
 \fn void fgrid3SliceLinear(fgrid3 * g, fgrid3 * ng, int xSlice, int ySlice, int zSlice)
 \brief Slices existing grid \a g into many grids with sizes \a xSlice, \a ySlice and \a zSlice

 Разделяет сетку по размеры xSlice, ySlice и zSlice.
 Несмотря на то, что создается много новых сеток, возвращается только одна - та, в которой находится текущий процесс
 В новую топологию устанавливается ссылка на старую.
*/
void fgrid3SliceLinear(fgrid3 * g, fgrid3 * ng, int xSlice, int ySlice, int zSlice)
{
	assert(g);
	assert(ng);
	assert(xSlice>0);
	assert(ySlice>0);
	assert(zSlice>0);
	assert((g->height % xSlice) == 0);
	assert((g->width % ySlice) == 0);
	assert((g->length % zSlice) == 0);
	
	ng->height = xSlice;
	ng->width = ySlice;
	ng->length = zSlice;
	int ww = g->width / ySlice;
	int ll = g->length / zSlice;
	
	MPI_Comm_split(g->comm, (g->x / xSlice)*ww*ll + (g->y / ySlice)*ll + (g->z / zSlice), 0, &(ng->comm));
	
	MPI_Comm_rank(ng->comm, &(ng->id));
	
	ng->topo.obj=g;
	ng->topo.type=Tfgrid3;
	
	ng->reverse = g->reverse;
	ng->reverse(ng);
	ng->map = g->map;
	
	//!!!!!!!!!!lines
	fgrid3CreateXYLine(ng);
	fgrid3CreateYZLine(ng);
	fgrid3CreateXZLine(ng);
	fgrid3CreateXSide(ng);
	fgrid3CreateYSide(ng);
	fgrid3CreateZSide(ng);
	//!!!!!!!!!!untested
}

void fgrid3SliceParts(fgrid3 * g, fgrid3 * ng, int* xParts, int* yParts, int* zParts, int xSize, int ySize, int zSize)
{
	assert(g);
	assert(ng);
	assert(xParts);
	assert(yParts);
	assert(zParts);
	assert(xSize>0);
	assert(ySize>0);
	assert(zSize>0);
	
	int i;
	int* xSum=malloc(sizeof(int) * xSize);
	int* ySum=malloc(sizeof(int) * ySize);
	int* zSum=malloc(sizeof(int) * zSize);
	int xColor=xSize-1;
	int yColor=ySize-1;
	int zColor=zSize-1;
	int resultColor=0;
	
	xSum[0]=0;
	ySum[0]=0;
	zSum[0]=0;
	
	for(i=1;i<xSize;i++)
		xSum[i]=xParts[i-1]+xSum[i-1];
	for(i=1;i<ySize;i++)
		ySum[i]=yParts[i-1]+ySum[i-1];
	for(i=1;i<zSize;i++)
		zSum[i]=zParts[i-1]+zSum[i-1];
	
	for(i=1;i<xSize;i++)
		if(xSum[i]>g->x)
		{
			xColor=i-1;
			break;
		}
	for(i=1;i<ySize;i++)
		if(ySum[i]>g->y)
		{
			yColor=i-1;
			break;
		}
	for(i=1;i<zSize;i++)
		if(zSum[i]>g->z)
		{
			zColor=i-1;
			break;
		}
	
	resultColor=xColor*ySize*zSize + yColor*zSize + zColor;
	//printf("Parts (%d,%d,%d) color=(%d,%d,%d) size=(%d,%d,%d) resultColor=%d\n", g->x, g->y, g->z, xColor, yColor, zColor, xParts[xColor], yParts[yColor], zParts[zColor], resultColor);
	
	//assert( ! (g->height - xSum[xSize-1] - xParts[xSize-1]) );
	//assert( ! (g->width - ySum[ySize-1] - yParts[ySize-1]) );
	//assert( ! (g->length - zSum[zSize-1] - zParts[zSize-1]) );
	
	ng->height = xParts[xColor];
	ng->width = yParts[yColor];
	ng->length = zParts[zColor];
	
	MPI_Comm_split(g->comm, resultColor, 0, &(ng->comm));
	
	MPI_Comm_rank(ng->comm,&(ng->id));
	
	ng->topo.obj=g;
	ng->topo.type=Tfgrid3;
	
	ng->reverse=g->reverse;
	ng->reverse(ng);
	ng->map=g->map;
	
	fgrid3CreateXYLine(ng);
	fgrid3CreateYZLine(ng);
	fgrid3CreateXZLine(ng);
	fgrid3CreateXSide(ng);
	fgrid3CreateYSide(ng);
	fgrid3CreateZSide(ng);
	
	free(xSum);
	free(ySum);
	free(zSum);
}

/*!
	\fn void fgrid3Free(fgrid3 * g)
	\brief Frees \a g structure

	Очищает поля структуры \a g, заполняя поля значением -1,
	так чтобы можно было легко отследить использование очищенной структуры
*/
void fgrid3Free(fgrid3 * g)
{
	assert(g);
	
	MPI_Comm_free(&(g->xSide));
	MPI_Comm_free(&(g->ySide));
	MPI_Comm_free(&(g->zSide));
	
	MPI_Comm_free(&(g->xyLine));
	MPI_Comm_free(&(g->yzLine));
	MPI_Comm_free(&(g->xzLine));
	
	g->xSideSelf = -1;
	g->ySideSelf = -1;
	g->zSideSelf = -1;
	g->xyLineSelf = -1;
	g->yzLineSelf = -1;
	g->xzLineSelf = -1;
	
	g->id = -1;
	g->x = g->y = g->z = -1;
	g->width = g->height = g->length = -1;
	
	g->map = NULL;
	g->reverse = NULL;
	
	g->topo.type = Tnone;
	g->topo.obj = NULL;
	
	MPI_Comm_free(&(g->comm));
	
}

/*!
 \fn void fgrid3Barrier(fgrid3 * g)
 \brief Locks barrier on the whole grid \a g

 */
void fgrid3Barrier(fgrid3 * g)
{
	assert(g);
	MPI_Barrier(g->comm);
}

/*!
 \fn void fgrid3BarrierX(fgrid3 * g)
 \brief Locks barrier on the nodes with the same X \a g

 */
void fgrid3BarrierX(fgrid3 * g)
{
	assert(g);
	MPI_Barrier(g->xSide);
}

/*!
 \fn void fgrid3BarrierY(fgrid3 * g)
 \brief Locks barrier on the nodes with the same Y \a g

 */
void fgrid3BarrierY(fgrid3 * g)
{
	assert(g);
	MPI_Barrier(g->ySide);
}

/*!
 \fn void fgrid3BarrierZ(fgrid3 * g)
 \brief Locks barrier on the nodes with the same Z \a g

 */
void fgrid3BarrierZ(fgrid3 * g)
{
	assert(g);
	MPI_Barrier(g->zSide);
}

/*!
 \fn void fgrid3BarrierXY(fgrid3 * g)
 \brief Locks barrier on the nodes with the same X and Y \a g

 */
void fgrid3BarrierXY(fgrid3 * g)
{
	assert(g);
	MPI_Barrier(g->xyLine);
}

/*!
 \fn void fgrid3BarrierYZ(fgrid3 * g)
 \brief Locks barrier on the nodes with the same Y and Z \a g

 */
void fgrid3BarrierYZ(fgrid3 * g)
{
	assert(g);
	MPI_Barrier(g->yzLine);
}

/*!
 \fn void fgrid3BarrierXZ(fgrid3 * g)
 \brief Locks barrier on the nodes with the same X and Z \a g

 */
void fgrid3BarrierXZ(fgrid3 * g)
{
	assert(g);
	MPI_Barrier(g->xzLine);
}
