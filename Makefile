#
#Author: yys
#

#path
DIR_BIN = bin/
DIR_LIB = lib/
DIR_SRC_COMPILER = src/compiler/
DIR_SRC_RUN = src/run/
DIR_SRC_TEST = src/test/
DIR_SRC_FORWARD = src/forward/
DIR_SRC_LIB = include/
#参数
CXX = g++
AR = ar
CFLAGS = -std=c++11 -O
REDIS = -lhiredis
JSON = -ljson
MYLIB = -I./include -L./lib -lutil
#target
TFORWARD = $(DIR_BIN)Forward
TCOMPILER = $(DIR_BIN)Compiler
TRUN = $(DIR_BIN)Run
TTEST = $(DIR_BIN)Test
TLIB = $(DIR_LIB)libutil.a

%.o:%.cpp
	$(CXX) $(CFLAGS) -c $< -o $@

SRC_FORWARD = $(wildcard $(DIR_SRC_FORWARD)*.cpp)
SRC_COMPILER = $(wildcard $(DIR_SRC_COMPILER)*.cpp)
SRC_RUN = $(wildcard $(DIR_SRC_RUN)*.cpp)
SRC_TEST = $(wildcard $(DIR_SRC_TEST)*.cpp)
SRC_LIB = $(wildcard $(DIR_SRC_LIB)*.cpp)

FORWARD_OBJS = $(patsubst %.cpp,%.o, $(SRC_FORWARD))
COMPILER_OBJS = $(patsubst %.cpp,%.o, $(SRC_COMPILER))
RUN_OBJS = $(patsubst %.cpp,%.o, $(SRC_RUN))
TEST_OBJS = $(patsubst %.cpp,%.o, $(SRC_TEST))
LIB_OBJS = $(patsubst %.cpp,%.o, $(SRC_LIB))

all:$(TLIB) $(TFORWARD) $(TCOMPILER) $(TRUN) $(TTEST)

$(TLIB):$(LIB_OBJS)
	$(AR) rcs $(TLIB) $(LIB_OBJS)

$(TFORWARD):$(FORWARD_OBJS)
	$(CXX) $(FORWARD_OBJS) -o $(TFORWARD) $(MYLIB) $(JSON) $(REDIS)
	chmod a+x $(TFORWARD)

$(TCOMPILER):$(COMPILER_OBJS)
	$(CXX) $(COMPILER_OBJS) -o $(TCOMPILER) $(MYLIB) $(JSON) $(REDIS)
	chmod a+x $(TCOMPILER)

$(TRUN):$(RUN_OBJS)
	$(CXX) $(RUN_OBJS) -o $(TRUN) $(MYLIB) $(JSON) $(REDIS)
	chmod a+x $(TRUN)

$(TTEST):$(TEST_OBJS)
	$(CXX) $(TEST_OBJS) -o $(TTEST) $(MYLIB) $(JSON) $(REDIS)
	chmod a+x $(TTEST)

.PHONY:clean
clean:
	-rm $(DIR_SRC_LIB)*.o
	-rm $(DIR_SRC_FORWARD)*.o
	-rm $(DIR_SRC_COMPILER)*.o
	-rm $(DIR_SRC_RUN)*.o
	-rm $(DIR_SRC_TEST)*.o
