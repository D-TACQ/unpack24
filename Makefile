
CXXFLAGS=-O2 -std=c++11 

CXXFLAGS+=-DMAKE_MAIN
#CXXFLAGS+=-fopenmp
CFLAGS+=-O2 -std=c99

APPS=unpack24

all: $(APPS)

clean:
	rm -f $(APPS)

install_user: all
	cp $(APPS) $(HOME)/bin



	
