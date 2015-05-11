function [x, y, z] = sph2cart_sq(th, phi, r)
    x = r .* cos(th) .* sin(phi);
    y = r .* sin(th) .* sin(phi);
    z = r .* cos(phi);
end