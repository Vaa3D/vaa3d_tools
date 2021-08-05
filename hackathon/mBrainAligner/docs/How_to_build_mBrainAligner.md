# Dependencies

```
Vaa3D
OpenCV 3.1.0
```

# Setting up environments

Put the `mBrainAligner` package in the `PATH\vaa3d_tools\hackathon\` directory. 

Configure the project in Release and x64 mode.

## Pre-requisites
### Vaa3D 

Please follow this link to build Vaa3D on Windows with qmake using VS2013 and Qt4.8.6:

```
https://github.com/Vaa3D/Vaa3D_Wiki/wiki/Build-Vaa3D-on-Windows-with-qmake-using-VS2013-and-Qt4.8.6
```
### OpenCV
Install OpenCV 3.1.0. 

Configure the project according to the OpenCV intallation directory.

# Build mBrainAligner
## Windows

Before compiling, modify the **".pro"** file according to the path of the installed dependencies (Vaa3D,Qt etc). 

Launch the "Visual Studio x64 Win64 Command Prompt" from Start->Microsoft Visual Studio 2013->Visual Studio Tools->Visual Studio x64 Win64 Command Prompt.

Example (global_registration):

    cd (YOUR PATH)/vaa3d_tools/hackathon/mBrainAligner/src/src_mBrainAligner/2.global_registration/
    qmake global_registration.pro
    vcvars64.bat
    nmake release
    
You can find the executable:

`(YOUR PATH)/vaa3d_tools/hackathon/mBrainAligner/src/src_mBrainAligner/2.global_registration/global_registration.exe`. 

## Linux
Before compiling, you also need to modify the **".pro"** file according to the path of the installed dependencies (Vaa3D,QT etc). 

Example (global_registration):

     cd (YOUR PATH)/vaa3d_tools/hackathon/mBrainAligner/src/src_mBrainAligner/2.global_registration/
     qmake global_registration.pro
     Make
You can find the executable file: 

`(YOUR PATH)/vaa3d_tools/hackathon/mBrainAligner/src/src_mBrainAligner/2.global_registration/global_registration`. 
