# how to get level0 partial data per neuron
# Yang Yu, 11/15/2018

1. resample your neuron reconstruction to ensure the nodes in swc file are distributed evenly

    For example, your input swc file is named "lgn.swc". If we put a node every 10 voxels, then we can

    vaa3d -x resample_swc -f resample_swc -i lgn.swc -o lgn_resampled.swc -p 10

2. Generate the level0 data for each resolution. (the coarsest resolution can be directly copied instead of running this plugin)

    Usage: vaa3d -x getlevel0data -f getlevel0data -i inputdir input.swc -o outputdir -p scale<0/1/2/3/4/5/...>

    For example, if your input converted data (in terafly format) looks as the following:

    mouse17302/RES(54600x34412x9847)
    mouse17302/RES(27300x17206x4923)
    mouse17302/RES(13650x8603x2461)
    mouse17302/RES(6825x4301x1230)
    mouse17302/RES(3412x2150x615)
    mouse17302/RES(1706x1075x307)
    mouse17302/RES(853x537x153)

    (1). If we generate the neuron's (lgn.swc) level 0 data into LGN folder, we can first copy the coarsest resolution to the destination folder.

        cp -r mouse17302/RES(853x537x153) LGN/

    (2). Then generate the rest level 0 data resolution by resolution

        a. vaa3d -x getlevel0data -f getlevel0data -i mouse17302/RES(1706x1075x307) lgn_resampled.swc -o LGN/RES(1706x1075x307) -p 5

        b. vaa3d -x getlevel0data -f getlevel0data -i mouse17302/RES(3412x2150x615) lgn_resampled.swc -o LGN/RES(3412x2150x615) -p 4

        c. vaa3d -x getlevel0data -f getlevel0data -i mouse17302/RES(6825x4301x1230) lgn_resampled.swc -o LGN/RES(6825x4301x1230) -p 3

        d. vaa3d -x getlevel0data -f getlevel0data -i mouse17302/RES(13650x8603x2461) lgn_resampled.swc -o LGN/RES(13650x8603x2461) -p 2

        e. vaa3d -x getlevel0data -f getlevel0data -i mouse17302/RES(27300x17206x4923) lgn_resampled.swc -o LGN/RES(27300x17206x4923) -p 1

        f. vaa3d -x getlevel0data -f getlevel0data -i mouse17302/RES(54600x34412x9847) lgn_resampled.swc -o LGN/RES(54600x34412x9847) -p 0


    *Please contact Yang Yu <gnayuy@gmail.com> if you have any questions.

