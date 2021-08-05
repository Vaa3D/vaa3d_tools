# imPreProcess Single-Neuron Image Enhancement (imPreProcess)

imPreProcess is a Vaa3D plugin including a pipeline of image enhancement for single-neuron images. The aims is to enhance the neurite signal as well as to suppress the background, with the ultimate goal of high signal-background contrast and better within- and between image homogeneity. 

Please cite the following manuscript for CMO analysis:

>  Guo et al. Image enhancement to leverage the 3D morphological reconstruction of single-cell neurons. Bioinformatics.

In this repo, we provide the following sources.

  * The source code for Vaa3D (vaa3d.org) plugin for image enhancement of single-neuron images.
  * Folder 'supporting_func': python code to benchmark image quality from the perspectives of signal-background contrast, dynamic range of  background/signal, for both within- and between-images.
  * Folder 'test_data': example image blocks from fMost image dataset of mouse brains. 


## Outline

1. [Installation](#Installation)
2. [Run via Command-Line Interface](#Command)
3. [Command-line parameters](#Parameter)
4. [FAQ](#FAQ)
5. [License](#License)


## <a name="Installation"></a>Installation

 -Please follow the build instructions here to build released plugins along with the Vaa3d main program:
         https://github.com/Vaa3D/Vaa3D_Wiki/wiki/BuildVaa3d.wiki
 -Build this Vaa3D plugin following this wiki page: https://github.com/Vaa3D/Vaa3D_Wiki/wiki/CompilePlugins.wiki 
  
## <a name="Command"></a>Run via Command-Line Interface
  The plugin takes one 3d image and performs the enhancement according to the parameters specified by the user. 
  It can be applied via either the GUI of the Vaa3D platform under the menu of 'plug-in/imPreProcess/image enhancement', or via the command line interface as following:
  
   Window-based: 
        vaa3d_msvc.exe /x path_of_plugin_dll /f im_enhancement /i imput_image /o output_image /p parameter_list 
   Linux/Mac-based: 
        vaa3d -x path_of_plugin_so -f im_enhancement -i imput_image -o output_image -p parameter_list

   input_image: 3d image to be enhanced (supported formats: .tif, .raw, .v3draw)
   output_image: where to save the enhanced image (supported formats: .tif, .raw, .v3draw)
   parameter_list: value of differet parameters, separated by space


## <a name="Parameter"></a>Command-line parameters

| Flag          | Usage                                                        | Default  |
| :------------ | ------------------------------------------------------------ | -------- |
| --wxy_bilateral    | window size of bilateral filter along the x- and y-axis | 3 |
| --wz_bilateral         | window size of bilateral filter along the z-axis  | 1 |
| --sigma_color_bilateral | color sigma of bilateral filter | 35 |
| --gain_sigmoid_correction  | intensity gain for the sigmoid intensity adjustment | 5|
| --gain_sigmoid_correction  | image intensity in percentile, used as cutoff intensity of the sigmoid intensity adjustment | 25|
| --b_do_bf  | whether or not conduct bilateral filter during the enhancement, 1 yes, 0 no | 1 |
| --b_do_fft  | whether or not conduct high-pass filter during the enhancement, 1 yes, 0 no | 1 |


## <a name="FAQ"></a>FAQ

    If you have questions, please check the documentations on the Vaa3D website: http://vaa3d.org, and the wiki pages: https://github.com/Vaa3D/Vaa3D_Wiki/wiki/.
    You could find answers of FAQs and submmit your questions using the following help forum:
        https://www.nitrc.org/forum/forum.php?forum_id=1553.


##  <a name="License"></a>License
    
    Vaa3D and various Vaa3D tools (including plugins etc) used a slightly modified revised MIT license unless specifically stated for some rare cases, see detail here:
        https://github.com/Vaa3D/Vaa3D_Wiki/wiki/LicenseAndAgreement.wiki .

Maintainer: [Shuxia Guo](shuxia_guo@seu.edu.cn)

[BrainTell](https://braintell.org)

Copyright (c) 2021-present, Shuxia Guo (shuxia_guo@seu.edu.cn)
