conc_scal_mem_alloc : custom_mem_alloc.o client_mem_alloc.o
	g++ -Wall -fopenmp custom_mem_alloc.o client_mem_alloc.o -o conc_scal_mem_alloc

custom_mem_alloc.o : custom_mem_alloc.cpp
	g++ -Wall -fopenmp -c custom_mem_alloc.cpp

client_mem_alloc.o : client_mem_alloc.cpp
	g++ -Wall -fopenmp -c client_mem_alloc.cpp

clean:
	rm -vf conc_scal_mem_alloc *.o