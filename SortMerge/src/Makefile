#
# Define DEBUGREL for some kind of debugging output (not from us, from
# the original Minibase implementors.)
#
# Warning: make depend overwrites this file.

.PHONY: depend clean backup setup

MAIN=SortMerge

MINIBASE = ..

CC=g++

CFLAGS= -DUNIX -Wall -g

INCLUDES = -I${MINIBASE}/include -I.

# LFLAGS= -L$(MINIBASE)/lib \
#        -ldefs -lbt -lbm -lhf -liberty -lm

SRCS =test_driver.C SMJTester.C main.C sortMerge.C sort.C scan.C \
	hfpage.C new_error.C system_defs.C heapfile.C db.C \
	page.C buf.C

OBJS = $(SRCS:.C=.o)

$(MAIN):  $(OBJS)
	 $(CC) $(CFLAGS) $(INCLUDES) $(OBJS) -o $(MAIN)

# Not really "all", but this is useful for setting up the libraries.
all: $(OBJS)

.C.o:
	$(CC) $(CFLAGS) $(INCLUDES) -c $<

depend: $(SRCS)
	makedepend $(INCLUDES) $^

clean:
	mv sort.o sort.o.bak
	mv buf.o buf.o.bak
	mv hfpage.o hfpage.o.bak
	mv heapfile.o heapfile.o.bak
	rm -f *.o *~ $(MAIN)
	mv sort.o.bak sort.o
	mv buf.o.bak buf.o
	mv hfpage.o.bak hfpage.o
	mv heapfile.o.bak heapfile.o

backup:
	mkdir bak
	cp Makefile *.[Ch] bak

# Grab the sources for a user who has only the makefile
setup:
	/bin/cp -i $(MINIBASE)/src/*.[Ch] .
	-/bin/cp -i $(MINIBASE)/src/README .

# DO NOT DELETE THIS LINE -- make depend needs it 
