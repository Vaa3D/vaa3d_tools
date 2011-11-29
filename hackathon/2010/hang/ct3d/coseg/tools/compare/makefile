CC = gcc
CXX = g++ -g -Wall

CXXFLAGS = -ltiff -llpsolve55

OBJ_COMPARE = ../../myalgorithms.o ../../component_tree.o ../../CT3D/bipartite.o methods_compare.o

all: methods_compare

methods_compare: $(OBJ_COMPARE)
	$(CXX) $(CXXFLAGS) $(OBJ_COMPARE) -o methods_compare

(%.o):(%.cpp)
	$(CXX) $(CXXFLAGS) -c $< -o $*.o
install:
	cp methods_compare ../bin
clean:
	rm -f *.o
	rm -f ../../*.o
	rm -f ../../CT3D/*.o
