
function S = load_v3dsurface(filename)
%% a function to load the vaa3d surface file (.v3ds)
% function S = load_v3dsurface(filename)
% by Hanchuan Peng
% 2015-05-7

f = fopen(filename, 'r');
if (f<0)
    return;
end;

f_num = 0;
v_num = 0;

fread( f, 128, 'uchar' );

g_num = fread( f, 1, 'uint32');%									//qDebug("		g_num= %d", g_num);
if (g_num==1)
    mesh_type = 1; %// range surface
else
    mesh_type = 0; %// label field
end;

for i=1:g_num,
    g_label0 = fread( f, 1, 'uint32');
    g_label1 = fread( f, 1, 'uint32');
    
    S(i).n = i;
    S(i).label = g_label0;
    S(i).label2 = g_label1;
    
    %read the default display option on 090223
    S(i).on = fread( f, 1, 'uchar');
    
    %added name and comment on 090220
    len =	fread( f, 1, 'uint32');
    str = fread( f, len, 'uint8=>char');
    S(i).name = str';
    
    len =	fread( f, 1, 'uint32');
    str = fread( f, len, 'uint8=>char');
    S(i).comment = str';
    
    fprintf('#%d surface object name = [%s]\n', i, (S(i).name));
    
    %now read the triangles
    
    t_num = fread( f, 1, 'uint32' );	%					//qDebug("		group(%d) t_num= %d", i, t_num);
 
    fprintf(' %d of triangles to load.\n', t_num);
    
    for iFace=1:t_num, % each face/triangle
        for iConner=1:3,
            v.x = fread( f, 1, 'float32' ); v.y = fread( f, 1, 'float32' ); v.z = fread( f, 1, 'float32' );
            vn.x = fread( f, 1, 'float32' ); vn.y = fread( f, 1, 'float32' ); vn.z = fread( f, 1, 'float32' );
            
            pT.vertex(iConner).p0 = v.x;
            pT.vertex(iConner).p1 = v.y;
            pT.vertex(iConner).p2 = v.z;
            pT.normal(iConner).p0 = vn.x;
            pT.normal(iConner).p1 = vn.y;
            pT.normal(iConner).p2 = vn.z;
            pT.next = 0;
        end;
        
        v_num = v_num + 3;
        
        S(i).listTriangle(iFace) = pT;
        % each face
        f_num = f_num + t_num;
        
    end; %}// each group
    
    fprintf('---------------------read %d objects, %d faces, %d vertices\n', length(S), f_num, v_num);
end;

fclose(f);

return;

%%===

%function g = QF_READ(f, length)
%g = fread(f,length, 'int32');
