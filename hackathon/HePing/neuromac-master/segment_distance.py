import numpy as np

def dist3D_segment_to_segment(p0,p1,p2,p3): # S1=(p0,p1), S2=(p2,3)
    # c++ from http://geomalgorithms.com/a07-_distance.html#dist3D_Segment_to_Segment%28%29
    SMALL_NUM =  0.00000001
    dot = lambda u,v: np.dot(u,v)
    norm = lambda v: np.sqrt(dot(v,v))
    d = lambda u,v: norm(u-v)
    
    u = p1-p0 # Vector   u = S1.P1 - S1.P0
    v = p3-p2 # Vector   v = S2.P1 - S2.P0
    w = p0-p2 # Vector   w = S1.P0 - S2.P0
    a = dot(u,u)         # always >= 0
    b = dot(u,v)
    c = dot(v,v)         # always >= 0
    d = dot(u,w)
    e = dot(v,w)
    D = a*c - b*b        # always >= 0
    sc, sN, sD = D,D,D       # sc = sN / sD, default sD = D >= 0
    tc, tN, tD = D,D,D       # tc = tN / tD, default tD = D >= 0

    # compute the line parameters of the two closest points
    if (D < SMALL_NUM):  # the lines are almost parallel
        sN = 0.0         # force using point P0 on segment S1
        sD = 1.0         # to prevent possible division by 0.0 later
        tN = e
        tD = c
    
    else:                 # get the closest points on the infinite lines
        sN = (b*e - c*d)
        tN = (a*e - b*d)
        if (sN < 0.0):        # sc < 0 => the s=0 edge is visible
            sN = 0.0
            tN = e
            tD = c
        
        elif (sN > sD):  # sc > 1  => the s=1 edge is visible
            sN = sD
            tN = e + b
            tD = c

    if (tN < 0.0):            # tc < 0 => the t=0 edge is visible
        tN = 0.0
        # recompute sc for this edge
        if (-d < 0.0):
            sN = 0.0
        elif (-d > a):
            sN = sD
        else:
            sN = -d
            sD = a
    
    elif (tN > tD):      # tc > 1  => the t=1 edge is visible
        tN = tD
        # recompute sc for this edge
        if ((-d + b) < 0.0):
            sN = 0
        elif ((-d + b) > a):
            sN = sD
        else:
            sN = (-d +  b)
            sD = a


    # finally do the division to get sc and tc
    sc,tc = 0.0,0.0
    # sc = (abs(sN) < SMALL_NUM ? 0.0 : sN / sD)
    if np.abs(sN) < SMALL_NUM:
        sc = 0.0
    else:
        sc = sN / sD

    #tc = (abs(tN) < SMALL_NUM ? 0.0 : tN / tD)
    if np.abs(tN) < SMALL_NUM:
        tc = 0.0
    else:
        tc = tN/tD

    # get the difference of the two closest points
    dP = w + (sc * u) - (tc * v)  # =  S1(sc) - S2(tc)

    return norm(dP)   # return the closest distance

