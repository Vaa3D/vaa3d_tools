function out=clockToString(t)
% converts a clock vector into a nice string

    t=fix(t);
    out=sprintf('%d/%d/%d %d:', t(2), t(3), t(1), t(4));
    if t(5)<10
        out=[out sprintf('0%d:', t(5))];
    else
        out=[out sprintf('%d:', t(5))];
    end
    if t(6)<10
        out=[out sprintf('0%d', t(6))];
    else
        out=[out sprintf('%d', t(6))];
    end
    