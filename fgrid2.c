#include "fgrid2.h"

/*!
    \fn int fgrid2_native(int x, int y, fgrid2 *fg)
    \brief Simple mapping function.
 
    Converts \a x and \a y coordinates to MPI comm rank. Fills lines (\a y coordinate) first. NOTE: \a x are rows and \a y are columns. If you want to use another mapping function, reimplement it and pass to field \a reverse in fgrid2 structure.
    Эта функция используется в паре с функцией fgrid2_reverse(fgrid2 *g) и вместе они задают топологию
    \sa fgrid2_reverse
 */
int fgrid2_native(int x, int y, fgrid2 *fg)
{
        assert((x<fg->height) && (y<fg->width) && (x>=0) && (y>=0));//?
	return x*fg->width+y;
}


/*!
    \fn void fgrid2_reverse(fgrid2 *g)
    \brief Reversed version of mapping function
    \param [in,out] g Структура, описывающая топологию
 
    Функция берёт MPI rank, указанный в структуре \a g, и записывает координаты сетки в эту же структуру
    \sa fgrid2_native
 */
void fgrid2_reverse(fgrid2 *g)
{
	assert(g);
	
	g->x = (int)(g->id / g->width);
	g->y = g->id % g->width;
}

/*!
 \fn void fgrid2CreateXLine(fgrid2 * g)
 \brief Creates communicator group for line
 
 Выделено в отдельную функцию для удобства
 \sa fgrid2CreateYLine
 */
void fgrid2CreateXLine(fgrid2 * g)
{
	assert(g);
	
	MPI_Comm_split(g->comm, g->x, 0, &(g->xLine));
	MPI_Comm_rank(g->xLine, &(g->xLineSelf));
}

/*!
 \fn void fgrid2CreateYLine(fgrid2 * g)
 \brief Creates communicator group for row
 
 Выделено в отдельную функцию для удобства
 \sa fgrid2CreateXLine
 */
void fgrid2CreateYLine(fgrid2 * g)
{
	assert(g);
	
	MPI_Comm_split(g->comm, g->y, 0, &(g->yLine));
	MPI_Comm_rank(g->yLine, &(g->yLineSelf));
}


/*!
 \fn void fgrid2FromRange(fgrid2 * g,MPI_Comm comm, int x, int y)
 \brief Creates two-dimensional grid with sizes \a s and \a y and with MPI communicator \a comm.
 
 Количество процессов в коммуникаторе должно совпадать с размером сетки (произведение \a x на \a y)
 Задаёт базовые параметры, как то: ширина и высота. При вызове создается копия коммуникатора \a comm (эта операция достаточно долгая).
 В простейшей реализации используются функции fgrid2_native(int x, int y, fgrid2 *fg) и fgrid2CreateYLine(fgrid2 * g)
 */
void fgrid2FromRange(fgrid2 * g, MPI_Comm comm, int x, int y)
{
	assert(g);
	assert((x>0) && (y>0));
	
	int numNodes;
	MPI_Comm_size(comm,&numNodes);
	assert(x*y==numNodes);
	
	g->width=y;
        g->height=x;
	
	MPI_Comm_dup(comm, &(g->comm));
	MPI_Comm_rank(g->comm, &(g->id));
	
	g->topo.obj=NULL;
	g->topo.type=Tnone;
	
	g->reverse=(void(*)(fgrid2 *g)) (fgrid2_reverse);
	g->reverse(g);
	g->map=(int(*)(int, int, fgrid2 *)) (fgrid2_native);
	
	fgrid2CreateXLine(g);
	fgrid2CreateYLine(g);
	
}

/*!
 \fn void fgrid2FromNative(fgrid2 * g)
 \brief Creates grid from native parameters.
 
 Является наиболее простой и быстрой функцией создания топологической сетки. Используются наперед заданные параметры, близкие к железу.
 */
void fgrid2FromNative(fgrid2 * g)
{
	assert(g);
	
	fgrid2FromRange(g, MPI_COMM_WORLD, nativeX, nativeY);
	
	g->topo.obj=NULL;
	g->topo.type=Tnone;
}

/*!
 \fn void fgrid2Free(fgrid2 * g)
 \brief Frees \a g structure
 
 При освобождении структуру все поля становятся равными -1, что позволяет отловить ошибку при повторном использовании очищенной структры.
 */
void fgrid2Free(fgrid2 * g)
{
	assert(g);
	
	MPI_Comm_free(&(g->xLine));
	MPI_Comm_free(&(g->yLine));
	MPI_Comm_free(&(g->comm));
	
	g->xLineSelf=-1;
	g->yLineSelf=-1;
	
	g->id=-1;
	g->x=g->y=-1;
        g->width=g->height=-1;
	
	g->map=NULL;
	g->reverse=NULL;
	
	g->topo.type=Tnone;
	g->topo.obj=NULL;
}

/*!
 \fn void fgrid2Slice(fgrid2 * g, fgrid2 * ng, int dim, int sliceIndex)
 \brief Creates new grid from existing one by slicing it.
 \param[in] g Existing grid
 \param[out] ng Returned value of a new grid
 \param[in] dim If \a dim == 0 sliced by X coordinate, if \a dim != 0 grid is sliced by y
 \param[in] sliceIndex Index by which grid is sliced (sliceIndex included into second topology)

 Элемент с номером sliceIndex вхожит во вторую топологию.
 Функция возвращает только одну из 2х топологий, в зависимости от того, в какой из них находится текущий процесс. В новую топологию устанавливается ссылка на старую.
 */
void fgrid2Slice(fgrid2 * g, fgrid2 * ng, int dim, int sliceIndex)
{
	assert(g);
	assert(ng);
	assert((dim>=0)&&(dim<2));
        assert((sliceIndex>=0) && ((dim==0) ? (sliceIndex < g->height) : (sliceIndex < g->width)));
	
	if(dim==0)
	{
		//slice on x
		ng->width=g->width;
		
		if(g->x<sliceIndex)
                        ng->height=sliceIndex;//in first part
		else
                        ng->height=g->height-sliceIndex;//in second part
		
		MPI_Comm_split(g->comm, g->x <sliceIndex, 0, &(ng->comm));
	}
	else
	{
		//slice on y
                ng->height=g->height;
		
		if(g->y<sliceIndex)
			ng->width=sliceIndex;//in first part
		else
			ng->width=g->width-sliceIndex;//in second part
		
		MPI_Comm_split(g->comm, g->y <sliceIndex, 0, &(ng->comm));
	}
	
	MPI_Comm_rank(ng->comm,&(ng->id));
	
	ng->topo.obj=g;
	ng->topo.type=Tfgrid2;
	
	ng->reverse=g->reverse;
	ng->reverse(ng);
	ng->map=g->map;
	
	// !!!!!!!!!!lines
	fgrid2CreateXLine(ng);
	fgrid2CreateYLine(ng);
	// !!!!!!!!!!untested
	
	
}

/*!
 \fn void fgrid2SliceLinear(fgrid2 * g, fgrid2 * ng, int xSlice, int ySlice)
 \brief Slices existing grid \a g by \a xSlice and \a ySlice lines and columns
 
 Разделяет сетку по координатам xSlice ySlice.
 Функция возвращает только одну из 4х топологий, в зависимости от того, в какой из них находится текущий процесс. В новую топологию устанавливается ссылка на старую.
 */
void fgrid2SliceLinear(fgrid2 * g, fgrid2 * ng, int xSlice, int ySlice)
{
	assert(g);
	assert(ng);
	assert(xSlice>0);
	assert(ySlice>0);
	assert((g->height % xSlice) == 0);
	assert((g->width % ySlice) == 0);
	
	ng->height=xSlice;
	ng->width=ySlice;
	int ww=g->width / ySlice;
	
	MPI_Comm_split(g->comm, (g->x / xSlice)*ww + (g->y / ySlice), 0, &(ng->comm));
	
	MPI_Comm_rank(ng->comm,&(ng->id));
	
	ng->topo.obj=g;
	ng->topo.type=Tfgrid2;
	
	ng->reverse=g->reverse;
	ng->reverse(ng);
	ng->map=g->map;
	
	// !!!!!!!!!!lines
	fgrid2CreateXLine(ng);
	fgrid2CreateYLine(ng);
	// !!!!!!!!!!untested
}

void fgrid2SliceParts(fgrid2 * g, fgrid2 * ng, int* xParts, int* yParts, int xSize, int ySize)
{
	assert(g);
	assert(ng);
	assert(xParts);
	assert(yParts);
	assert(xSize>0);
	assert(ySize>0);
	
	int i;
	int* xSum=malloc(sizeof(int) * xSize);
	int* ySum=malloc(sizeof(int) * ySize);
	
	xSum[0]=0;
	ySum[0]=0;
	
	for(i=1;i<xSize;i++)
		xSum[i]=xParts[i-1]+xSum[i-1];
	for(i=1;i<ySize;i++)
		ySum[i]=yParts[i-1]+ySum[i-1];
	
	assert( ! (g->height - xSum[xSize-1] - xParts[xSize-1]) );
	assert( ! (g->width - ySum[ySize-1] - yParts[ySize-1]) );
	
	free(xSum);
	free(ySum);
}

/*!
 \fn void fgrid2Barrier(fgrid2 * g)
 \brief Locks barrieir on the whole grid
 
 */
void fgrid2Barrier(fgrid2 * g)
{
	assert(g);
	MPI_Barrier(g->comm);
}
