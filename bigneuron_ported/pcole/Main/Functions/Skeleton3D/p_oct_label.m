function cube = p_oct_label(octant, label, cube)

% check if there are points in the octant with value 1
if( octant==1 )
    
    % set points in this octant to current label
    % and recurseive labeling of adjacent octants
    idx_1 = find(cube(1,:) == 1);
    if(~isempty(idx_1))
        cube(1,idx_1) = label(idx_1);
    end;
    
    idx_2 = find(cube(2,:) == 1);
    if(~isempty(idx_2))
        cube(2,idx_2) = label(idx_2);
        cube(:,idx_2) = p_oct_label(2,label(idx_2),cube(:,idx_2));
    end;
    
    idx_4 = find(cube(4,:) == 1);
    if(~isempty(idx_4))
        cube(4,idx_4) = label(idx_4);
        cube(:,idx_4) = p_oct_label(3,label(idx_4),cube(:,idx_4));
    end;
    
    idx_5 = find(cube(5,:) == 1);
    if(~isempty(idx_5))
        cube(5,idx_5) = label(idx_5);
        cube(:,idx_5) = p_oct_label(2,label(idx_5),cube(:,idx_5));
        cube(:,idx_5) = p_oct_label(3,label(idx_5),cube(:,idx_5));
        cube(:,idx_5) = p_oct_label(4,label(idx_5),cube(:,idx_5));
    end;
    
    idx_10 = find(cube(10,:) == 1);
    if(~isempty(idx_10))
        cube(10,idx_10) = label(idx_10);
        cube(:,idx_10) = p_oct_label(5,label(idx_10),cube(:,idx_10));
    end;
    
    idx_11 = find(cube(11,:) == 1);
    if(~isempty(idx_11))
        cube(11,idx_11) = label(idx_11);
        cube(:,idx_11) = p_oct_label(2,label(idx_11),cube(:,idx_11));
        cube(:,idx_11) = p_oct_label(5,label(idx_11),cube(:,idx_11));
        cube(:,idx_11) = p_oct_label(6,label(idx_11),cube(:,idx_11));
    end;
    
    idx_13 = find(cube(13,:) == 1);
    if(~isempty(idx_13))
        cube(13,idx_13) = label(idx_13);
        cube(:,idx_13) = p_oct_label(3,label(idx_13),cube(:,idx_13));
        cube(:,idx_13) = p_oct_label(5,label(idx_13),cube(:,idx_13));
        cube(:,idx_13) = p_oct_label(7,label(idx_13),cube(:,idx_13));
    end;
    
end;

if( octant==2 )
    
    idx_2 = find(cube(2,:) == 1);
    if(~isempty(idx_2))
        cube(2,idx_2) = label(idx_2);
        cube(:,idx_2) = p_oct_label(1,label(idx_2),cube(:,idx_2));
    end;

    idx_5 = find(cube(5,:) == 1);
    if(~isempty(idx_5))
        cube(5,idx_5) = label(idx_5);
        cube(:,idx_5) = p_oct_label(1,label(idx_5),cube(:,idx_5));
        cube(:,idx_5) = p_oct_label(3,label(idx_5),cube(:,idx_5));
        cube(:,idx_5) = p_oct_label(4,label(idx_5),cube(:,idx_5));
    end;

    idx_11 = find(cube(11,:) == 1);
    if(~isempty(idx_11))
        cube(11,idx_11) = label(idx_11);
        cube(:,idx_11) = p_oct_label(1,label(idx_11),cube(:,idx_11));
        cube(:,idx_11) = p_oct_label(5,label(idx_11),cube(:,idx_11));
        cube(:,idx_11) = p_oct_label(6,label(idx_11),cube(:,idx_11));
    end;

    idx_3 = find(cube(3,:) == 1);
    if(~isempty(idx_3))
        cube(3,idx_3) = label(idx_3);
    end;

    idx_6 = find(cube(6,:) == 1);
    if(~isempty(idx_6))
        cube(6,idx_6) = label(idx_6);
        cube(:,idx_6) = p_oct_label(4,label(idx_6),cube(:,idx_6));
    end;
    
    idx_12 = find(cube(12,:) == 1);
    if(~isempty(idx_12))
        cube(12,idx_12) = label(idx_12);
        cube(:,idx_12) = p_oct_label(6,label(idx_12),cube(:,idx_12));
    end;

    idx_14 = find(cube(14,:) == 1);
    if(~isempty(idx_14))
        cube(14,idx_14) = label(idx_14);
        cube(:,idx_14) = p_oct_label(4,label(idx_14),cube(:,idx_14));
        cube(:,idx_14) = p_oct_label(6,label(idx_14),cube(:,idx_14));
        cube(:,idx_14) = p_oct_label(8,label(idx_14),cube(:,idx_14));
    end;

end;

if( octant==3 )
    
    idx_4 = find(cube(4,:) == 1);
    if(~isempty(idx_4))
        cube(4,idx_4) = label(idx_4);
        cube(:,idx_4) = p_oct_label(1,label(idx_4),cube(:,idx_4));
    end;

    idx_5 = find(cube(5,:) == 1);
    if(~isempty(idx_5))
        cube(5,idx_5) = label(idx_5);
        cube(:,idx_5) = p_oct_label(1,label(idx_5),cube(:,idx_5));
        cube(:,idx_5) = p_oct_label(2,label(idx_5),cube(:,idx_5));
        cube(:,idx_5) = p_oct_label(4,label(idx_5),cube(:,idx_5));
    end;

    idx_13 = find(cube(13,:) == 1);
    if(~isempty(idx_13))
        cube(13,idx_13) = label(idx_13);
        cube(:,idx_13) = p_oct_label(1,label(idx_13),cube(:,idx_13));
        cube(:,idx_13) = p_oct_label(5,label(idx_13),cube(:,idx_13));
        cube(:,idx_13) = p_oct_label(7,label(idx_13),cube(:,idx_13));
    end;

    idx_7 = find(cube(7,:) == 1);
    if(~isempty(idx_7))
        cube(7,idx_7) = label(idx_7);
    end;

    idx_8 = find(cube(8,:) == 1);
    if(~isempty(idx_8))
        cube(8,idx_8) = label(idx_8);
        cube(:,idx_8) = p_oct_label(4,label(idx_8),cube(:,idx_8));
    end;
    
    idx_15 = find(cube(15,:) == 1);
    if(~isempty(idx_15))
        cube(15,idx_15) = label(idx_15);
        cube(:,idx_15) = p_oct_label(7,label(idx_15),cube(:,idx_15));
    end;

    idx_16 = find(cube(16,:) == 1);
    if(~isempty(idx_13))
        cube(16,idx_16) = label(idx_16);
        cube(:,idx_16) = p_oct_label(4,label(idx_16),cube(:,idx_16));
        cube(:,idx_16) = p_oct_label(7,label(idx_16),cube(:,idx_16));
        cube(:,idx_16) = p_oct_label(8,label(idx_16),cube(:,idx_16));
    end;
    
end;

if( octant==4 )
    
    idx_5 = find(cube(5,:) == 1);
    if(~isempty(idx_5))
        cube(5,idx_5) = label(idx_5);
        cube(:,idx_5) = p_oct_label(1,label(idx_5),cube(:,idx_5));
        cube(:,idx_5) = p_oct_label(2,label(idx_5),cube(:,idx_5));
        cube(:,idx_5) = p_oct_label(3,label(idx_5),cube(:,idx_5));
    end;

    idx_6 = find(cube(6,:) == 1);
    if(~isempty(idx_6))
        cube(6,idx_6) = label(idx_6);
        cube(:,idx_6) = p_oct_label(2,label(idx_6),cube(:,idx_6));
    end;

    idx_14 = find(cube(14,:) == 1);
    if(~isempty(idx_14))
        cube(14,idx_14) = label(idx_14);
        cube(:,idx_14) = p_oct_label(2,label(idx_14),cube(:,idx_14));
        cube(:,idx_14) = p_oct_label(6,label(idx_14),cube(:,idx_14));
        cube(:,idx_14) = p_oct_label(8,label(idx_14),cube(:,idx_14));
    end;
    
    idx_8 = find(cube(8,:) == 1);
    if(~isempty(idx_8))
        cube(8,idx_8) = label(idx_8);
        cube(:,idx_8) = p_oct_label(3,label(idx_8),cube(:,idx_8));
    end;

    idx_16 = find(cube(16,:) == 1);
    if(~isempty(idx_16))
        cube(16,idx_16) = label(idx_16);
        cube(:,idx_16) = p_oct_label(3,label(idx_16),cube(:,idx_16));
        cube(:,idx_16) = p_oct_label(7,label(idx_16),cube(:,idx_16));
        cube(:,idx_16) = p_oct_label(8,label(idx_16),cube(:,idx_16));
    end;

    idx_9 = find(cube(9,:) == 1);
    if(~isempty(idx_9))
        cube(9,idx_9) = label(idx_9);
    end;

    idx_17 = find(cube(17,:) == 1);
    if(~isempty(idx_17))
        cube(17,idx_17) = label(idx_17);
        cube(:,idx_17) = p_oct_label(8,label(idx_17),cube(:,idx_17));
    end;

end;

if( octant==5 )
    
    idx_10 = find(cube(10,:) == 1);
    if(~isempty(idx_10))
        cube(10,idx_10) = label(idx_10);
        cube(:,idx_10) = p_oct_label(1,label(idx_10),cube(:,idx_10));
    end;

    idx_11 = find(cube(11,:) == 1);
    if(~isempty(idx_11))
        cube(11,idx_11) = label(idx_11);
        cube(:,idx_11) = p_oct_label(1,label(idx_11),cube(:,idx_11));
        cube(:,idx_11) = p_oct_label(2,label(idx_11),cube(:,idx_11));
        cube(:,idx_11) = p_oct_label(6,label(idx_11),cube(:,idx_11));
    end;
    
    idx_13 = find(cube(13,:) == 1);
    if(~isempty(idx_13))
        cube(13,idx_13) = label(idx_13);
        cube(:,idx_13) = p_oct_label(1,label(idx_13),cube(:,idx_13));
        cube(:,idx_13) = p_oct_label(3,label(idx_13),cube(:,idx_13));
        cube(:,idx_13) = p_oct_label(7,label(idx_13),cube(:,idx_13));
    end;

    idx_18 = find(cube(18,:) == 1);
    if(~isempty(idx_18))
        cube(18,idx_18) = label(idx_18);
    end;

    idx_19 = find(cube(19,:) == 1);
    if(~isempty(idx_19))
        cube(19,idx_19) = label(idx_19);
        cube(:,idx_19) = p_oct_label(6,label(idx_19),cube(:,idx_19));
    end;

    idx_21 = find(cube(21,:) == 1);
    if(~isempty(idx_21))
        cube(21,idx_21) = label(idx_21);
        cube(:,idx_21) = p_oct_label(7,label(idx_21),cube(:,idx_21));
    end;

    idx_22 = find(cube(22,:) == 1);
    if(~isempty(idx_22))
        cube(22,idx_22) = label(idx_22);
        cube(:,idx_22) = p_oct_label(6,label(idx_22),cube(:,idx_22));
        cube(:,idx_22) = p_oct_label(7,label(idx_22),cube(:,idx_22));
        cube(:,idx_22) = p_oct_label(8,label(idx_22),cube(:,idx_22));
    end;

end;

if( octant==6 )
    
    idx_11 = find(cube(11,:) == 1);
    if(~isempty(idx_11))
        cube(11,idx_11) = label(idx_11);
        cube(:,idx_11) = p_oct_label(1,label(idx_11),cube(:,idx_11));
        cube(:,idx_11) = p_oct_label(2,label(idx_11),cube(:,idx_11));
        cube(:,idx_11) = p_oct_label(5,label(idx_11),cube(:,idx_11));
    end;

    idx_12 = find(cube(12,:) == 1);
    if(~isempty(idx_12))
        cube(12,idx_12) = label(idx_12);
        cube(:,idx_12) = p_oct_label(2,label(idx_12),cube(:,idx_12));
    end;

    idx_14 = find(cube(14,:) == 1);
    if(~isempty(idx_14))
        cube(14,idx_14) = label(idx_14);
        cube(:,idx_14) = p_oct_label(2,label(idx_14),cube(:,idx_14));
        cube(:,idx_14) = p_oct_label(4,label(idx_14),cube(:,idx_14));
        cube(:,idx_14) = p_oct_label(8,label(idx_14),cube(:,idx_14));
    end;
    
    idx_19 = find(cube(19,:) == 1);
    if(~isempty(idx_19))
        cube(19,idx_19) = label(idx_19);
        cube(:,idx_19) = p_oct_label(5,label(idx_19),cube(:,idx_19));
    end;


    idx_22 = find(cube(22,:) == 1);
    if(~isempty(idx_22))
        cube(22,idx_22) = label(idx_22);
        cube(:,idx_22) = p_oct_label(5,label(idx_22),cube(:,idx_22));
        cube(:,idx_22) = p_oct_label(7,label(idx_22),cube(:,idx_22));
        cube(:,idx_22) = p_oct_label(8,label(idx_22),cube(:,idx_22));
    end;
    
    idx_20 = find(cube(20,:) == 1);
    if(~isempty(idx_20))
        cube(20,idx_20) = label(idx_20);
    end;

    idx_23 = find(cube(23,:) == 1);
    if(~isempty(idx_23))
        cube(23,idx_23) = label(idx_23);
        cube(:,idx_23) = p_oct_label(8,label(idx_23),cube(:,idx_23));
    end;
 
end;

if( octant==7 )
    
    idx_13 = find(cube(13,:) == 1);
    if(~isempty(idx_13))
        cube(13,idx_13) = label(idx_13);
        cube(:,idx_13) = p_oct_label(1,label(idx_13),cube(:,idx_13));
        cube(:,idx_13) = p_oct_label(3,label(idx_13),cube(:,idx_13));
        cube(:,idx_13) = p_oct_label(5,label(idx_13),cube(:,idx_13));
    end;

    idx_15 = find(cube(15,:) == 1);
    if(~isempty(idx_15))
        cube(15,idx_15) = label(idx_15);
        cube(:,idx_15) = p_oct_label(3,label(idx_15),cube(:,idx_15));
    end;

    idx_16 = find(cube(16,:) == 1);
    if(~isempty(idx_16))
        cube(16,idx_16) = label(idx_16);
        cube(:,idx_16) = p_oct_label(3,label(idx_16),cube(:,idx_16));
        cube(:,idx_16) = p_oct_label(4,label(idx_16),cube(:,idx_16));
        cube(:,idx_16) = p_oct_label(8,label(idx_16),cube(:,idx_16));
    end;

    idx_21 = find(cube(21,:) == 1);
    if(~isempty(idx_21))
        cube(21,idx_21) = label(idx_21);
        cube(:,idx_21) = p_oct_label(5,label(idx_21),cube(:,idx_21));
    end;

    idx_22 = find(cube(22,:) == 1);
    if(~isempty(idx_22))
        cube(22,idx_22) = label(idx_22);
        cube(:,idx_22) = p_oct_label(5,label(idx_22),cube(:,idx_22));
        cube(:,idx_22) = p_oct_label(6,label(idx_22),cube(:,idx_22));
        cube(:,idx_22) = p_oct_label(8,label(idx_22),cube(:,idx_22));
    end;

    idx_24 = find(cube(24,:) == 1);
    if(~isempty(idx_24))
        cube(24,idx_24) = label(idx_24);
    end;
    
    idx_25 = find(cube(25,:) == 1);
    if(~isempty(idx_25))
        cube(25,idx_25) = label(idx_25);
        cube(:,idx_25) = p_oct_label(8,label(idx_25),cube(:,idx_25));
    end;
end;

if( octant==8 )
    
    idx_14 = find(cube(14,:) == 1);
    if(~isempty(idx_14))
        cube(14,idx_14) = label(idx_14);
        cube(:,idx_14) = p_oct_label(2,label(idx_14),cube(:,idx_14));
        cube(:,idx_14) = p_oct_label(4,label(idx_14),cube(:,idx_14));
        cube(:,idx_14) = p_oct_label(6,label(idx_14),cube(:,idx_14));
    end;

    idx_16 = find(cube(16,:) == 1);
    if(~isempty(idx_16))
        cube(16,idx_16) = label(idx_16);
        cube(:,idx_16) = p_oct_label(3,label(idx_16),cube(:,idx_16));
        cube(:,idx_16) = p_oct_label(4,label(idx_16),cube(:,idx_16));
        cube(:,idx_16) = p_oct_label(7,label(idx_16),cube(:,idx_16));
    end;
    
    idx_17 = find(cube(17,:) == 1);
    if(~isempty(idx_17))
        cube(17,idx_17) = label(idx_17);
        cube(:,idx_17) = p_oct_label(4,label(idx_17),cube(:,idx_17));
    end;
    
    idx_22 = find(cube(22,:) == 1);
    if(~isempty(idx_22))
        cube(22,idx_22) = label(idx_22);
        cube(:,idx_22) = p_oct_label(5,label(idx_22),cube(:,idx_22));
        cube(:,idx_22) = p_oct_label(6,label(idx_22),cube(:,idx_22));
        cube(:,idx_22) = p_oct_label(7,label(idx_22),cube(:,idx_22));
    end;
    
    idx_17 = find(cube(17,:) == 1);
    if(~isempty(idx_17))
        cube(17,idx_17) = label(idx_17);
        cube(:,idx_17) = p_oct_label(4,label(idx_17),cube(:,idx_17));
    end;
    
    idx_23 = find(cube(23,:) == 1);
    if(~isempty(idx_23))
        cube(23,idx_23) = label(idx_23);
        cube(:,idx_23) = p_oct_label(6,label(idx_23),cube(:,idx_23));
    end;
    
    idx_25 = find(cube(25,:) == 1);
    if(~isempty(idx_25))
        cube(25,idx_25) = label(idx_25);
        cube(:,idx_25) = p_oct_label(7,label(idx_25),cube(:,idx_25));
    end;
    
    idx_26 = find(cube(26,:) == 1);
    if(~isempty(idx_26))
        cube(26,idx_26) = label(idx_26);
    end;
end;
