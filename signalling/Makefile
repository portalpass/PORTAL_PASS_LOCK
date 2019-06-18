#To add a main file, put it in "TARGETS"
#This assumes it will have a respective cpp file
#copy an existing main rule and replace its trail with the appropriate object file

#All headers must be in DEPS because they will require full compiling if they redefine function

#Headers are in the IDIR Directory, Cpp and Compiled source are in the SDIR (current) directory, LIBS are in LDIR
#RDIR is the root of the github directory

#To add a header template / implementation pair, add the header to _DEPS and the
#implementation to _OBJ

CONFIG_DIR   := ${HOME}/.config/PORTAL_PASS
CONFIG_FILES := qr.txt setup.txt paired_serials.txt

SDIR := ${CURDIR}
RDIR := ${CURDIR}/..
IDIR := ${RDIR}/include
ODIR := ${RDIR}/obj
LDIR := ${RDIR}/lib
BIN_DIRECTORY := /usr/local/bin

CXX      := g++
CXXFLAGS := -std=c++11 -O2 -I$(IDIR) 
LDFLAGS  := -lcrypto -lzmq

TARGETS  := lockSender lockReceiver

#download appropriate header from the agora website (in the readme) and put in LDIR
_LIBS   := libagorasig.so
LIBS    := $(patsubst %,$(LDIR)/%,$(_LIBS))

#list of header files that are needed for proper linking
_DEPS   := receiverCallBack.h senderCallBack.h lockAPI.h helper_functions.h agora_sig.h
DEPS    := $(patsubst %,$(IDIR)/%,$(_DEPS))

#List of mains which need to be excluded from linking all other mains
_MAINS  := $(addsuffix .o, $(TARGETS) )
MAINS   := $(patsubst %,$(ODIR)/%,$(_MAINS))

#list of all object files that will be compiled from src to obj
_OBJ   := receiverCallBack.o senderCallBack.o lockAPI.o helper_functions.o $(_MAINS)
OBJ    := $(patsubst %,$(ODIR)/%,$(_OBJ))

.PHONY: all install uninstall remove clean

#make all will make all binaries, and make blank config files in ~/.config/PORTAL_PASS
all: $(TARGETS) 
	@if [ ! -d $(CONFIG_DIR) ]; then mkdir -p $(CONFIG_DIR) && echo "making DIR $(CONFIG_DIR)"; fi
	@if [[ ! -f $(CONFIG_DIR)/setup.txt ]] && [[ -f setup.txt.def ]]; then cp -f setup.txt.def $(CONFIG_DIR)/setup.txt; fi
	@for i in $(CONFIG_FILES); do if [ ! -f $(CONFIG_DIR)/$$i ]; then touch $(CONFIG_DIR)/$$i; fi; done

install: all
	for i in $(TARGETS); do cp -f $$i $(BIN_DIRECTORY)/$$i; done

uninstall:
	for i in $(TARGETS); do rm -f $(BIN_DIRECTORY)/$$i; done

remove:
	rm -f $(TARGETS)

#delete object files, to force everything to recompile
clean:
	rm -f $(ODIR)/*.o

#build needed object files from all source code,
$(OBJ): $(ODIR)/%.o : $(SDIR)/%.cpp $(DEPS)
	$(CXX) -c -o $@ $< $(CXXFLAGS)

#EACH MAIN will depend on and link all OBJ files except for the other mains
$(TARGETS): % : $(filter-out $(MAINS), $(OBJ)) $(ODIR)/%.o
	$(CXX) -o $@ $(LIBS) $^ $(CXXFLAGS) $(LDFLAGS)