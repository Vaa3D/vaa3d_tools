SQBlib is an open-source gradient boosting / boosted trees implementation,
coded fully in C++, offering the possibility to generate mex files to ease the integration with MATLAB.

This is version 0.1. For the latest version, check http://sites.google.com/site/carlosbecker/

Please READ THE FAQ at the end of this document to make sure that you make the most out of this library
and gradient boosting.


=========== Main Features ===========

- Fully-multithreaded (OpenMP)
- MATLAB Mex interface
- Possible losses:
    - Classification: exponential, log
    - Regression: squared

    
=========== Download & Compile (MATLAB mex) ===========

You should first try the pre-compiled binaries, found in the
build/ folder (Linux 64-bit, Mac OS 64-bit and Windows 7 64-bit).
For __MAC__ you need to install the gcc47 port with "sudo port install gcc47"
to use the precompiled mex files.

If you experience issues then you should compile your own version:

1) Download the LBFGS library (used for line search).
    - Decompress LBFGS and compile it with 
       ./configure --disable-shared --enable-static --enable-sse2 && make -j

2) Open MATLAB, and type cd <folder_with_SQBlib>/src/MatrixSQB
    - Compile the mex files with compileMex('<folder_where_lbfgs_is>/')
    - Now the mex files should be compiled in the build/ directory.

For Mac Os, it is recommended to install gcc47 with macports.
Compiling binaries for Windows is trickier, but the precompiled binaries should work with Windows 7.
In linux, works like a charm.



=========== Trainining & prediction ===========

Now we are ready to train a classifier and run predictions.
You can find some examples in tests/classificationTest.m
and tests/regressionTest.m.

(*) Training the classifier:

Syntax is 
    model = SQBMatrixTrain( featureMatrix, labelVector, maxIters, options )
    
    where featureMatrix is of type single of size NxM, where N is the number of samples and M feature dimensionality.
    labelVector is a Nx1 vector, containing labels, either -1/+1 for classification problems or real values for regression.
    __WARNING__: there is no check for a classification loss, so make sure your labels are either +1 or -1.
    
    maxIters is of type uint32, and must be greater than zero.
    
    options is a structure with the following (some optional) fields:
        options.loss = 'exploss', 'logloss' or 'squaredloss'
        options.shrinkageFactor = shrinkage (regularization), (0,1]. Typically 0.1.
        options.subsamplingFactor = (0,1] fraction of training examples sampled randomly at each iteration. Typically 0.5.
        options.maxTreeDepth = uint32, >= 0. Maximum tree depth, if it equals to zero it means a single split with two possible return values.
                                                Typically 2.
        options.disableLineSearch = uint32(0) or uint32(1), if line search should be disabled. Typically leave to false (0).
        options.mtry = similar to random forests, number of features 'explored' per split when building the tree.
                            Optional, if not given then options.mtry = size(featureMatrix,2).
                            In many cases it is good to choose mtry = uint32(ceil(sqrt(size(featureMatrix,2)))) as in RFs.
        options.randseed = random seed, optional, uint32
    

(*) Prediction:
    This one is easier, just call
        scores = SQBMatrixPredict( model, featureMatrix )
            where featureMatrix is of type single.
            
You can always get the description of each required argument by calling the respective mex file with no arguments.


=========== Frequently Asked Questions ===========

a) Shall I use the exponential or the log loss for a classification problem?
    I haven't seen any important behavioral difference between the two, but generally
    the exponential loss does better, given that you apply enough shrinkage and iterate
    long enough.

b) Should I limit the number of iterations to reduce overfitting?
    This is something that was suggested in some publications, but in my experience
    it is better to set the right tree depth and shrinkage factor to proper values,
    so that early stopping is not necessary and adaboost doesn't show overfitting as
    the number of iterations increases. This is a not well understood characteristic of
    boosting: if you set up your weak learners properly, it is better to iterate
    long enough to achieve a certain 'averaging' phase, where generalization
    improves asymptotically, contrary to what common sense would tell.
    
c) What is a typical good value for tree depth and shrinkage?
    Typically I try shrinkage 0.1 and max tree depth = 2. But it depends on each case,
    in particular how powerful the features are.

d) What about mtry?
    In general it improves performance, you can try mtry = sqrt( num of features ).
    Setting this value depends on the power of the features and how many of them are relevant,
    as with Random Forests.
    
c) Is there a connection between Random Forests and Boosted Trees?
    Yes, indeed. If you set shrinkage to a very low value, and set mtry = sqrt( num of features),
    disable line search, and use the exponential loss, then the resulting classifier will be very
    similar to Random Forests. The main difference is that this Boosted Trees uses regression trees,
    that in the case of the exp loss try to reduce misclassification error at each split, while
    Random Forests generally use the gini index or entropy. Some interesting insights are found in
    "Robust Boosting and its Relation to Bagging" (2005) by S. Rosset.

    