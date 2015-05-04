function [xbar, varx] = avevar(v)
%AVEVAR average and variance of sample.
%       AVEVAR(X) gives the average of the sample in X.
%       X is a vector of values.
%       [A, V] = AVEVAR(X) returns the average of X in A,
%       and the variance in V. The variance is corrected
%       using the two-pass formula.
%
%       Ref: [1] Chan, Golub and LeVeque. 1983. American
%                Statistician, vol. 37, pp. 242--247.
%            [2] Press et al. 1992. Numerical recipes in C.
%                Cambridge university press.
[n l] = size(v) ;
x = reshape(v, n*l, 1) ;
xbar = sum(x) / n / l ;
d = x - xbar ;
varx = ( sum(d .* d) - sum(d) * sum(d) / n / l) / (n * l - 1) ;
