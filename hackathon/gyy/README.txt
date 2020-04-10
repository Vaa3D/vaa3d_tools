1. Chromatic_Aberration_Check plugin:
	Input: A channel folder including different Z plains;
	Output: XZ and YZ plains whose pixels is obtained by averaging the rangeNum x rangeNum region;
	Pros:
		1) Save 6 bit images;

2. Export_GrayValue plugin:
	Input: A folder including XZ and YZ plain images for 3 channel;
	Output: The gray value in 960 and 961 line for all images;
	Pros:
		1) Regex to find XZ or YZ and channel number in image name;
		2) Output two worksheet can achieve appenSheet;

3. Mean_Of_Region plugin:
	Input: A tile folder including 3 channel (Tile 12);
	Output: The gray value averaging a 20x20 region whose center is same as XY plain center for all plain;
	Pros:
		1) Filter dir folder and go through all sub folder(Note: the sub folder includes . and .. folder);
		2) Convert number to string according to ASCII and not;

4. Define_boundary plugin:
	Input: A folder including mean value files for 28 tiles that generated from Mean_of_Region plugin.
	Output: 28 files containing the boundary information for all tiles placed the current folder.
	Pros:
		1) Define point class and some member functions;
		2) Find boundary according to three continue flag_1 and specific range for three channel;

5. Calculate_Z_Shift_between_tiles plugin:
	Input: Three folder. 
		Folder1 is an image stack for a channel;
		Folder2 is another image stack for the same channel;
		Folder3 is selected to place the result.
	Output: An excel file whose content is the mean value in all regions.
