function [out,out1]=computePixelTimeStats(times,sr)

totalAreaInt=[];
totalAreaSamp=[];

for j=1:length(sr)
	for i = 1:times
		[AreaInt,AreaSamp]=modelPMTIntegral(2e-6,sr(j));
		totalAreaInt=[totalAreaInt AreaInt];
		totalAreaSamp=[totalAreaSamp AreaSamp];
	end
	CVInt=std(totalAreaInt)/mean(totalAreaInt);
	CVSamp=std(totalAreaSamp)/mean(totalAreaSamp);
	
	out(j,:)=[mean(totalAreaInt) std(totalAreaInt) CVInt 1/CVInt^2];
	out1(j,:)=[mean(totalAreaSamp) std(totalAreaSamp) CVSamp 1/CVSamp^2];
	totalAreaInt=[];
	totalAreaSamp=[];
end
