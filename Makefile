CC			=	/wsu/apps/gnu-4.4.7/openmpi/openmpi-1.8.8/bin/mpicc
CCLINK	=	/wsu/apps/gnu-4.4.7/openmpi/openmpi-1.8.8/bin/mpicc
SHELL		=	/bin/sh

EXEC		=	mst_Parallel

mst_Parallel:	mst_Parallel.c
	$(CC) -o $(EXEC) $(EXEC).c

clean:
	/bin/rm -f $(EXEC) $(EXEC)*.o $(EXEC)*.s

