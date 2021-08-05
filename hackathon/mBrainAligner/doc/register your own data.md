# Register your own data 

**data preparation**

If you want to register your own data, you first need to specify a target/fixed image and a subject/moving image, and generate the target affiliated files (as shown in `examples/target/`). All images need to be in `.v3draw` format.  

You can prepare the target and subject data in the following way:

- **Target data**: If you want to register your data to CCFv3, you can use the target files we provided in `examples/target/` directory. **Note that** the CCFv3 average template and its affiliated files we provided are downsampled (~50um) to faciliate the network transfer and storage. We encourage user to use CCFv3 average template (25um) as the target to obtain a good balance between accuarcy and computational time. You can contact us (17718151568@126.com) for the padded CCFv3 average template (25um) and its affiliated files, or generated the affiliated files (as shown in `examples/target/`) following the instructions described in [Image registration pipeline tutorial](https://github.com/Vaa3D/vaa3d_tools/blob/master/hackathon/mBrainAligner/doc/Image_reg_pipeline_win.md).

- **Subject data**: The subject image need to be resized to roughly match the size of target image. 

**Registration process**

The simplest way to run mBrainAligner on your own data is to modify the scripts file we provided in `examples` and run global and local registration in batch.

- (1) If you generate the `Target data` yourself, copy them to `examples/target/`.

- (2) Modify the `config.txt` to meet your requirements.

- (3) Modify the registration data path and corresponding parameters in `run_script_windows.bat` or `run_script_linux.sh`. Parameter `-m Registration_sample_data/raw_sample_data/fMOST_18458_raw.v3draw` needs to be changed to the filename of your data. Description of other parameters can be found in the script file.

- (4) double-click `run_script_windows.bat` or: 
  ```
  sh run_script_linux.sh
  ```

