seq_mem_alloc : custom_mem_alloc.o client_mem_alloc.o
	g++ -g -Wall custom_mem_alloc.o client_mem_alloc.o -o seq_mem_alloc

custom_mem_alloc.o : custom_mem_alloc.cpp
	g++ -g -Wall -c custom_mem_alloc.cpp

client_mem_alloc.o : client_mem_alloc.cpp
	g++ -g -Wall -c client_mem_alloc.cpp

clean:
	rm -vf seq_mem_alloc *.o