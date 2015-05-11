function dcos = sphveccos(th1, phi1, th2, phi2)
% Calculate the value of the cosine of the angle between two spherical vectors
% Ref: http://math.stackexchange.com/questions/231221/great-arc-distance-between-two-points-on-a-unit-sphere
% Larger cosine between these two angles means closer these two angles are

dcos = cos(th1) .* cos(th2) + sin(th1) .* sin(th2) .* cos(phi1 - phi2);
end