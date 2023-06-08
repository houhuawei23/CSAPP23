# Copyright 2022 by mars

# Description: Makefile for building a Cache Simulator.
#


LDFLAGS += 

LDLIBS   += -lzstd

CPPFLAGS := -O3 -Wall -Wextra -Winline -Winit-self -Wno-sequence-point\
           -Wno-unused-function -Wno-inline -fPIC -W -Wcast-qual -Wpointer-arith -Icbsl/include -g

#CPPFLAGS := -g

objects = $(srcFolder)CacheHelper.o $(srcFolder)getopt.o cbsl/src/buffer.o cbsl/src/file.o cbsl/src/flush.o cbsl/src/read.o cbsl/src/record.o cbsl/src/utils.o cbsl/src/write.o

srcFolder = ./src/

Cache%: $(srcFolder)Cache%.o $(objects)
	gcc $(CPPFLAGS) $(LDFLAGS) -o $@ $^ $(LDLIBS)
	# rm -f $(objects)
clean:
	rm -f Cache*
	rm -f $(objects)
