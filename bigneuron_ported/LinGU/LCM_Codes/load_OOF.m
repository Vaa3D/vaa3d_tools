function I = load_OOF(fn,I)

% this function serves to load the OOF estimation from either previously saved file
% or instantly calculate and store it.

fn = ['../Cache/OOF/' fn '.mat'];

if(exist(fn))
    
    load(fn);

else

    I = oof3response(double(I),1:10);

    save(fn,'I');

end
