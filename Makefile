INST_DIR = $(HOME)/bin
BIN  = mod_reprocessed_data

# need to include the math library when the math.h header file is used
#cflags = -O -lm
cflags = -O 
#CFLAGS = $(DBG) $(cflags) -arch i386
#FFLAGS = -O2 -ffixed-line-length-none 
CFLAGS = $(DBG) $(cflags) 
FFLAGS = -O2 -ffixed-line-length-none -m64

FC = gfortran
CC = gcc

DBG = 
FOBJS = $(BIN).o read_line_fm_csvFile.o delaz.o compute_doy.o compute_epochTime.o

$(BIN) : $(FOBJS)
	$(CC) $(CFLAGS) $(FOBJS) -o $(BIN)

install :: $(BIN)
	install -s $(BIN) $(INST_DIR)

clean ::
	rm -f $(BIN) $(FOBJS)
