DEBUGFLAGS = -g
OPTFLAGS = -O0
VERSION = #-std=c++20
INCLUDES = #-I /usr/include
LIBS = #-L /usr/lib -lboost_math_tr1
CXXFLAGS = $(INCLUDES) $(LIBS) $(DEBUGFLAGS) $(OPTFLAGS) $(VERSION) -Wall


all:
	g++ $(CXXFLAGS) -o flow_optimisation main.cpp -lfltk -lfltk_images