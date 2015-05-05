% LBFGS_BUILD_PATH is the path where LBFGS was built
function compileMex(LBFGS_BUILD_PATH, OUTPUT_PATH, debug)

if nargin < 2
    OUTPUT_PATH = '../../build';
end

if nargin < 3
    debug = false;
end

switch computer
    case {'GLNXA64', 'GLNX86'}
        mexCmd = ...
            sprintf('mex -DMEX -I../../include -lrt -I%s/include %s/lib/.libs/lbfgs.o', LBFGS_BUILD_PATH, LBFGS_BUILD_PATH);
        mexCmd = [mexCmd ' LDFLAGS=''\$LDFLAGS -fopenmp'' CXXFLAGS=''\$CXXFLAGS -mtune=native -march=native -fopenmp'''];
    case {'MACI64'}
        mexCmd = ...
            sprintf('mex -DMEX -I../../include -I%s/include %s/lib/.libs/liblbfgs.a', LBFGS_BUILD_PATH, LBFGS_BUILD_PATH);
        mexCmd = [mexCmd ' LDFLAGS=''\$LDFLAGS -fopenmp'' CXXFLAGS=''\$CXXFLAGS -fopenmp -msse2'''];
    case {'PCWIN', 'PCWIN64'}
        mexCmd = ...
            sprintf('mex -DMEX -I../../include -I%s/include -L%s/Release -llbfgs', LBFGS_BUILD_PATH, LBFGS_BUILD_PATH);
        mexCmd = [mexCmd ' LINKFLAGS="$LINKFLAGS /openmp" COMPFLAGS="$COMPFLAGS /openmp"'];
    otherwise
        error('Could not identify architecture type');
end

if debug
    mexCmd = [mexCmd ' -g'];
else
    mexCmd = [mexCmd ' -O'];
end

mkdir(OUTPUT_PATH); % in case it doesn't exist

mexCmdTrain = [mexCmd sprintf(' -DSQBTREES_TRAIN SQBTrees.cpp -output %s/SQBMatrixTrain', OUTPUT_PATH)];
mexCmdPred = [mexCmd sprintf(' SQBTrees.cpp -output %s/SQBMatrixPredict', OUTPUT_PATH)];

eval(mexCmdTrain);
eval(mexCmdPred);


