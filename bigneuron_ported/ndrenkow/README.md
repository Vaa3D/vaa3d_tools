# README
#### Apache License, version 2.0
## ocp\_2\_vaa3d Description

This plugin was developed as part of the BigNeuron hackathon (June 1-5, 2015).  The intent of this plugin is to create a prototype for connecting Vaa3D to data that exists in the cloud.  The Open Connectome Project was selected as the data source for this prototype plugin because:

- It contains a large amount of neural data
- All data is freely available for download
- A clean API has been developed for accessing data
- Data has been collected using multiple methods (EM, Array Tomography, etc.)

The ocp\_2\_vaa3d plugin has been only moderately tested and should serve as a preliminary attempt at linking Vaa3D to a cloud-based datasource. 

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

#### Original date created: 2015-06-04
