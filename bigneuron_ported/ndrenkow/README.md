# README - ocp\_2\_vaa3d plugin
#### Apache License, version 2.0
---
## *ocp\_2\_vaa3d* Description

This plugin was developed as part of the BigNeuron hackathon (June 1-5, 2015).  The intent of this plugin is to create a prototype for connecting Vaa3D to data that exists in the cloud.  The Open Connectome Project (OCP) was selected as the data source for this prototype plugin because:

- It contains a large amount of imaging data
- All data is freely available for download
- OCP provides a cutout service for extracting spatially contiguous data volumes
- Imaging data has been collected using multiple methods (EM, Array Tomography, etc.)

The plugin operates by first requiring the user to provide dataset and volume extent information to Vaa3d. An OCP-compatible RESTful query is generated.  The query is made to the OCP server and a volume cutout is retrieved and downloaded as an HDF5 file.  Image volume data and metadata are extracted from the HDF5 file and used to create a Vaa3D Image4DSimple object.  Single channel EM data is returned as an 8-bit image whereas multi-channel data is 16-bit.  Conversion from OCP to Vaa3D data formats occurs automatically within the plugin.  

The ocp\_2\_vaa3d plugin has been only moderately tested and should serve as a preliminary attempt at linking Vaa3D to a cloud-based datasource. 

_NOTE: This code was developed and tested on Mac OSX exclusively.  No claims are made to how the plugin will work on Linux or Windows._

---
## Dependencies
- HDF5 (currently committed as part of the `ocp_2_vaa3d` plugin) 
  - libhdf5
  - libhdf5\_cpp
  - include/
- Boost
  - libboost\_filesystem
  - libboost\_system
  - boost/filesystem.hpp
  - boost/asio.hpp
  - boost/algorithm/string.hpp
---
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
    - All indices should be written as *<start_idx>,<stop_idx>*
    - Some error checking is implemented to verify valid extents are entered, but the checks are not comprehensive.  Any errors encountered will result in the plugin closing. 
   
### Vaa3D Commandline
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
---
## Capabilities and Limitations
### Capabilities
- Retrieve arbitrary volumes from OCP datasets
- Load single- or multi-channel volumes into the Vaa3D interactive environment
- Download volume(s) from OCP and save to Vaa3D data format (via the commandline interface)

### Plugin Limitations
In current form, this plugin _does NOT_ due the following:

- Interface with non-OCP servers  
- Load time series data 
- Visualize more than 3 channels
- Load annotation data

---
## Links and references
[http://openconnecto.me](http://openconnecto.me) 

[Original Open Connectome paper](http://www.ncbi.nlm.nih.gov/pmc/articles/PMC3881956/)

---
## Primary developers

- Nathan Drenkow (JHU/APL)
- Dean Kleissas (JHU/APL)

#### Original date created: 2015-06-05
