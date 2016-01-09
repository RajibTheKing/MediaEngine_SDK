
.SUFFIXES: .cc $(.SUFFIXES)

ALL = libVideoEngineController.a

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
CCOPT_DEBUG = -g -Wall -Wno-deprecated
PROF = 

BOOST_HEADER = ./../OthersLib/boost/

STATIC = -static

INCLUDE = \
	-I./ \
	-I$(top_srcdir)/VideoEngineUtilities \
	-I$(BOOST_HEADER) \

LIBDIRS = \
	-L$(top_srcdir)/VideoEngineUtilities \

DEFINE =  -D_LINUX -D_INDENT_DB_PRINT -fsigned-char -fno-inline -fPIC -D_REENTRANT -D_POSIX_PTHREAD_SEMANTICS -D_POSIX_PER_PROCESS_TIMER_SOURCE -D_PTHREADS -DUNICODE -DUSE_JNI

SHLIB_SUFFIX   = .so
SHLIB_LD       = $(CC) -shared
SHLIB_LD_LIBS  = 
SHLIB_CFLAGS   = -fPIC

BFLAGS = $(DEFINE) $(INCLUDE)
CFLAGS = $(CCOPT) $(BFLAGS)

OBJ_C =

OBJ_CC = \
	./CommonElementsBucket.o \
	./Controller.o \
	./Decoder.o \
	./Encoder.o \
	./EncoderListHandler.o \
	./EventNotifier.o \
	./LogPrinter.o \
	./Tools.o \
	./VideoSession.o \
	./VideoSessionListHandler.o \

OBJS = $(OBJ_C) $(OBJ_CC) 

SRCS = $(OBJ_C:.o=.c) $(OBJ_CC:.o=.cc)

debug: DEFINE += -DDEBUG
debug: BFLAGS = $(DEFINE) $(INCLUDE)
debug: CFLAGS = $(CCOPT_DEBUG) $(BFLAGS)
debug: $(OBJS)
	$(AR) crsu libVideoEngineController_debug.a $(OBJS)
		-o libVideoEngineController_debug.so.1.0.1 $(OBJS)

debug_eval: DEFINE += -DDEBUG -DAFW_TIMEBOMB_ENABLED -DBOOST_ALL_NO_LIB
debug_eval: BFLAGS = $(DEFINE) $(INCLUDE)
debug_eval: CFLAGS = $(CCOPT_DEBUG) $(BFLAGS)
debug_eval: $(OBJS)
	$(AR) crsu libVideoEngineController_debug_eval.a $(OBJS)

eval: DEFINE += -DAFW_TIMEBOMB_ENABLED -DBOOST_ALL_NO_LIB
eval: $(OBJS)
	$(AR) crsu libVideoEngineController_eval.a $(OBJS)

libVideoEngineController.a: $(OBJS)
	$(AR) crsu $@ $(OBJS)

cleandeps:	
	$(RM) ./*.o .depend* core

clean: cleandeps
	$(RM) libVideoEngineController_eval.a libVideoEngineController_debug_eval.a libVideoEngineController_debug.a
	$(RM) $(ALL)

