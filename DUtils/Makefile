CC=gcc
DEPS=BinaryFile.h DUtils.h LineFile.h Random.h Timestamp.h DException.h FileModes.h Math.hpp
OBJS=BinaryFile.o LineFile.o Random.o Timestamp.o

%.o: %.cpp $(DEPS)
	$(CC) -fPIC -O3 -Wall -c $< -o $@ 

libDUtils.so: $(OBJS)
	$(CC) $^ -shared -o $@

clean:
	rm -f *.o *.so


