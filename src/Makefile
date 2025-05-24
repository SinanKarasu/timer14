

BINNAME1 = timer14m
## define a BINDIR
BINDIR = ../dummy
BIN1  = $(BINDIR)/$(BINNAME1)

$(BINDIR)/timer14m := INCLUDES=  -I. -I/usr/src/uts/sun4m
$(BINDIR)/timer14m := DEFINES = -D_KERNEL -DSUNDDI -Dsun4m

CFLAGS  = -O 
CPPFLAGS= $(DEFINES) $(INCLUDES)
INC.h   = timer14_io.h timer14_reg.h

all:	
	make $(BIN1)

$(BIN1):	$(INC.h)    timer14m.o swap_word.o ld_st_alt.o
	ld -r -o $@ timer14m.o swap_word.o ld_st_alt.o

clean:
	-rm -f *.o $(BIN1) $(CONF)
