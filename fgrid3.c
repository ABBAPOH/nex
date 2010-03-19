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
	
	MPI_Comm_split(g->comm, g->x*g->w + g->y, 0, &(g->xyLine));
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
	
	MPI_Comm_split(g->comm, g->y*g->l + g->z, 0, &(g->yzLine));
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
	
	MPI_Comm_split(g->comm, g->x*g->l + g->z, 0, &(g->xzLine));
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
	g->x = (g->id) / (g->w * g->l);
	g->y = (g->id / g->l) % g->w;
	g->z = (g->id) % (g->w * g->l) % (g->l);
}

/*!
	\fn int fgrid3_native(int x, int y, int z, fgrid3 *fg)
	\brief Simple map function. Calculates process rank from it's coordinates

	\sa fgrid3_reverse
*/
int fgrid3_native(int x, int y, int z, fgrid3 *fg)
{
	assert((x<fg->h) && (y<fg->w) && (z<fg->l) && (x>=0) && (y>=0) && (x>=0));//?
	//return z*fg->w*fg->h + x*fg->w+y;
	return x*fg->w*fg->l + y*fg->l + z;
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

	g->w = y;
	g->h = x;
	g->l = z;

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
		case 0: assert(sliceIndex < g->h); break;
		case 1: assert(sliceIndex < g->w); break;
		case 2: assert(sliceIndex < g->l); break;
		default: assert(0);
	}
	
	switch(dim)
	{
		case 0:{
			//slice on x
			ng->w = g->w;
			ng->l = g->l;
			
			if(g->x<sliceIndex)
				ng->h = sliceIndex;//in first part
			else
				ng->h = g->h-sliceIndex;//in second part
			
			MPI_Comm_split(g->comm, g->x <sliceIndex, 0, &(ng->comm));
		} break;
		
		case 1:{
			//slice on y
			ng->h = g->h;
			ng->l = g->l;
			
			if(g->y<sliceIndex)
				ng->w = sliceIndex;//in first part
			else
				ng->w = g->w-sliceIndex;//in second part
			
			MPI_Comm_split(g->comm, g->y <sliceIndex, 0, &(ng->comm));
		} break;
		
		case 2:{
			//slice on y
			ng->h = g->h;
			ng->w = g->w;
			
			if(g->z<sliceIndex)
				ng->l = sliceIndex;//in first part
			else
				ng->l = g->l-sliceIndex;//in second part
			
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
	assert((g->h % xSlice) == 0);
	assert((g->w % ySlice) == 0);
	assert((g->l % zSlice) == 0);
	
	ng->h = xSlice;
	ng->w = ySlice;
	ng->l = zSlice;
	int ww = g->w / ySlice;
	int ll = g->l / zSlice;
	
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
	g->w = g->h = g->l = -1;
	
	g->map = NULL;
	g->reverse = NULL;
	
	g->topo.type = Tnone;
	g->topo.obj = NULL;
	
	MPI_Comm_free(&(g->comm));
	
}
