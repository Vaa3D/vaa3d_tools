
README

* Description
    -This code repository hosts source code for Vaa3D (vaa3d.org) plugin for image enhancement 
    of single-neuron images.

    -Folder 'supporting_func': python code to benchmark image quality from the perspectives of 
                                           signal-background contrast, dynamic range of  background/signal, 
                                           for both within- and between-images.
    -Folder 'test_images': example image blocks from fMost image dataset of mouse brains. 


* How to build Vaa3D plugins:
    Please follow the build instructions here to build released plugins along with the Vaa3d main program:
         https://github.com/Vaa3D/Vaa3D_Wiki/wiki/BuildVaa3d.wiki
    If you would like to build plugins in the hackathon folder or plugins for bigneuron project, please follow
    this wiki page:
         https://github.com/Vaa3D/Vaa3D_Wiki/wiki/CompilePlugins.wiki


** How to use the plugin via command line after successfully built:
   Window: vaa3d_msvc.exe /x path_of_plugin_dll /f im_enhancement /i imput_image /o output_image /p parameter_list 
   Linux/Mac: vaa3d -x path_of_plugin_so -f im_enhancement -i imput_image -o output_image -p parameter_list

   input_image: 3d image to be enhanced (supported formats: .tif, .raw, .v3draw)
   output_image: where to save the enhanced image (supported formats: .tif, .raw, .v3draw)
   parameter_list (differet parameters separated by space): wxy_bilateral  wz_bilateral  sigma_color_bilateral  gain_sigmoid_correction  cutoff_sigmoid_correction  b_do_bf  b_do_fft

   e.g.: vaa3d_msvc.exe /x imPreProcess.dll /f im_enhancement /i 17109_4142.778_1929.072_2794.948.v3draw /o 17109_4142.778_1929.072_2794.948_pre.v3draw /p 3 3 1 35 3 25 1 1


* License
    Vaa3D and various Vaa3D tools (including plugins etc) used a slightly modified revised MIT license 
    unless specifically stated for some rare cases, see detail here:
        https://github.com/Vaa3D/Vaa3D_Wiki/wiki/LicenseAndAgreement.wiki .

* Questions
    If you have questions, please check the documentations on the Vaa3D website: http://vaa3d.org , 
    and the wiki pages: https://github.com/Vaa3D/Vaa3D_Wiki/wiki/.
    You could find answers of FAQs and submmit your questions using the following help forum:
        https://www.nitrc.org/forum/forum.php?forum_id=1553.



 

