
.SUFFIXES: .cc $(.SUFFIXES)

ALL = libVideoEngineUtilities.a

all: $(ALL)

.cpp.o:
	$(C++) -o $@ -c $(PROF) $(CFLAGS) $*.cpp
.cc.o:
	$(C++) -o $@ -c $(PROF) $(CFLAGS) $*.cc
.c.o:
	$(CC) -o $@ -c $(PROF) $(CFLAGS) $*.c

top_srcdir = ..

CC = $(CROSS_COMPILE)gcc -std=gnu++11
C++ = $(CROSS_COMPILE)g++ -std=gnu++11
AR = $(CROSS_COMPILE)ar

CCOPT = -Os -Wall -Wno-deprecated
CCOPT_DEBUG = -Wall -g -Wno-deprecated
PROF = 

STATIC = -static

BOOST_HEADER = ./../OthersLib/boost/


INCLUDE = \
	-I./ \
	-I./../VideoEngineController/ \
	-I$(BOOST_HEADER) \

DEFINE =  -D_LINUX -DDEBUGLOG -D_INDENT_DB_PRINT -fsigned-char -fno-inline -fPIC -D_REENTRANT -D_POSIX_PTHREAD_SEMANTICS -D_POSIX_PER_PROCESS_TIMER_SOURCE -D_PTHREADS

SHLIB_SUFFIX   = .so
SHLIB_LD       = $(CC) -shared
SHLIB_LD_LIBS  = 
SHLIB_CFLAGS   = -fPIC

BFLAGS = $(DEFINE) $(INCLUDE)
CFLAGS = $(CCOPT) $(BFLAGS)

OBJ_C =

OBJ_CC = \
	./LockHandler.o \

OBJS = $(OBJ_C) $(OBJ_CC) 

SRCS = $(OBJ_C:.o=.c) $(OBJ_CC:.o=.cc)

debug: DEFINE += -DDEBUG
debug: BFLAGS = $(DEFINE) $(INCLUDE)
debug: CFLAGS = $(CCOPT_DEBUG) $(BFLAGS)
debug: $(OBJS)
	$(AR) crsu libVideoEngineUtilities_debug.a $(OBJS)

libVideoEngineUtilities.a: $(OBJS)
	$(AR) crsu libVideoEngineUtilities.a $(OBJS)

cleandeps:	
	$(RM) ./*.o .depend* core

clean: cleandeps	
	$(RM) ./libVideoEngineUtilities.a ./libVideoEngineUtilities_debug.a
	$(RM) $(ALL)
