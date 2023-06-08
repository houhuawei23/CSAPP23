CC=gcc
 
CFLAGS = -Wall -m32 -O0 -mavx -mavx2 -mfma -msse -msse2 -msse3
 
DEFINE =
INCLUDE = -I.
 
SOURCES = $(wildcard *.c)
OBJS = $(patsubst %.c,%.o,$(SOURCES))
 
TARGET  = matrix
 
All:$(OBJS)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJS)	
 
%.o:%.c
	$(CC) -c $(CFLAGS) $(DEFINE) $(INCLUDE) $< -o $@	
clean:	
	rm -f $(OBJS)