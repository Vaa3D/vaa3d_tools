function [Ppos,Pneg,Pboth]=Fisherextest(a,b,c,d)
%[Ppos,Pneg,Pboth]=Fisherextest(a,b,c,d)- Fisher's Exact Probability Test.
%   FISHEREXTEST performs the Fisher exact probability test for a table of 
%   frequency data cross-classified according to two categorical variables, 
%   each of which has two levels or subcategories (2x2). It is a non-parametric
%   statistical test used to determine if there are nonrandom associations 
%   between the two categorical variables. Fisher's exact test is used to 
%   calculate an exact P-value with small number of expected frequencies, for
%   which the Chi-square test is not appropriate (in case the total number of
%   observations is less than 20 or there is a cell-value less than 5). This
%   test is based upon the hypergeometric probability. The test was proposed in
%   the 1934 edition of the famous Ronald Aylmer Fisher's book 'Statistical 
%   Methods for Research Workers'.
%
%   So, according to the next 2x2 table design,
%
%                    Var.1
%                --------------
%                  a       b      r1=a+b
%         Var.2
%                  c       d      r2=c+d
%                --------------
%                c1=a+c  c2=b+d  n=c1+c2
%
%   The Fisher's exact test it is conditioned to all the a+b values (all the 
%   possible cell combinations that would still result in the marginal frequencies
%   as highlighted) such that,
%
%          H(a+b) = {X: X € H and a+b = r1}.
%
%   Then,
%
%          P(a,b|a+b,n) = P(X|X € H(r1)) = b(a;c1,p)*b(b;c2,p)/b(a+b;n,p).
%
%   This binomials relationship reduces each 2x2 table to the exact hypergeometric 
%   distribution to compute the P-value. Now considering only the binomial 
%   coefficients,
%
%          P(X|X € H(r1)) = C(c1,a)*C(c2,b)/C(n,a+b)
%
%   Thereby Fisher's exact P-values are readily evaluated as,
%
%          P = SUM(P(X|X € H(r1))).        
%
%   IMPORTANT: Due that Matlab could not work for factorials greater than 170. We
%   use the function sum([log(x+1)......]), in order to avoid further calculation 
%   problems.
%
%   Syntax: function [Ppos,Pneg,Pboth]=Fisherextest(a,b,c,d) 
%
%   $$-If user does not put any output arguments, file returns a P-value table.-$$
%      
%   Inputs:
%         a,b,c,d - observed frequency cells
%
%   Output:
%         Three p-values [decide to use Left, Right or 2-Tail before
%         collecting (or looking at) the data]: 
%           - Right tail (Ppos): Use this when the alternative to independence is that there is
%             positive association between the variables. That is, the observations 
%             tend to lie in upper left and lower right. 
%           - Left tail (Pneg): Use this when the alternative to independence is that there is 
%             negative association between the variables. That is, the observations
%             tend to lie in lower left and upper right. 
%           - 2-Tail (Pboth): Use this when there is no prior alternative. 
%
%   Example: From the example given on the Ina Parks S. Howell's internet homepage 
%            (http://www.fiu.edu/~howellip/Fisher.pdf). Suppose Crane and Egret are
%            two very small collages, the results of the beginning physics course at
%            each of the two schools are given in the follow table.
%
%                                   Physics
%                              Pass         Fail
%                            ---------------------
%                      Crane     8           14
%            Collage                 
%                      Egret     1            3
%                            ---------------------
%                                       
%   Calling on Matlab the function: 
%             Fisherextest(8,14,1,3)
%
%   Answer is:
%
%   Fisher's exact test P-values for cells:
%   a = 8, b = 14, c = 1, d = 3
%   ----------------------------------------------
%                      P-value  
%   ----------------------------------------------
%     Left tail       Right tail         2-tail   
%     (negative)      (positive)         (both)  
%   ----------------------------------------------
%   0.8412876883     0.5685618729     1.0000000000
%   ----------------------------------------------
%
%  $-We thank Dr. Stan Letovsky for his valuable suggestions to improve this m-file.-$
%
%  Created by A. Trujillo-Ortiz, R. Hernandez-Walls and A. Castro-Perez
%             Facultad de Ciencias Marinas
%             Universidad Autonoma de Baja California
%             Apdo. Postal 453
%             Ensenada, Baja California
%             Mexico.
%             atrujo@uabc.mx
%             And the special collaboration of the post-graduate students of the 2004:2
%             Multivariate Statistics Course: Laura Rodriguez-Cardozo, Norma Alicia Ramos-Delgado,
%             and Rene Garcia-Sanchez. 
%  Copyright (C) September 27, 2004.
%
%  To cite this file, this would be an appropriate format:
%  Trujillo-Ortiz, A., R. Hernandez-Walls, A. Castro-Perez, L. Rodriguez-Cardozo 
%    N.A. Ramos-Delgado and R. Garcia-Sanchez. (2004). Fisherextest:Fisher's Exact
%    Probability Test. A MATLAB file. [WWW document]. URL http://www.mathworks.com/
%    matlabcentral/fileexchange/loadFile.do?objectId=5957
%
%  References:
% 
%  Agresti, A. (1992), A Survey of Exact Inference for Contegency Tables. 
%           Statistical Science,7:131-153.
%  Fisher, R.A. (1934), Statistical Methods for Research Workers. Chapter 12. 
%           5th Ed., Scotland:Oliver & Boyd.
%  Howell, I.P.S. (Internet homepage), http://www.fiu.edu/~howellip/Fisher.pdf
%  Zar, J.H. (1999), Biostatistical Analysis (2nd ed.). NJ: Prentice-Hall,
%           Englewood Cliffs. p. 543-555. 
%

if nargin < 4, 
   error('You need to input four arguments.');
   return,
end;

tt =  [a b;c d];
t = [a b;c d];
pp = t(:,1)./sum(t')';

if (pp(1) < pp(2))
   if a == 0
      t = t;
  elseif d == 0
      t = t([2,1],[2,1]);
  else
      t = t([2,1],:);
  end;
end;

a = t(1,1);
b = t(1,2);
c = t(2,1);
d = t(2,2);

r1 = a+b;
r2 = c+d;
c1 = a+c;
c2 = b+d;
n = c1+c2;

ap = [0:min([r1 c1])]';
bp = r1-ap;
cp = c1-ap;
dp = r2-cp;
len = length(ap);
factor1 = sum([log(1:r1) log(1:r2) log(1:c1) log(1:c2) -log(1:n)]);

for k = 1:len
   factor2 = sum([log(1:ap(k)) log(1:bp(k)) log(1:cp(k)) log(1:dp(k))]);
   Pap(k) = factor1-factor2;
end;

Pap = exp(Pap);
P1 = sum(Pap(find(ap <= a)));
if P1 > 1;
   P1 = 1;
else
   P1;
end;

P2 = sum(Pap(find(ap >= a)));
if P2 > 1;
   P2 = 1;
else
   P2;
end;

factor2 = sum([log(1:a) log(1:b) log(1:c) log(1:d)]);

Pa = exp(factor1-factor2);
P3 = sum(Pap(find(Pap <= Pa)));
if P3 > 1;
   P3 = 1;
else
   P3;
end;

a=tt(1,1);
b=tt(1,2);
c=tt(2,1);
d=tt(2,2);
    
if nargout ~= 0,
    disp(' ')
    disp('Fisher''s exact test P-values for cells:')
    disp(['a = ' num2str(a) ', b = ' num2str(b) ', c = ' num2str(c) ', d = ' num2str(d) '']);
    Ppos=P2;
    Pneg=P1;
    Pboth=P3;
else
    disp(' ')
    disp('Fisher''s exact test P-values for cells:')
    disp(['a = ' num2str(a) ', b = ' num2str(b) ', c = ' num2str(c) ', d = ' num2str(d) '']);
    fprintf('----------------------------------------------\n');
    disp('                   P-value  '); 
    fprintf('----------------------------------------------\n');
    disp('  Left tail       Right tail         2-tail  '); 
    disp('  (negative)      (positive)         (both)  '); 
    fprintf('----------------------------------------------\n');
    fprintf('%10.10f     %10.10f     %10.10f\n',[P1,P2,P3].');
    fprintf('----------------------------------------------\n');
end;
return;