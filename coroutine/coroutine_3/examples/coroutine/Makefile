PROJECT := $(shell pwd)
SRCDIR  := $(PROJECT)/../../
SRC := $(SRCDIR)switch_to.S $(SRCDIR)sched.c $(SRCDIR)rbtree.c $(SRCDIR)syscall.c $(SRCDIR)event.c $(SRCDIR)call_to.S $(SRCDIR)call.c
HEADER := $(SRCDIR)co.h  $(SRCDIR)compiler.h  $(SRCDIR)list.h  $(SRCDIR)rbtree.h $(SRCDIR)co_inner.h
OBJS := coroutine

CFLAGS += -g -O2 -std=gnu99 -Wp,-MMD,.$(notdir $@).d $(EXTRA_$@)

ifeq ($(ARCH), i386)
    CFLAGS += -m32
    ASFLAGS += -m32
endif
CC		= $(CROSS_COMPILE)gcc
AR		= $(CROSS_COMPILE)ar
STRIP   = $(CROSS_COMPILE)strip


debug:$(OBJS)
	@:

release: $(OBJS)
	@$(STRIP) $^

-include $(foreach o, $(OBJS) $(addsuffix .o,$(basename $(SRC))), .$(notdir $o).d)

$(OBJS) : libco.a
$(OBJS) : % : %.c
	$(CC) $(CFLAGS) -o $@ $< -L. -lco -lrt

libco.a : Makefile
libco.a : $(addsuffix .o,$(basename $(SRC)))
	$(AR) rcs $@ $(filter %.o,$^)

%.o : %.c
	$(CC) $(CFLAGS) -c -o $@ $<
%.o : %.S
	$(CC) $(ASFLAGS) -c -o $@ $<

switch_to.S call_to.S : co_offsets.h

co_offsets.h : autogen
	./autogen > $@

clean:
	@rm -f $(OBJS) *.o *.a .*.d co_offsets.h autogen glibc.c glibc.h