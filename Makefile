DEBUGFLAGS = -g
OPTFLAGS = -O0
CXXFLAGS = $(DEBUGFLAGS) $(OPTFLAGS) 

all:
	g++ $(CXXFLAGS) -o flow_optimisation main.cpp -lfltk -lfltk_images