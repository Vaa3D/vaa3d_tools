function somalist2apo(filename,outfilename)
%designed by shengdian 20181015
%

%filename='/home/penglab/Desktop/17781_Soma.xlsx';
%outfilename='/home/penglab/Desktop/17781_Soma.apo';
data=readtable(filename);
[length,~]=size(data);
m_struct = [];

somalist=data(:,2);
for i=1:length
    somainfo=somalist.Z_X_Y(i);
    soma=strsplit(somainfo{1},'-');
    [~,somasize]=size(soma);
    for j=1:somasize
        soma_z1=str2num(soma{1});
        soma_z2=str2num(soma{2});
        soma_z=floor((soma_z2+soma_z1)/2);
        soma_x1=(strsplit(soma{3},'X'));
        [~,soma_x1_n]=size(soma_x1);
        soma_x1_temp=soma_x1(soma_x1_n);
        soma_x=str2num(soma_x1_temp{1});
        soma_y1=(strsplit(soma{4},'Y'));
        [~,soma_y1_n]=size(soma_y1);
        soma_y1_temp=soma_y1(soma_y1_n);
        soma_y=str2num(soma_y1_temp{1});        
    end
    
  S.n = i;
  S.orderinfo = '';
  S.name = '';
  S.comment = '';
  S.z = soma_z;
  S.x = soma_x;
  S.y = soma_y;
  S.pixmax = 0;
  S.intensity = 0;
  S.sdev = 0;
  S.volsize = 0;
  S.mass = 0;
  rr = rand(3,1);
  rr = rr./sqrt(sum(rr.*rr)).*255;
  S.color.r = round(rr(1));
  S.color.g = round(rr(2));
  S.color.b = round(rr(3));  
  m_struct{i}=S;
end
save_v3d_apo_file(m_struct, outfilename);

end