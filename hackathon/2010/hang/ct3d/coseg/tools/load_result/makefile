CC = gcc
CXX = g++ -g -Wall

CXXFLAGS = -ltiff

OBJ_LOAD_RESULT_IMAGES = ../../myalgorithms.o ../../component_tree.o ../../CT3D/cell_track.o ../../CT3D/palette.o load_result_images.o

load_result_images:$(OBJ_LOAD_RESULT_IMAGES)
	$(CXX) $(CXXFLAGS) -llpsolve55 $(OBJ_LOAD_RESULT_IMAGES) -o load_result_images

(%.o):(%.cpp)
	$(CXX) $(CXXFLAGS) -c $< -o $*.o
install:
	cp load_result_images ../bin
clean:
	rm *.o
	rm ../../*.o
	rm ../../CT3D/*.o
