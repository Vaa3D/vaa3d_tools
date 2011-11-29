#! /bin/bash
input=$1
R --no-save --quiet << istop
data=read.table("$1", header=TRUE)
require(lattice)
pdf("$1.pdf")
print(wireframe(score ~ scale * rotate, data, drape = TRUE, aspect = c(3,1), colorkey = TRUE))
dev.off()
max_rotate=0
scales=c(12.5,25,50)
for (i in 1:length(scales)){	data2=data[data\$scale==scales[i],]; 
	#print(data2); 
	#print(data2\$rotate[which.max(data2\$score)]);
	max_rotate[i]=data2\$rotate[which.max(data2\$score)];
}
# get the maximum mean score
#len_rotate=sum(duplicated(data\$rotate)==FALSE)
#mean_scores=0
#for (i in 1:len_rotate){mean_scores[i]=mean(data\$score[data\$rotate==(5*(i-1))])}
#(which.max(mean_scores)-1)*5
max_rotate
istop 

