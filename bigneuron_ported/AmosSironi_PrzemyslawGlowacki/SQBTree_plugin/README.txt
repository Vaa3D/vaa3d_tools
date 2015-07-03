# This plugin implements the regression tubularity measure described in [1] for 3-Dimensional image stacks.
# Features are computed using predefined convolutional filters that are provided with the plugin.
# Multiscale prediction is not yet supported.

# Training a model from terminal:
./vaa3d -x RegressionTubularityAC -f train
     -i <Train_Img1.tif> <Train_Img2.tif> ... <Train_ImgN.tif>
     -o <Output_Results_Dir>
     -p <GroundTruth_Img1.swc> <GroundTruth_Img2.swc> ... <GroundTruth_imgN.swc>
        <N_Autocontext_Iters> <N_Train_Samples> <N_boosting_Iters> <Tree_Depth> <Shrinkage_factor> <m_try>

Input: Supported formats: Tiff and vaa3d raw UINT8 3d volumes
Output: The trained models will be saved in <Output_Results_Dir> under the names Regressor_ac_0.cfg, Regressor_ac_1.cfg, ..., Regressor_ac_<N_Autocontext_Iters>.cfg
Parameters: <GroundTruth_Img1.swc> <GroundTruth_Img2.swc> ... <GroundTruth_imgN.swc> swc files contining ground truth. Must be same number of input images.
											 N.B. It Assumes swc files are in image coordinate !
            <N_Autocontext_Iters> number of Auto-context iterations (set to 0 in order not to use Auto-context). Default 1.
            <N_Train_Samples> number of training samples. Default 100000.
            <N_boosting_Iters> number of boosting iterations. Default 200.
            <Tree_Depth> max depth of trees used as weak learners. Default 2.
            <Shrinkage_factor> shrinkage factor. Defualt 0.1.
            <m_try> number of features randomly sampled to lean a weak learner. Default 0 ( = use all features).


# Tesing a model from terminal:
./vaa3d -x RegressionTubularityAC -f test
     -i <Test_Img1.tif> <Test_Img2.tif> ... <Test_ImgN.tif>
     -o  <Tubularity_img1.raw> <Tubularity_Img2.raw> ... <Tubularity_ImgN.v3draw>
     -p <Path_to_Regressor_ac_0.cfg> <Path_to_Regressor_ac_1.cfg> ... <Path_to_Regressor_ac_M.cfg>

Input: Supported formats: Tiff and vaa3d raw UINT8 3d volumes
Output: format: vaa3d raw FLOAT32 3d volumes
Parameters: <Path_to_Regressor_ac_0.cfg> <Path_to_Regressor_ac_1.cfg> ... <Path_to_Regressor_ac_M.cfg> correspond to M+1 trained models, one for each Auto-context iteration.
    When M>0, results for the different Auto-context iterations will be saved in the /temp_results directory



[1] A. Sironi, E. Türetken, V. Lepetit and P. Fua. Multiscale Centerline Detection, submitted to IEEE Transactions on Pattern Analysis and Machine Intelligence.
