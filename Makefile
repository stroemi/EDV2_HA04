all: pi_mpi

pi_mpi: pi_mpi.c
	mpicc -o pi_mpi pi_mpi.c 
