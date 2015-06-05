# README
#### Apache License, version 2.0
## ocp\_2\_vaa3d Description

This plugin was developed as part of the BigNeuron hackathon (June 1-5, 2015).  The intent of this plugin is to create a prototype for connecting Vaa3D to data that exists in the cloud.  The Open Connectome Project was selected as the data source for this prototype plugin because:

- It contains a large amount of neural data
- All data is freely available for download
- A clean API has been developed for accessing data
- Data has been collected using multiple methods (EM, Array Tomography, etc.)

The ocp\_2\_vaa3d plugin has been only moderately tested and should serve as a preliminary attempt at linking Vaa3D to a cloud-based datasource. 

_NOTE: This code was developed and tested on Mac OSX exclusively.  No claims are made to how the plugin will work on Linux or Windows._

## Dependencies
- HDF5 (currently committed as part of the ocp\_2\_vaa3d plugin) 
  - libhdf5
  - libhdf5\_cpp
  - include/
- Boost
  - libboost\_filesystem
  - libboost\_system
  - boost/filesystem.hpp
  - boost/asio.hpp
  - boost/algorithm/string.hpp

## Instructions
### Vaa3D GUI
1. Open Vaa3D GUI
2. Select `OCP_2_Vaa3D` plugin
3. A dropdown list of project tokens will be presented -> Select the desired dataset
4. Dataset information will be presented in a new window:
  - OCP data is saved at multiple resolution levels, select the desired level
    - *Note* - Resolution is listed as *<resolution\_level>,<x\_end>,<y\_end>,<z\_start>,<z\_end>*
  - If the dataset contains multiple channels, select the desired channels
    - *Note* - Only 3 channels may selected since they will be rendered as RGB.  If multiple channels are selected, only the first three channels checked (in the original list order) will be visualized
  - Select the desired volume indices
    - All indices should be written as *<start>,<stop>*
    - Some error checking is implemented to verify valid extents are entered, but the checks are not comprehensive.  Any errors encountered will result in the plugin closing. 
   
### Vaa3D Commanline
1. Navigate to the Vaa3D binary directory
2. Run as follows:

```
./vaa3d64 -x ocp_2_vaa3d -f Import_data -i ocp -o <output_filepath> -p <project_token> <resolution_level> <x_start>,<x_stop> <y_start>,<y_stop> <z_start>,<z_stop> [<channel1_str> <channel2_str> ...]
```

_NOTE_: Be careful about the use of commas and spaces.  Improper use may result in the plugin crashing.

*Example*:

```
./vaa3d64 -x ocp_2_vaa3d -f Import_data -i ocp -o /home/user/output.raw -p Ex10R55 1 1000,1500 1000,1500 0,50 Arc-5 PSD95-1 Synapsin1-2
```

## Capabilities and Limitations
### Capabilities
- Retrieve arbitrary volumes from OCP datasets
- Load single- or multi-channel volumes into the Vaa3D interactive environment
- Download volume(s) from OCP and save to Vaa3D data format (via the commandline interface)

### Plugin Limitations
In current form, this plugin _CANNOT_ due the following.

- Interface with non-OCP servers  
- Load time series data 
- Load more than 3 channels

## Links
[http://openconnecto.me](http://openconnecto.me) 

## Primary developers

- Nathan Drenkow (JHU/APL)
- Dean Kleissas (JHU/APL)

#### Original date created: 2015-06-05
