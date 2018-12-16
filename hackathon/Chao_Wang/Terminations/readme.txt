This plugin is a simplified version of 3D tree-like structure termination detection method, based on paper: "A Multiscale Ray-Shooting Model for Termination Detection of Tree-Like Structures in Biomedical Images".

Compared with the proposed algorithm, this plugin has not implemented the preprocessing steps and the local neurite diameter estimation step, because Vaa3D does not provide these modules. The preprocessing steps are replaced by a global thresholding in this plugin. Users can preprocess (denoise) the images before using this plugin. The modules that have not implemented will be implemented in the future.



