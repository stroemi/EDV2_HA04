all: parpi

parpi: parpi.c
	mpicc -o parpi parpi.c 
