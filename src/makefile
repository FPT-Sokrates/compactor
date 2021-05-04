IDIR =../include
CC=gcc
CFLAGS=-I$(IDIR) -Wall

ODIR=obj
LDIR =../lib

LIBS=

_DEPS = commandLineParameter.h printHelpText.h readInputByte.h roadc.h compactor.h readInput.h readInputFile.h writeOutputFile.h
DEPS = $(patsubst %,$(IDIR)/%,$(_DEPS))

_OBJ = compactor.o commandLineParameter.o printHelpText.o readInput.o readInputByte.o readInputFile.o roadc.o writeOutputFile.o
OBJ = $(patsubst %,$(ODIR)/%,$(_OBJ))


$(ODIR)/%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

compactor: $(OBJ)
	$(CC) -o $@ $^ $(CFLAGS) $(LIBS)

.PHONY: clean

clean:
	rm -f $(ODIR)/*.o *~ core $(INCDIR)/*~

