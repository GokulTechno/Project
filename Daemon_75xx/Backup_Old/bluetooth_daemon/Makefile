OBJS = blue_daemon.o
CFLAG = -Wall -g
CC = arm-poky-linux-gnueabi-gcc
LIBS = -lm

blued:$(OBJS)
	${CC} ${CFLAGS} -o $@ ${OBJS} ${LIBS}
clean:
	rm *.o blued

