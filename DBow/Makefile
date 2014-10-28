CC=gcc
CFLAGS=-I../DUtils
LFLAGS=-L../DUtils
LIBS=-lstdc++ -lDUtils

DEPS=BowVector.h DbInfo.h HVocParams.h Vocabulary.h Database.h DBow.h QueryResults.h VocInfo.h DatabaseTypes.h HVocabulary.h VocParams.h
OBJS=BowVector.o DbInfo.o HVocParams.o Vocabulary.o VocParams.o Database.o HVocabulary.o QueryResults.o VocInfo.o

%.o: %.cpp $(DEPS)
	$(CC) $(CFLAGS) -fPIC -O3 -Wall -c $< -o $@ 

libDBow.so: $(OBJS)
	$(CC) $^ $(LFLAGS) $(LIBS) -shared -o $@

clean:
	rm -f *.o *.so


