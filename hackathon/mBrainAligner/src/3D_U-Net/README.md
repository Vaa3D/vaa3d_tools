
3D U-Net (https://arxiv.org/abs/1606.06650) is adopted to buid a segmentation network and generate segmentations for several critical brain regions, which can be used in later steps of local registration.

# Dependencies
To run the training and inference scripts, several dependencies are required to be installed.

The user can install the dependencies directly by running:

```shell
  pip install -r requirements.txt
```
To facilitate the installation process, we have also prepared a docker image which contains all the required packages. If the docker environment is available on the workstation, the docker image can be accessed by running:

```shell
  docker pull oyl98/3d_u-net:v0
```

# Preparing the training data
The training data needs to be prepared in .h5 format, which can be done by running the following script:
```shell
  python ./DataPreprocess/ToH5.py 
```

Before running the above script, the original images and the label files should be put at `./data/orig/img` and `./data/orig/label`, respectively. These files should be provided in the `v3draw` format.

Two result files in the name of `train.h5` and `val.h5` will be generated.

# Training

Once the h5 files are generated, the user can start training the 3D U-Net by running:

```shell
  python train_unet.py
```

Training the 3D U-Net with the default parameters would require a GPU with more than 12GB of RAM.
Paramters such as the model depth and the input size can be modified in `train_unet.py`.

The trained model will be stored at `./logs/U_model.h5`. To simplify the work, we have provided a pre-trained model in the same directory.

# Inference

The images for segmentation need to be placed at `./data/test`. The following script is used to segment the images:

```shell
  python predict_unet.py
```

The results will be stored at `./data/predict/[filename]`. For each input image, several result files are generated, including `seg.v3draw`, `0.v3draw`, `1.v3draw`, ..., `8.v3draw`.

Optionally, the resulting `v3draw` files can be visualized using [Vaa3D](https://vaa3d.org).


