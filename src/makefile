IDIR =../include
CC=gcc
CFLAGS=-I$(IDIR) -Wall

ODIR=obj
LDIR =../lib

LIBS=

_DEPS = commandLineParameter.h compactor.h printHelpText.h readInput.h readInputFile.h readInputBinary.h readInputByte.h roadc.h stringHandling.h writeOutputFile.h
DEPS = $(patsubst %,$(IDIR)/%,$(_DEPS))

_OBJ = commandLineParameter.o compactor.o printHelpText.o readInput.o readInputBinary.o readInputByte.o readInputFile.o roadc.o stringHandling.o writeOutputFile.o
OBJ = $(patsubst %,$(ODIR)/%,$(_OBJ))


$(ODIR)/%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

compactor: $(OBJ)
	$(CC) -o $@ $^ $(CFLAGS) $(LIBS)

.PHONY: clean

clean:
	rm -f $(ODIR)/*.o *~ core $(INCDIR)/*~

