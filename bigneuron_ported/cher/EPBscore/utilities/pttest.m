function [p, t, df] = pttest(d1, d2)
%PTTEST Student's paired t-test.
%       PTTEST(X1, X2) gives the probability that Student's t
%       calculated on paired data X1 and X2 is higher than
%       observed, i.e. the "significance" level. This is used
%       to test whether two paired samples have significantly
%       different means.
%       [P, T] = PTTEST(X1, X2) gives this probability P and the
%       value of Student's t in T. The smaller P is, the more
%       significant the difference between the means.
%       E.g. if P = 0.05 or 0.01, it is very likely that the
%       two sets are sampled from distributions with different
%       means.
%
%       This works for PAIRED SAMPLES, i.e. when elements of X1
%       and X2 correspond one-on-one somehow.
%       E.g. residuals of two models on the same data.
%
%See also: TTEST, UTTEST.
[l1 c1] = size(d1) ;
n1 = l1 * c1 ;
x1 = reshape(d1, l1 * c1, 1) ;
[l2 c2] = size(d2) ;
n2 = l2 * c2 ;
if (n1 ~= n2)
   error('PTTEST: paired samples must have the same number of elements !')
end
x2 = reshape(d2, l2 * c2, 1) ;
[a1 v1] = avevar(x1) ;
[a2 v2] = avevar(x2) ;
df  = n1 - 1 ;
cab = (x1 - a1)' * (x2 - a2) / (n1 - 1) ;
if (a1 ~= a2)
  % use abs to avoid numerical errors for very similar data
  % for which v1+v2-2cab may be close to 0.
  t = (a1 - a2) / sqrt(abs(v1 + v2 - 2 * cab) / n1) ;
  p = betainc( df / (df + t*t), df/2, 0.5) ;
else
  t = 0 ;
  p = 1 ;
end
