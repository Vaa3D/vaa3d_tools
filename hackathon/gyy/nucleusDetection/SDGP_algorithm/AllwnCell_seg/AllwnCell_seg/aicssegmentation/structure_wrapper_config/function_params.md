# Glossary
## Description of parameters in each function widget

***Note: the parameters described here are only for the widgets in the Napari plugin. Not all parameters are exposed to the widget for the sake of simplicity. For programmer users, please refer the full documentation to see the detailed APIs.***  

### 1. Intensity Normalization

Auto-contrast normalizaiton. First, *mean* and standard deviaion (*std*) of the original intensity in image are calculated. Next, the intensity is truncated into range `[mean - a * std, mean + b * std]`, and then rescaled to `[0, 1]`. `a` and `b` are parameters controling effect of the adjustment. 
* `scaling_param`: a list of two float values, corresponding to `a` and `b` is the aforementioned equation.


### 2. Intensity Normalization with bound

Auto-contrast normalization. Similat to *Intensity Normalization* above, but with two extra parameters: one upper bound and one lower bound. The intensity of the image will be first clipped into the range defined by the upper bound and the lower bound, and then do the normalization as in *Intensity Normalization*. 
* `scaling_param`: a list of four values. The first two are float values, as in *Intensity Normalization*. The last two are integer values corresponding to the lower bound and upper bound respectively.


### 3. Intensity Normalization using min-max with bound

Min-Max normalization, but the intensity will be clipped by an upper bound first. Namely, any original intensity value higher than an upper bound will be considered as outlier and reset using min intensity of the image. After the clipping, the max intensity will be mapped to 1 and min intensity will be mapped to 0.
* `scaling_param`: a list of one integer value, corresponding to the upper bound value.


### 4. Edge Preserving Smoothing

A smoothing method that reduce the noise, while retaining the sharp edges. 
* No parameter is needed.


### 5. Gaussian Smoothing 3D

A smoothing method based on 3D Gaussian filter. 
* `sigma`: the size of the Gaussian kernal. Larger kernel will result in more smoothing effect.


### 6. Gaussian Smoothing Slice by Slice

A smoothing method based on 2D Gaussian filter and applied on 3D images slice by slice.
* `sigma`: the size of the Gaussian kernal. Larger kernel will result in more smoothing effect.


### 7. Filament Filter 3D

Apply the 3D filament filter on a 3D image.
* `sigmas` indicates the "scale" of your objects, and is usually set based on the estimated thickness of your target filaments. For example, if visually the thickness of the filaments is usually 3~4 pixels, then you may want to set `sigmas` as 1 or something near 1 (like 1.25). Multiple scales can be used by applying this filter multiple times with different `sigmas`, if you have filaments of very different thickness.
* `cutoff` is a threshold applied on the actual filter reponse to get the binary result. Smaller `cutoff` may yield more filaments, especially detecting more dim ones and gets thicker segmentation, while larger `cutoff` could be less permisive and yield less filaments and thinner segmentation.


### 8. Filament Filter Slice by Slice

Apply the 2D filament filter on a 3D image slice by slice.
* `sigmas` indicates the "scale" of your objects, and is usually set based on the estimated thickness of your target filaments. For example, if visually the thickness of the filaments is usually 3~4 pixels, then you may want to set `sigmas` as 1 or something near 1 (like 1.25). Multiple scales can be used by applying this filter multiple times with different `sigmas`, if you have filaments of very different thickness.
* `cutoff` is a threshold applied on the actual filter reponse to get the binary result. Smaller `cutoff` may yield more filaments, especially detecting more dim ones and gets thicker segmentation, while larger `cutoff` could be less permisive and yield less filaments and thinner segmentation.


### 9. Spot Filter 3D

Apply the 3D spot filter on a 3D image.
* `log_sigma` indicates the "scale" of your objects, and is usually set based on the estimated radius of your target dots. For example, if visually the diameter of the dots is about 3~4 pixels, then you may want to set `log_sigma` as 1 or something near 1 (like 1.25). Multiple scales can be used by applying this filter multiple times with different `log_sigma`, if you have dots of very different sizes.
* `cutoff` is a threshold applied on the actual filter reponse to get the binary result. Smaller `cutoff` may yield more dots and "fatter" segmentation, while larger `cutoff` could be less permisive and yield less dots and slimmer segmentation.


### 10. Spot Filter Slice by Slice

Apply the 2D spot filter on a 3D image slice by slice.
* `log_sigma` indicates the "scale" of your objects, and is usually set based on the estimated radius of your target dots. For example, if visually the diameter of the dots is about 3~4 pixels, then you may want to set `log_sigma` as 1 or something near 1 (like 1.25). Multiple scales can be used by applying this filter multiple times with different `log_sigma`, if you have dots of very different sizes.
* `cutoff` is a threshold applied on the actual filter reponse to get the binary result. Smaller `cutoff` may yield more dots and "fatter" segmentation, while larger `cutoff` could be less permisive and yield less dots and slimmer segmentation.


### 11. Generate seeding image

Build a seed image for an image of 3D objects (assuming roughly convex shapes in 3D) using the information in the center slice.

***This function takes two inputs.***
* **Input 1** is a grayscale image and **Input 2** is a binary segmentation image.
* **Input 1** is only used to determine the center slice of the objects in this image. Think about the image that has a few cells in it. Depending on the depth of the imaging, if there are 50 Z slices, the actual center Z of cells may not live right on slice 25, maybe 22, maybe 31. This function will first estimate the actual center Z slice of the cells according to the intensity profile of **Input 1** along Z. Supppose the estimated center is slice Z=21. Then, slice Z=21 from **Input 2** will be taked to search for objects. The estimation of minimal and maximal size will be needed as parameters. After extracting the object from the center slice, one seed will be placed at the centroid of each object.
* `area_min` and `area_max` are estimated minimal and maximal size on the middle slice (thinking about the center XY plane of a 3D ball) of any object.
* `bg_seed` is a boolean value. `bg_seed`=True will add a background seed at the first frame (z=0). This function usually serves as a seed detection step for marker-controled watershed. In this case, we usually need to apply **Remove Index Object** to remove the object corresponding to the background seed.
 

### 12. Find Local Maxima

***This function takes two inputs.***
* **Input 1** is a grayscale image and **Input 2** is a binary image (used as a mask).
This function will detect pixels (only with in the mask defined by **Input 2**) with local maximum intensity in **Input 1**. This function usually serves as a seed detection step for marker-controled watershed.
* No parameter is needed.


### 13. Watershed for cutting

Marker controled watershed for cutting falsely merged ball-shape objects.

***This function takes two inputs.***
* **Input 1** is a binary image of current segmentation and **Input 2** is a another binary image, where the pixels of 1 indicates seeds for watershed.
* No parameter is needed.


### 14. Watershed for segmentation

Marker controled watershed for segmenting objects from grayscale images. ***This function takes two inputs.*** **Input 1** is a grayscale image to be segmented and **Input 2** is a labeled image, where the pixels have either value 0 or an integer value (the index of the object seed).
* `watershed_line`: a boolean value indicating whether the watershed line is returned in the segmentation.


### 15. Masked Object Threshold - Part 1: low level 

Implementation of "Masked Object Thresholding" algorithm. Specifically, the algorithm is a hybrid thresholding method combining two levels of thresholds. The steps are [1] a global threshold is calculated, [2] extract each individual connected componet after applying the global threshold, [3] remove small objects, [4] within each remaining object, a local Otsu threshold is calculated and applied with an optional local threshold adjustment ratio (to make the segmentation more and less conservative). An extra check can be used in step [4], which requires the local Otsu threshold larger than 1/3 of the global Otsu threhsold and otherwise this connected component is discarded. This function implements the low level part (step 1-3) and outputs the low level segmentation results.

* `global_thresh_method`has three options: "triangle" (or "tri"), "median" (or "med"), and "ave_tri_med" (or "ave"). "ave" refers the average of "triangle" threshold and "mean" threshold.
* `object_minArea` the size filter for excluding small object in step 3.
* `dilate` is a boolean value indicating whether to perform dilation (by 2 pixels) on low level segmentation results to be conservative so that the high level step will be more robust.


### 16. Masked Object Threshold - Part 2: high level

This function implements the high level part of the "Masked Object Thresholding" algorithm.
* `extra_criteria` is a boolean value indicating whether to use the extra check described in step [4].
* `local_adjust` is a float value to be multiplied on the Otsu threshold within each local object. A value between 0 and 1 can make the segmentation more conservative, while a value larger than 1 can make the segmentation less permissive.


### 17. Merge Segmentation

Merge multiple segmentations into a single result.

***This function takes at least two inputs.***
* No parameter is needed.


### 18. Segmentation XOR

Get the XOR of multiple segmentations into a single result.

***This function takes at least two inputs.***
* No parameter is needed.


### 19. Remove Index Object

Remove object with index 1 from the segmentation. This function is usually used together when a background seed is added in the seed image for marker-controlled watershed.
* No parameter is needed.


### 20. Mask Image

***This function takes two inputs.***
* **Input 1** is an image to be masked on.
* **Input 2** is the mask, an image of the same size as **Input 1**, but each pixel is either 0 or 1 to represent the mask.

This function is commonly used to mask out some area from a specific segmentation.
* `value`: all the pixels in **Input 1** that are 1 in the mask image will be set to `value`.   


### 21. Invert Image

Assume the input is an image, this function will return an image, each pixel is 1 - original value.
* No parameter is needed.


### 22. Topology Preserving Thinning

Perform thinning on segmentation without breaking topology. For example, if there are some thin filaments or very thin connection between objects, traditional thinning operation could break the thin filaments or the thin connection, which alters the topology.
* `min_thickness` is half of the minimum width you want to keep from being thinned. For example, when the object width is smaller than 4, you don't want to make this part even thinner (may break the thin object and alter the topology), you can set `min_thickness` as 2.
* `thin` is the amount to thin (has to be a positive integer), i.e. the number of pixels to be removed from the outer boundary towards the center.


### 23. Prune Z-Slices

This function will prune the segmentation by only keep a certain range of z-slices with the assumption of all signals living only in a few consecutive z-slices. This function will first determine the key z-slice where most of the signals living on and then include a few slices up/down along z to make the segmentation completed. This is useful when you have prior knowledge about your segmentation target and can effectively exclude small segmented objects due to noise/artifacts in those z-slices we are sure the signal should not live on.
* No parameter is needed.


### 24. Hole Filling

Filling holes in the segmentation.
* `hole_min` and `hole_max` defines the size range of the holes are allowed to fill.
* `fill_2d` is a boolean value. If `fill_2d`=True, a 3D image will be filled slice by slice. If you think of a hollow tube along the z direction, the inside is not a hole under 3D topology, but the interior on each slice is indeed a hole under 2D topology.


### 25. Size Filter

Assume the input is a binary image:
* `min_size`: the size filter will remove any objects with size smaller than `min_size` (an integer value).
* `method`: The size is calculated using `method`, either `3D`(the size is calculated and applied in 3D) or `slice-by-slice` (the size is calculated and applied in a slice by slice manner).


### 26. Extract Boundary of Objects

Extract the boundary of segmented objects.
* `connectivity` has three options: 0, 1, or 2.
* `mode` can be "think", "thin", or "outer".

Find more details in https://scikit-image.org/docs/dev/api/skimage.segmentation.html#skimage.segmentation.find_boundaries
