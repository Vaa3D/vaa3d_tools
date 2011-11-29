CC = gcc
CXX = g++ -g 

CXXFLAGS = -ltiff -llpsolve55

OBJ_COSEG = ../../myalgorithms.o ../../component_tree.o ../../CT3D/cell_track.o ../../CT3D/palette.o ../../CT3D/bipartite.o coseg.o
OBJ_COSEG0 = ../../myalgorithms.o ../../component_tree.o ../../CT3D/cell_track.o ../../CT3D/palette.o ../../CT3D/bipartite.o coseg0.o
OBJ_COSEG1 = ../../myalgorithms.o ../../component_tree.o ../../CT3D/cell_track.o ../../CT3D/palette.o ../../CT3D/bipartite.o coseg1.o

all: coseg coseg0 coseg1

coseg:$(OBJ_COSEG)
	$(CXX) $(CXXFLAGS) $(OBJ_COSEG) -o coseg
coseg0:$(OBJ_COSEG0)
	$(CXX) $(CXXFLAGS) $(OBJ_COSEG0) -o coseg0
coseg1:$(OBJ_COSEG1)
	$(CXX) $(CXXFLAGS) $(OBJ_COSEG1) -o coseg1

(%.o):(%.cpp)
	$(CXX) $(CXXFLAGS) -c $< -o $*.o
install:
	cp coseg ../bin
	cp coseg0 ../bin
	cp coseg1 ../bin
clean:
	rm -f *.o
	rm -f ../../*.o
	rm -f ../../CT3D/*.o
