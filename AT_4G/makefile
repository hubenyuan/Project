
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
LIBPATH5  = gpio_l
LIBPATH6  = network
LIBPATH7  = /home/hubenyuan/my_libgpiod/install/include
LIBPATH8  = /home/hubenyuan/my_libgpiod/install/lib

CFLAGS+=-I${LIBPATH1}
CFLAGS+=-I${LIBPATH7}
LDFLAGS+=-L${LIBPATH2}
LDFLAGS+=-L${LIBPATH8}
LDFLAGS+=-l${LIBPATH3}
LDFLAGS+=-l${LIBPATH4}
LDFLAGS+=-l${LIBPATH5}
LDFLAGS+=-l${LIBPATH6}

CC = /opt/buildroot/gcc-5.5-arm926t-2019.02/bin/arm-linux-gcc

all:
	${CC} main.c -o ${APPNAME} ${CFLAGS} ${LDFLAGS} -lgpiod -lpthread -static

clean:
	rm -f ${APPNAME}
	rm -f ${DATABASE}
	rm -rf ${LIBNAME}

run:
	export LD_LIBRARY_PATH=${LD_LIBRARY_PATH}:LIBPATH
	./${APPNAME} -b ${Baud_rate} -p ${Parity} -s ${Stopbits} -m ${Devsn}
