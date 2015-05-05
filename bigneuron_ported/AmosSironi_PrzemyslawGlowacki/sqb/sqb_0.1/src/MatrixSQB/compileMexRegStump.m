% LBFGS_BUILD_PATH is the path where LBFGS was built
function compileMexRegStump(LBFGS_BUILD_PATH, OUTPUT_PATH, debug)

if nargin < 2
    OUTPUT_PATH = '../../build';
end

if nargin < 3
    debug = false;
end


mexCmd = ...
    sprintf('mex -DMEX -I../../include -I%s/include -lrt %s/lib/.libs/lbfgs.o', LBFGS_BUILD_PATH, LBFGS_BUILD_PATH);

mexCmdOpenmp = [mexCmd ' LDFLAGS=''\$LDFLAGS -fopenmp'' CXXFLAGS=''\$CXXFLAGS -mtune=native -march=native -fopenmp'''];
mexCmd = [mexCmd ' LDFLAGS=''\$LDFLAGS'' CXXFLAGS=''\$CXXFLAGS -mtune=native -march=native'''];

%mexCmd = mexCmdOpenmp;

if debug
    mexCmd = [mexCmd ' -g'];
else
    mexCmd = [mexCmd ' -O'];
end

mkdir(OUTPUT_PATH); % in case it doesn't exist

mexCmdTrain = [mexCmdOpenmp sprintf(' -DSQB_TRAIN LDARegStump.cpp -output %s/LDARegStumpTrain', OUTPUT_PATH)];
mexCmdPred = [mexCmd sprintf(' LDARegStump.cpp -output %s/LDARegStumpPredict', OUTPUT_PATH)];

mexCmdLS = [mexCmdOpenmp sprintf(' mexLineSearch.cpp -output %s/mexLineSearch', OUTPUT_PATH)];

eval(mexCmdLS);
eval(mexCmdTrain);
eval(mexCmdPred);
