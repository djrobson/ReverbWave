CC=g++
CFLAGS=-std=c++14 -Wall -O2
LIBS=-lm

all: SimpleReverb

SimpleReverb: SimpleReverb.cpp
	$(CC) $(CFLAGS) -o SimpleReverb SimpleReverb.cpp $(LIBS)

clean:
	rm -f SimpleReverb reverb_output.wav original.wav

run: SimpleReverb
	./SimpleReverb
