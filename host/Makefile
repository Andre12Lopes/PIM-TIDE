BUILDDIR ?= ../bin

NR_TASKLETS ?= 11
BATCH_SIZE ?= 128
DIST_PAYMENT ?= 0

TARGET := ${BUILDDIR}/tpcc

SOURCES := $(wildcard *.cpp)
OBJECTS := $(SOURCES:%.cpp=${BUILDDIR}/%.o)

FLAGS := -Wall -Wextra -Werror -Wno-unused-parameter -std=c++17
DPU_FLAGS := `dpu-pkg-config --cflags --libs dpu`
LDFLAGS := -pthread

TASKLETS := -D NR_TASKLETS=${NR_TASKLETS}
BATCH := -D BATCH_SIZE=${BATCH_SIZE} -D DIST_PAYMENT=${DIST_PAYMENT}

.PHONY:	all debug clean

all: FLAGS += -O3
all: ${BUILDDIR}/${TARGET}

debug: FLAGS += -O0 -g -D DEBUG
debug: ${BUILDDIR}/${TARGET}

${BUILDDIR}/${TARGET}: ${OBJECTS}
	${CXX} ${OBJECTS} -o $@ ${DPU_FLAGS} ${LDFLAGS}

${BUILDDIR}/sub_transaction.o: sub_transaction.cpp sub_transaction.h \
 ../common/communication.h
${BUILDDIR}/system.o: system.cpp ../common/communication.h system.h barrier.h \
 buffer.h sub_transaction.h transaction.h
${BUILDDIR}/tpcc.o: tpcc.cpp system.h barrier.h buffer.h sub_transaction.h \
 ../common/communication.h transaction.h tpcc_client.h clock.h random.h
${BUILDDIR}/tpcc_client.o: tpcc_client.cpp tpcc_client.h clock.h random.h system.h \
 barrier.h buffer.h sub_transaction.h ../common/communication.h \
 transaction.h ../common/app_common.h
${BUILDDIR}/transaction.o: transaction.cpp transaction.h sub_transaction.h \
 ../common/communication.h

${BUILDDIR}/%.o: %.cpp
	${CXX} ${FLAGS} ${DPU_FLAGS} ${TASKLETS} ${BATCH} -c $< -o $@
