# SDGP algorithm
Semantic deconvolution based on Gaussian probability (SDGP) model is used for Nuclear detection with 3D U-Net.
# Outline
1. [Installation](#Installation)
2. [Nucleus Detection](#NucleusDetection)
3. [License](#Licence)

# Installation
- Open "Anaconda Prompt (anaconda3)";
- In the terminal change to the "SDGP.yaml" file folder.
	
	`$ cd <your SDGP.yaml folder path>`
- Create a SDGP conda enviroment using the SDGP environmrnt file "SDGP.yaml".
	
	`$ conda env create -f SDGP.yaml`
- Create a new python project using the SDGP environmrnt.
- Copy the nucleus detection codeto the python project.

#Nucleus Detection
- **trainingModel.py**: Train the ".h5 "model by entering the labeled images and labeled points folder as a training set.
- **useModel.py**: Input ".h5 "model and image folder to be run as test set to get semantic image.
- **gwdtFunction.searchLocalMax**: Obtain the local maximum value of semantic image.
- **evaluation.watershed**: Taking the local maximum point of semantic image as the seed point, the rough segmentation of watershed algorithm is carried out.
- **evaluation.mylabel2rgb**: Use different colors to represent the segmentation results of each nucleus.
- **evaluation.seg_by_density**: The density peak is used to finely segment the image.
- **evaluation.py**: Recall, Precision and F1 were used to evaluate the detection results.