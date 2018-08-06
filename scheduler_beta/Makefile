# see ../../deploy.make.common for most of the instructions.

OBJS_DIR = .objs

# define simulator executables
EXE_MAIN=main
EXE_GTEST=gtest
EXE_TEST=scheduler_test

# list object file dependencies for each
OBJS_TEST=$(EXE_TEST).o
OBJS_GTEST=$(EXE_GTEST).o
OBJS_MAIN=$(EXE_MAIN).o
OBJS_SCHEDULER=libscheduler.o
OBJS_GTHREAD=gthread.o gtswtch.o
OBJS_TEST_DEP=$(OBJS_SCHEDULER) $(OBJS_TEST)
OBJS_GTEST_DEP=$(OBJS_GTEST) $(OBJS_SCHEDULER) $(OBJS_GTHREAD)
OBJS_ALL=$(OBJS_MAIN) $(OBJS_SCHEDULER) $(OBJS_GTHREAD)

# set up compiler
CC = clang
INCLUDES=-I./libs/
WARNINGS = -Wall -Wextra -Wno-unused -Wno-unused-parameter
CFLAGS_DEBUG = -O0 $(INCLUDES) $(WARNINGS) -g -std=c99 -c  -D_GNU_SOURCE -DDEBUG
CFLAGS_RELEASE = -O0 $(INCLUDES) $(WARNINGS) -std=c99 -c  -D_GNU_SOURCE

# set up linker
LD = clang
LDFLAGS = -Llibs/ -lprovided

# The string in grep must appear in the hostname, otherwise the Makefile will
# not allow the assignment to compile
IS_VM=$(shell hostname | grep "cs241")

ifeq ($(IS_VM),)
$(error This assignment must be compiled on the CS241 VMs)
endif

.PHONY: all
all: release

.PHONY: debug
.PHONY: release

release: $(EXE_MAIN) $(EXE_GTEST) $(EXE_TEST)
debug:   $(EXE_MAIN:%=%-debug) $(EXE_GTEST:%=%-debug) $(EXE_TEST:%=%-debug)

#include dependencies
-include $(OBJS_DIR)/*.d
$(OBJS_DIR):
	@mkdir -p $(OBJS_DIR)

# patterns to create objects
# keep the debug and release postfix for object files so that we can always
# separate them correctly

$(OBJS_DIR)/gtswtch-debug.o: gthreads/gtswtch.s | $(OBJS_DIR)
	$(CC) $(CFLAGS_DEBUG) gthreads/gtswtch.s -o $@

$(OBJS_DIR)/gtswtch-release.o: gthreads/gtswtch.s | $(OBJS_DIR)
	$(CC) $(CFLAGS_RELEASE) gthreads/gtswtch.s -o $@

$(OBJS_DIR)/%-debug.o: %.c | $(OBJS_DIR)
	$(CC) $(CFLAGS_DEBUG) $< -o $@

$(OBJS_DIR)/%-release.o: %.c | $(OBJS_DIR)
	$(CC) $(CFLAGS_RELEASE) $< -o $@

$(OBJS_DIR)/gthread-debug.o: gthreads/gthreads.c | $(OBJS_DIR)
	$(CC) $(CFLAGS_DEBUG) $< -o $@

$(OBJS_DIR)/gthread-release.o: gthreads/gthreads.c | $(OBJS_DIR)
	$(CC) $(CFLAGS_RELEASE) $< -o $@

# exes
# you will need exe and exe-debug
$(EXE_MAIN): $(OBJS_ALL:%.o=$(OBJS_DIR)/%-release.o)
	$(LD) $^ $(LDFLAGS) -o $@

$(EXE_MAIN)-debug: $(OBJS_ALL:%.o=$(OBJS_DIR)/%-debug.o)
	$(LD) $^ $(LDFLAGS) -o $@

$(EXE_GTEST): $(OBJS_GTEST_DEP:%.o=$(OBJS_DIR)/%-release.o)
	$(LD) $^ $(LDFLAGS) -o $@

$(EXE_GTEST)-debug: $(OBJS_GTEST_DEP:%.o=$(OBJS_DIR)/%-debug.o)
	$(LD) $^ $(LDFLAGS) -o $@

$(EXE_TEST): $(OBJS_TEST_DEP:%.o=$(OBJS_DIR)/%-release.o)
	$(LD) $^ $(LDFLAGS) -o $@

$(EXE_TEST)-debug: $(OBJS_TEST_DEP:%.o=$(OBJS_DIR)/%-debug.o)
	$(LD) $^ $(LDFLAGS) -o $@

.PHONY : clean
clean:
	rm -rf .objs $(EXE_MAIN) $(EXE_MAIN:%=%-debug) $(EXE_GTEST) $(EXE_GTEST:%=%-debug) $(EXE_TEST) $(EXE_TEST:%=%-debug)
