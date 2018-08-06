# not using standard Makefile template because this makefile creates shared
# objects and weird stuff
CC = clang
WARNINGS = -Wall -Wextra -Werror -Wno-error=unused-parameter
CFLAGS_DEBUG   = -O0 $(WARNINGS) -g -std=c99 -D_GNU_SOURCE -DDEBUG
CFLAGS_RELEASE = -O3 $(WARNINGS) -g -std=c99 -D_GNU_SOURCE

# the string in grep must appear in the hostname, otherwise the Makefile will
# not allow the assignment to compile
IS_VM=$(shell hostname | grep "cs241")

ifeq ($(IS_VM),)
$(error This assignment must be compiled on the CS241 VMs)
endif

INC = -I.
TESTERS = $(patsubst %.c, %, $(wildcard testers/*.c))

all: alloc.so contest-alloc.so mreplace mcontest $(TESTERS:testers/%=testers_exe/%)

alloc.so: alloc.c
	$(CC) $^ $(CFLAGS_DEBUG) -o $@ -shared -fPIC

mreplace: mcontest.c
	$(CC) $^ $(CFLAGS_RELEASE) -o $@ -ldl -lpthread

mcontest: mcontest.c contest.h
	$(CC) $< $(CFLAGS_RELEASE) -o $@ -ldl -lpthread -DCONTEST_MODE


# testers compiled in debug mode to prevent compiler from optimizing away the
# behavior we are trying to test
testers_exe/%: testers/%.c
	@mkdir -p testers_exe/
	$(CC) $^ $(CFLAGS_DEBUG) -o $@

.PHONY : clean
clean:
	-rm -rf *.o alloc.so mreplace mcontest testers_exe/
