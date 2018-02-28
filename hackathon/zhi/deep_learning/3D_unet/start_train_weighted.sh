#export LD_LIBRARY_PATH=/usr/local/cuda/lib64
LOGDIR=./training_log
CAFFE=/local1/work/caffe_unet/3D-Caffe/build/tools/caffe
SOLVER=./solver_weighted.prototxt

mkdir snapshot
mkdir $LOGDIR

GLOG_log_dir=$LOGDIR $CAFFE train -solver $SOLVER -gpu 0


