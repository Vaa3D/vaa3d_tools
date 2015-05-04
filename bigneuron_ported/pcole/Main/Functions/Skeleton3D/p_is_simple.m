function p_is_simple = p_is_simple(N)

% copy neighbors for labeling
n_p = size(N,1);
p_is_simple = ones(1,n_p);

cube = zeros(26,n_p);
cube(1:13,:)=N(:,1:13)';
cube(14:26,:)=N(:,15:27)';

label = 2*ones(1,n_p);

% for all points in the neighborhood
for i=1:26
    
    idx_1 = find(cube(i,:)==1);
    idx_2 = find(p_is_simple);
    idx = intersect(idx_1,idx_2);
    
    if(~isempty(idx))
        
        % start recursion with any octant that contains the point i
        switch( i )
            
            case {1,2,4,5,10,11,13}
                cube(:,idx) = p_oct_label(1, label, cube(:,idx) );
            case {3,6,12,14}
                cube(:,idx) = p_oct_label(2, label, cube(:,idx) );
            case {7,8,15,16}
                cube(:,idx) = p_oct_label(3, label, cube(:,idx) );
            case {9,17}
                cube(:,idx) = p_oct_label(4, label, cube(:,idx) );
            case {18,19,21,22}
                cube(:,idx) = p_oct_label(5, label, cube(:,idx) );
            case {20,23}
                cube(:,idx) = p_oct_label(6, label, cube(:,idx) );
            case {24,25}
                cube(:,idx) = p_oct_label(7, label, cube(:,idx) );
            case 26,
                cube(:,idx) = p_oct_label(8, label, cube(:,idx) );
        end;

        label(idx) = label(idx)+1;
        del_idx = find(label>=4);
        
        if(~isempty(del_idx))
            p_is_simple(del_idx) = 0;
        end;
    end;
end;


