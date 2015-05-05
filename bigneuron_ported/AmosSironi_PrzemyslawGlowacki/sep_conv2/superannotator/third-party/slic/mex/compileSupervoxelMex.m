mex -O slic_gen_twosided_histograms.cpp CXXFLAGS='\$CXXFLAGS -fopenmp' LDFLAGS='\$LDFLAGS -fopenmp' -I../
mex -O slic_genhistograms.cpp CXXFLAGS='\$CXXFLAGS -fopenmp' LDFLAGS='\$LDFLAGS -fopenmp' -I../
mex -O slic_slicmap.cpp -I../
mex -O slic_supervoxel.cpp ../LKM.cpp ../utils.cpp CXXFLAGS='\$CXXFLAGS -fopenmp' LDFLAGS='\$LDFLAGS -fopenmp' -I../
mex -O slic_superpixel.cpp ../LKM.cpp ../utils.cpp CXXFLAGS='\$CXXFLAGS -fopenmp' LDFLAGS='\$LDFLAGS -fopenmp' -I../
%mex -O slic_supervoxel.cpp ../LKM.cpp ../utils.cpp -I../
mex -O slic_findneighbors.cpp CXXFLAGS='\$CXXFLAGS -fopenmp' LDFLAGS='\$LDFLAGS -fopenmp' -I../
mex -O slic_findcentroids.cpp -I../
