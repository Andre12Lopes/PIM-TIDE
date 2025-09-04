BUILDDIR ?= ../bin

NR_TASKLETS ?= 11
BATCH_SIZE ?= 128
R_SET_SIZE ?= 2000
W_SET_SIZE ?= 1000

TARGET := ${BUILDDIR}/dpu_main

SOURCES := $(wildcard *.c)
OBJECTS := $(SOURCES:%.c=${BUILDDIR}/%.o)

FLAGS := -Wall -Wextra -Werror -Wno-return-type

TASKLETS := -D NR_TASKLETS=${NR_TASKLETS}

DEFINES += ${TASKLETS}
DEFINES += -D BATCH_SIZE=${BATCH_SIZE}
DEFINES += -D R_SET_SIZE=${R_SET_SIZE}
DEFINES += -D W_SET_SIZE=${W_SET_SIZE}

.PHONY:	all debug clean

all: FLAGS += -O3
all: ${BUILDDIR}/${TARGET}

debug: FLAGS += -O1 -g
debug: DEFINES += -D DEBUG
debug: ${BUILDDIR}/${TARGET}

${BUILDDIR}/${TARGET}: ${OBJECTS}
	dpu-upmem-dpurte-clang ${TASKLETS} ${OBJECTS} -o $@

${BUILDDIR}/alloc.o: alloc.c alloc.h
${BUILDDIR}/application.o: application.c ../common/app_common.h alloc.h application.h \
 util.h avl_tree.h byte_utils.h tm.h ../common/communication.h
${BUILDDIR}/avl_tree.o: avl_tree.c avl_tree.h util.h alloc.h
${BUILDDIR}/dpu_main.o: dpu_main.c scheduler.h ../common/communication.h util.h
${BUILDDIR}/scheduler.o: scheduler.c application.h ../common/app_common.h util.h \
 scheduler.h ../common/communication.h
${BUILDDIR}/tm.o: tm.c alloc.h tm.h ../common/communication.h util.h

${BUILDDIR}/%.o: %.c
	dpu-upmem-dpurte-clang ${FLAGS} ${DEFINES} -c $< -o $@
