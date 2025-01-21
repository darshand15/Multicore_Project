concurrent_mem_alloc : custom_mem_alloc.o client_mem_alloc.o
	g++ -Wall custom_mem_alloc.o client_mem_alloc.o -o concurrent_mem_alloc

custom_mem_alloc.o : custom_mem_alloc.cpp
	g++ -Wall -c custom_mem_alloc.cpp

client_mem_alloc.o : client_mem_alloc.cpp
	g++ -Wall -c client_mem_alloc.cpp

clean:
	rm -vf concurrent_mem_alloc *.o