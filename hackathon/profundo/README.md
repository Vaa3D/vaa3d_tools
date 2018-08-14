# profundo

Download using `git clone https://github.com/Vaa3D/vaa3d_tools/`. Navigate to [hackathon/profundo](https://github.com/Vaa3D/vaa3d_tools/tree/master/hackathon/profundo). 

You will need to unpack the data into `/data/06_origin_cubes`. This data is not open to the public yet.


To train, run the following inside the `/agent/` directory: 

`python DQN.py --task train --algo DQN --gpu 0`

You can find your GPU number using `nvidia-smi` (default is 0). You can run on multiple GPUs with commas, e.g. `--gpu 0,1,2,3`.

If you have pre-trained models saved inside `/agent/model_checkpoints`, you can start training from a checkpoint:

`python DQN.py --task train --algo DQN --gpu 0 --load model_checkpoints/model-25000`

You can also run a pretrained model (without updating the network) using the "play" task. You can also use the `--saveVideo` flag to visualize the tracing, e.g.:

`python DQN.py --task play --saveVideo --algo DQN --gpu 0 --load  model_checkpoints/model-21658`

You can view the interactive training logs in real-time by running `tensorboard --logdir train_log/<log directory>`, e.g. `tensorboard --logdir train_log/2018-08-14_11-02-12/`. This will start a tensorboard server and print the address it is being served on. Copy paste the URL into your browser to see the logs. At first, there will only be the graph structure, but after a few thousand iterations it will start populating graphs.
