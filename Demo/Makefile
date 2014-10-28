CC=g++
CFLAGS=-I../DUtils -I../DBow $(OPENCV_CFLAGS)
LFLAGS=-L../DUtils -L../DBow $(OPENCV_LFLAGS)
LIBS=-lDUtils -lDBow $(OPENCV_LIBS)
DEPS=
OBJS=Demo.o

%.o: %.cpp $(DEPS)
	$(CC) $(CFLAGS) -O3 -Wall -c $< -o $@ 

Demo: $(OBJS)
	$(CC) $^ $(LFLAGS) $(LIBS) -o $@

clean:
	rm -f *.o Demo

