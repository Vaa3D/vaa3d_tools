Collaborative annotation between 2 users (through an additional server)

Setup and run:
1. On the server machine, run "vrserver.exe" to start the server. (The server can share the same PC with one of the clients.)
2. On both client machines, open the Vaa3D VS project. If "#define __VR_SERVER_FUNCS__" in "v3dr_gl_vr.cpp" is commented, uncomment it, and build vaa3d.
3. On both client machines, Make sure the IP address of the server is correctly configured in "v3d_external\v3d_main\v3d\release\serverIPaddress.txt". 
4. Start both clients, load a same neuron SWC file, and enter VR mode.
5. Both clients will automatically connect to the server, and synchronize with each other once in a while.
6. The local annotations will be shown in red, while remote ones in black.
7. Once either client quits VR, the local and remote annotations will be merged into a single one and saved as "MergedSWCfile.swc".

P.S.: The collaborative version of Vaa3D VR can also be used in standalone mode, without having to comment "#define __VR_SERVER_FUNCS__" and build again.
