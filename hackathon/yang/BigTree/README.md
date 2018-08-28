# BigTree

BigTree, **B**ig **I**ma**G**e hierarchical **TREE** construction software, is developed to efficient reformat large-scale images data sets into a hierarchical tree data structure that can be visualized and annotated with Vaa3D-TeraFly [1].

More details see our paper "BigTree: high-performance hierarchical tree construction for large image data sets". (in preparation)

## build with cmake

Install [libtiff][] by following the instructions on their website. This is required for read and write images.

To get the source via the following commands:

    git clone https://github.com/gnayuy/BigTree.git
    git checkout release1.0
    
To build the source via the following commands:
    
    % mkdir build
    % cd build
    % ccmake ..
    % make

## use BigTree

Assuming your input images are 2D image slices stored in one folder in order. For example, converting to a 5-scale tree data structure, we simply type command:

    % bigtree <path_dir_input_images> <path_dir_output> 5
    
This is equavelent to using [TeraConverter][] [2] as:

    % teraconverter -s="<path_dir_input_images>" -d="<path_dir_output>" --sfmt="TIFF (series, 2D)" --dfmt="TIFF (tiled, 3D)" --rescale=4 --resolutions=01234 --width=256 --height=256 --depth=256 --halve=max --libtiff_rowsperstrip=-1
    
## image file formats

[TIFF][]: tiff file format

## references

[1]. Bria A, Iannello G, Onofri L, Peng H. TeraFly: real-time three-dimensional visualization and annotation of terabytes of multidimensional volumetric images. Nat Methods. 2016;13:192–4.

[2]. Bria A, Iannello G. TeraStitcher - a tool for fast automatic 3D-stitching of teravoxel-sized microscopy images. BMC bioinformatics. 2012;13:316. doi: 10.1186/1471-2105-13-316.

[libtiff]:http://www.libtiff.org
[TIFF]:http://www.libtiff.org/support.html
[cxxopts]:https://github.com/jarro2783/cxxopts
[TeraConverter]:https://github.com/Vaa3D/Vaa3D_Wiki/wiki/TeraConverter
