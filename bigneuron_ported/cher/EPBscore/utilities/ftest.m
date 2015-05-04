function [p, f] = ftest(d1, d2)
%FTEST F-test for two samples.
%       FTEST(X1, X2) gives the probability that the F value
%       calculated as the rati of the variances of the two samples is
%       greater than observed, i.e. the significance level.
%       [P, F] = FTEST(X1, X2) gives the probability P and returns
%       the value of F.
%
%       A small value of P would lead to reject the hypothesis that
%       both data sets are sampled from distributions with the same
%       variances.
%
%See also : TTEST, TEST.
[l1 c1] = size(d1) ;
n1 = l1 * c1 ;
x1 = reshape(d1, l1 * c1, 1) ;
[l2 c2] = size(d2) ;
n2 = l2 * c2 ;
x2 = reshape(d2, l2 * c2, 1) ;
[a1 v1] = avevar(x1) ;
[a2 v2] = avevar(x2) ;
f = v1 / v2 ;
df1 = n1 - 1 ;
df2 = n2 - 1 ;
if (v1 > v2)
   p = 2 * betainc( df2 / (df2 + df1 * f), df2 / 2, df1 / 2) ;
else
   f = 1 / f ;
   p = 2 * betainc( df1 / (df1 + df2 * f), df1 / 2, df2 / 2) ;
end
if (p > 1)
   p = 2 - p ;
end   