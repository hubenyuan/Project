
APPNAME   = ppp-4G
LIBNAME   = lib
Baud_rate = 115200
Parity    = n
Stopbits  = 1
Devsn     = /dev/ttyUSB3

LIBPATH1  = `pwd`/include/
LIBPATH2  = `pwd`/lib/
LIBPATH3  = comport
LIBPATH4  = atcmd

CFLAGS+=-I${LIBPATH1}
LDFLAGS+=-L${LIBPATH2}
LDFLAGS+=-l${LIBPATH3}
LDFLAGS+=-l${LIBPATH4}

CC = gcc
AR = ar

all:
	${CC} ${CFLAGS} main.c -o ${APPNAME} ${LDFLAGS}

clean:
	rm -f ${APPNAME}
	rm -f ${DATABASE}
	rm -rf ${LIBNAME}

run:
	export LD_LIBRARY_PATH=${LD_LIBRARY_PATH}:LIBPATH
	./${APPNAME} -b ${Baud_rate} -p ${Parity} -s ${Stopbits} -m ${Devsn}
