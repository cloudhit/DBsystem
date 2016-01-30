
.PHONY: depend clean backup setup

MAIN=hfpage

MINIBASE=..

CC=g++

CFLAGS= -DUNIX -Wall -g

INCLUDES = -I${MINIBASE}/include -I.

# LFLAGS= -L${MINIBASE}/lib -liberty -lm

SRCS = main.C hfpage.C hfp_driver.C test_driver.C \
		db.C new_error.C page.C system_defs.C

OBJS = $(SRCS:.C=.o)

$(MAIN):  $(OBJS)
	 $(CC) $(CFLAGS) $(INCLUDES) $(OBJS) buf.o -o $(MAIN) $(LFLAGS)

.C.o:
	$(CC) $(CFLAGS) $(INCLUDES) -c $<

depend: $(SRCS)
	makedepend $(INCLUDES) $^

clean:
	mv buf.o buf.o.bak	
	rm -f *.o *~ $(MAIN) $(MAKECLEANGARBAGE) 
	mv buf.o.bak buf.o

backup:
	mkdir bak
	cp Makefile *.[Ch] bak

# Grab the sources for a user who has only the makefile
setup:
	/bin/cp -i $(MINIBASE)/src/*.[Ch] .
	-/bin/cp -i $(MINIBASE)/src/README .

# DO NOT DELETE THIS LINE -- make depend needs it 

