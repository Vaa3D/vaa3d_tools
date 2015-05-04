function [p, t] = ttest(d1, d2)
%TTEST Student's t-test for equal variances.
%       TTEST(X1, X2) gives the probability that Student's t
%       calculated on data X1 and X2, sampled from distributions
%       with the same variance, is higher than observed, i.e.
%       the "significance" level. This is used to test whether
%       two sample have significantly different means.
%       [P, T] = TTEST(X1, X2) gives this probability P and the
%       value of Student's t in T. The smaller P is, the more
%       significant the difference between the means.
%       E.g. if P = 0.05 or 0.01, it is very likely that the
%       two sets are sampled from distributions with different
%       means.
%
%       This works if the samples are drawn from distributions with
%       the SAME VARIANCE. Otherwise, use UTTEST.
%
%See also: UTTEST, PTTEST.
[l1 c1] = size(d1) ;
n1 = l1 * c1 ;
x1 = reshape(d1, l1 * c1, 1) ;
[l2 c2] = size(d2) ;
n2 = l2 * c2 ;
x2 = reshape(d2, l2 * c2, 1) ;
[a1 v1] = avevar(x1) ;
[a2 v2] = avevar(x2) ;
df = n1 + n2 - 2 ;
pvar = ((n1 - 1) * v1 + (n2 - 1) * v2) / df ;
t = (a1 - a2) / sqrt( pvar * (1/n1 + 1/n2)) ;
p = betainc( df / (df + t*t), df/2, 0.5) ;
