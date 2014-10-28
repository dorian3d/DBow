
all:  libraries demo

libraries: DUtils/libDUtils.so DBow/libDBow.so
demo: Demo/Demo

DUtils/libDUtils.so:
	make -C DUtils

DBow/libDBow.so:
	make -C DBow

Demo/Demo:
	make -C Demo \
	OPENCV_CFLAGS='`pkg-config --cflags opencv`' \
	OPENCV_LFLAGS='`pkg-config --libs-only-L opencv`' \
	OPENCV_LIBS='-lcxcore -lcv -lhighgui -lcvaux -lml'

nocv: libraries

install-nocv: libraries
	mkdir -p lib && cp DUtils/*.so lib && cp DBow/*.so lib

install: all
	mkdir -p lib && cp DUtils/*.so lib && cp DBow/*.so lib && \
	mkdir -p bin && cp Demo/Demo bin && cp Demo/*.png bin

clean:
	make -C DUtils clean && make -C DBow clean && make -C Demo clean

