CC = gcc
CXX = g++ -g -Wall

CXXFLAGS = -ltiff

OBJ_CREATE = ../../myalgorithms.o ../../component_tree.o  create_tree.o
OBJ_VIEW_TREE = ../../myalgorithms.o ../../component_tree.o  view_tree.o
OBJ_VIEW_DEGREE = ../../myalgorithms.o ../../component_tree.o  view_degree.o
OBJ_TREE_GRAPH = ../../myalgorithms.o ../../component_tree.o  tree_graph.o
OBJ_TREE_HEADER = ../../myalgorithms.o ../../component_tree.o  tree_header.o

all: view_tree create_tree view_degree tree_graph tree_header

tree_graph: $(OBJ_TREE_GRAPH)
	$(CXX) $(CXXFLAGS) $(OBJ_TREE_GRAPH) -o tree_graph
tree_header: $(OBJ_TREE_HEADER)
	$(CXX) $(CXXFLAGS) $(OBJ_TREE_HEADER) -o tree_header
view_tree: $(OBJ_VIEW_TREE)
	$(CXX) $(CXXFLAGS) $(OBJ_VIEW_TREE) -o view_tree
view_degree: $(OBJ_VIEW_DEGREE)
	$(CXX) $(CXXFLAGS) $(OBJ_VIEW_DEGREE) -o view_degree
create_tree: $(OBJ_CREATE)
	$(CXX) $(CXXFLAGS) $(OBJ_CREATE) -o create_tree

(%.o):(%.cpp)
	$(CXX) $(CXXFLAGS) -c $< -o $*.o
install:
	cp create_tree ../bin
	cp view_tree ../bin
	cp view_degree ../bin
	cp tree_header ../bin
	cp tree_graph ../bin
clean:
	rm -f *.o
	rm -f ../../*.o
