


mBrainAligner is divided into GPU-accelerated mode and non-GPU-accelerated mode.if you need to use GPU-accelerated mode, your Machines need RAM 64G,GPU 12G and CUDA 10.0. The build process of non-GPU-accelerated and GPU-accelerated mode can be referred to the following respectively


####non-GPU-accelerated mode

# Dependencies
```
QT 4.8.6
OpenCV 3.1.0
```
### OpenCV
Install OpenCV 3.1.0. 

Configure the project according to the OpenCV intallation directory.

### QT 

Install QT 4.8.6. 

Configure the project according to the QT intallation directory.

# Build mBrainAligner

#Windows

Before compiling, modify the **".pro"** file according to the path of the installed dependencies (Qt4.8.6,opencv3.1.0 etc). 

Launch the "Visual Studio x64 Win64 Command Prompt" from Start->Microsoft Visual Studio 2013->Visual Studio Tools->Visual Studio x64 Win64 Command Prompt.

Example (global_registration):

    cd (YOUR PATH)/src/global_registration/CPU
    qmake global_registration.pro
    vcvars64.bat
    nmake release
    
You can find the executable:

`(YOUR PATH)/src/global_registration/CPU/release/global_registration.exe`. 

The local registration module can be compiled in the same way.

## Linux

Before compiling, you also need to modify the **".pro"** file according to the path of the installed dependencies (QT4.8.6,OpenCV 3.1.0 etc). 

Example (global_registration):

     cd (YOUR PATH)/src/global_registration/CPU
     qmake global_registration.pro
     Make
You can find the executable file: 

`(YOUR PATH)/src/global_registration/CPU/release/global_registration.exe`

The local registration module can be compiled in the same way.


#non-GPU-accelerated mode

# Dependencies
```
QT 4.8.6
OpenCV 3.1.0
CUDA10.0
```
### OpenCV
Install OpenCV 3.1.0. 

Configure the project according to the OpenCV intallation directory.

### QT 

Install QT 4.8.6. 

Configure the project according to the QT intallation directory.

### CUDA

Install CUDA10.0.

#Windows

You can use Microsoft Visual Studio2013 to compile the demo.

Example (local_registration): 

Launch Microsoft Visual Studio2013 and navigate to the ‘../src/local_registration/GPU’ directory, and open "**.vcxproj**" to modify your project configuration based on required version of the installed dependencies. You can modify the project configuration by following these steps：

Right click on the project named main_local_registration ->  Generate dependency -> check CUDA 10.0 option 

Right click on the project named main_local_registration ->  property option ->  general option under "C/C++" -> modify your dependencies path(QT4.8.6,OpenCV3.1.0).

Last, Build your solution.

Then you can find the executable file:

`(YOUR PATH)/src/local_registration/GPU/release/local_registration.exe`

The global registration module can be compiled in the same way.


#linux

Before compiling, you also need to modify the **".pro"** file according to the path of the installed dependencies (QT4.8.6,OpenCV3.1.0). 

Example (global_registration): 

cd (YOUR PATH)/src/global_registration/GPU

qmake global_registration.pro

make

You can find the executable file: 

`(YOUR PATH)/(YOUR PATH)/src/global_registration/GPU/`

The local registration module can be compiled in the same way. 








