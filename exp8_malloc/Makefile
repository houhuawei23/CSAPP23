# Copyright 2022 by mars

# Description: Makefile for building a malloc/free Simulator.
#


LDFLAGS += 

LDLIBS   += 

CPPFLAGS := -O3 -Wall -Wextra -Winline -Winit-self -Wno-sequence-point\
           -Wno-unused-function -Wno-inline -fPIC -W -Wcast-qual -Wpointer-arith -Wno-unused-parameter

#CPPFLAGS := -g
PROGRAMS := malloc

objects = clock.o fcyc.o fsecs.o mdriver.o memlib.o

all: $(PROGRAMS)

malloc : $(objects)
	gcc $(CPPFLAGS) $(LDFLAGS) -o $@ $^ $(LDLIBS)
	rm -f $(objects)

malloc_%: mm_%.o $(objects)
	gcc $(CPPFLAGS) $(LDFLAGS) -o $@ $^ $(LDLIBS)
	rm -f $(objects)

clean:
	rm -f $(PROGRAMS) $(objects)
