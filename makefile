.PHONY : clean
.PHONY : run
PROC_NUM = 8
LIB_PATH = -L/usr/lib64/mpi/gcc/openmpi/lib64
INCLUDE_PATH = -I/usr/lib64/mpi/gcc/openmpi/include -Ilib/
LIBS = -lmpi -lm
BINARY_NAME = nex
SOURCES = examples/main.c lib/fgrid2.c lib/fgrid3.c lib/fgridn.c lib/grid2.c lib/ntree.c lib/array.c lib/narray.c lib/range.c lib/box.c lib/cache.c lib/tasks.c
COMMON_HEADERS = lib/defs.h
HEADERS = $(SOURCES:.c=.h)
OBJECTS = $(SOURCES:.c=.o)

#$@ - target name
#$? - updated files

#$< - updated file
#$* - updated prefix
#.c.o - common rule for *.c -> *.o

$(BINARY_NAME): $(OBJECTS)
	gcc $? -o $@ $(LIB_PATH) $(LIBS);

$(OBJECTS):: $(COMMON_HEADERS)
	gcc $(@:.o=.c) -c $(INCLUDE_PATH)

$(OBJECTS):: $(HEADERS) $(SOURCES)
	gcc $*.c -c $(INCLUDE_PATH) -o $*.o

#.c.o:: $*.c $*.h 
#	gcc $*.c -c $(INCLUDE_PATH)
	
run: $(BINARY_NAME)
	mpirun -n $(PROC_NUM) ./$(BINARY_NAME)

clean:
	rm -f ./*.o
