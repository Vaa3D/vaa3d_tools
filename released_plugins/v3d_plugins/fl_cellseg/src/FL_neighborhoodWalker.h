// neighborhoodwalker class
// F. Long
// 20080509

#ifndef __FL_NEIGHBORHOOD_WALKER__
#define __FL_NEIGHBORHOOD_WALKER__

#define NH_USE_ALL       0
#define NH_SKIP_TRAILING 1
#define NH_SKIP_LEADING  2
#define NH_SKIP_CENTER   4 

// NH_USE_ALL: Nwalk will go to every neighbor in a linear scan of pixels.
// NH_SKIP_TRAILING: walker will skip neighbors that follow the center pixel in a linear scan of pixels
// NH_SKIP_LEADING: walker will skip neighbors that precede the center pixel in a linear scan of pixels
// NH_SKIP_CENTER: walker will skip the center pixel itself

#include "FL_neighborhood.h"
#include "FL_defType.h"

class NeighborhoodWalker
{

public:
    V3DLONG *its_array_coords;  // its_num_neighbors-by-its_num_dims array containing relative offsets.	
    V3DLONG *its_neighbor_offsets; // an array containing linear neighbor offsets, computed with respect to a given image size.
    V3DLONG *its_image_size;
//	V3DLONG *array_sz; // size of the array along each dimension
	
    V3DLONG *its_center_coords;
	V3DLONG *its_cumprod; // Contains the cumulative product of the its_image_size array used in the sub_to_ind and ind_to_sub calculations.
	V3DLONG its_pixel_offset; //Linear index of image pixel whose neighborhood we are about to walk.
	bool *its_use; //Used to filter out certain neighbors in a neighborhood walk.
    V3DLONG its_next_neighbor_idx; // Index of the next neighbor in the walk.
    V3DLONG its_num_neighbors;
    V3DLONG its_num_dims;
	
public:

	// constructor
	NeighborhoodWalker()
	{
		its_array_coords = 0;
		its_neighbor_offsets = 0;
		its_image_size = 0;
		its_center_coords = 0;
		its_cumprod = 0;
		its_pixel_offset = 0;
		its_use = 0;
		its_next_neighbor_idx = 0;
		its_num_neighbors = 0;
		its_num_dims = 0;
	}
	
	//create NeighborhoodWalker object.
	NeighborhoodWalker(Neighborhood *nhood,  const V3DLONG input_size[], V3DLONG num_input_dims, unsigned V3DLONG flags)
	{
		V3DLONG in_num_dims;
		V3DLONG in_num_neighbors = nhood->its_num_neighbors;
		V3DLONG k;
		V3DLONG *pr_in;
		V3DLONG *pr_out;
		V3DLONG p;

		/*
		 * The dimensionality of the walker is either the image dimensionality
		 * or the neighborhood dimensionality, whichever is greater.
		 */
		in_num_dims = (num_input_dims > nhood->its_num_dims)? num_input_dims:nhood->its_num_dims;
		
		NeighborhoodWalkerInit(in_num_neighbors, in_num_dims);
		
		for (k = 0; k < its_num_neighbors; k++)
		{
			pr_in = nhood->its_array_coords + k*nhood->its_num_dims;
			pr_out = its_array_coords + k*its_num_dims;
			
			for (p = 0; p < nhood->its_num_dims; p++)
			{
				pr_out[p] = pr_in[p];
			}

			/*
			 * The next loop will only be entered if the image
			 * has higher dimensionality than the neighborhood; in this case
			 * the neighborhood offsets along those higher dimensions
			 * are all 0.
			 */
			for (p = nhood->its_num_dims; p < its_num_dims; p++)
			{
				pr_out[p] = 0;
			}
		}

		for (k = 0; k < num_input_dims; k++)
		{
			its_image_size[k] = input_size[k];
		}
		/*
		 * If the neighborhood has higher dimensionality than the
		 * input image, the image size along all those higher
		 * dimensions is 1.
		 */
		for (k = num_input_dims; k < its_num_dims; k++)
		{
			its_image_size[k] = 1;
		}

		its_cumprod[0] = 1;
		for (k = 1; k <= its_num_dims; k++)
		{
			its_cumprod[k] = its_cumprod[k-1] * 
				its_image_size[k-1];
		}
		
		for (k = 0; k < its_num_neighbors; k++)
		{
			pr_in = its_array_coords + k*its_num_dims;
			its_neighbor_offsets[k] = sub_to_ind(pr_in, its_cumprod, its_num_dims);
		}

		for (k = 0; k < its_num_neighbors; k++)
		{
			its_use[k] = true;
		}

		processWalkerFlags(flags);
		
	}


	// initialize NeighborhoodWalker object
	void NeighborhoodWalkerInit(V3DLONG in_num_neighbors, V3DLONG in_num_dims)
	{
				
		its_num_neighbors = in_num_neighbors;
		its_num_dims = in_num_dims;
		its_array_coords = new V3DLONG [its_num_neighbors*its_num_dims];
		its_neighbor_offsets = new V3DLONG [its_num_neighbors];
		its_image_size = new V3DLONG [its_num_dims];
		
//		array_sz = new (V3DLONG *) [its_num_dims];
		its_center_coords = new V3DLONG [its_num_dims];	
		its_cumprod = new V3DLONG [its_num_dims+1];
		its_use = new bool [its_num_neighbors];

		/*
		 * Set the pixel offset to be negative. An assertion in 
		 * nhGetNextInboundsNeighbor will guarantee that nhsetWalkerLocation
		 * gets called first.
		 */
		its_pixel_offset = -1;
	}



	void processWalkerFlags(unsigned V3DLONG flags)
	{
		V3DLONG k;
		V3DLONG *pr;
		bool skip_center;
		bool skip_leading;
		bool skip_trailing;

		skip_center = flags & NH_SKIP_CENTER;
		skip_leading = flags & NH_SKIP_LEADING;
		skip_trailing = flags & NH_SKIP_TRAILING;
		
		for (k = 0; k < its_num_neighbors; k++)
		{
			pr = its_array_coords + k * its_num_dims;
			if ((skip_leading && isLeadingNeighbor(pr, its_num_dims)) ||
				(skip_trailing && isTrailingNeighbor(pr, its_num_dims)) ||
				(skip_center && isNeighborhoodCenter(pr, its_num_dims)))
			{
			   its_use[k] = false;
			}
		}
	}


	// Prepare to walk the neighborhood surrounding a specified pixel.

	void setWalkerLocation(V3DLONG p)
	{
	//    mxAssert(walker != NULL, "");
	//    mxAssert(p >= 0, "");
	//
		its_pixel_offset = p;
	//    ind_to_sub(p, its_num_dims, its_image_size, its_cumprod, its_center_coords);
		ind_to_sub(p, its_num_dims, its_cumprod, its_center_coords);

		its_next_neighbor_idx = 0;
	}


	// test if the neighbor is within the bounds of the image
	bool isInboundsNeighbor(V3DLONG *offset_coords, V3DLONG *array_coords)
	{
		bool result = true;
		V3DLONG k;
		V3DLONG coordinate;
				
		for (k = 0; k < its_num_dims; k++)
		{
			coordinate = array_coords[k] + offset_coords[k];
			if ((coordinate < 0) || (coordinate >= its_image_size[k]))
			{
				result = false;
				break;
			}
		}
		
		return result;
	}


	// Gets the next neighbor that isn't outside the image.  Caller must call nhSetWalkerLocation first.
	// Outputs:
	//- p       Neigbhbor location, expressed as a zero-based offset into
	//            the image array
	//- idx     Index of the neighbor, 0 to P-1, where P is the number of 
	//		  neighbors in the neighborhood.  idx can be NULL, in which
	//            case this value is not output.
	// 
	// Return value:
	// true if successful; false if there were no more neighbors.  If false is 
	// returned then p and idx are not set.
	 
	 bool getNextInboundsNeighbor(V3DLONG *p, V3DLONG *idx)
	{
		bool found = false;
		V3DLONG k;
		
		for (k = its_next_neighbor_idx; k < its_num_neighbors; k++)
		{
			if (its_use[k] && isInboundsNeighbor(its_array_coords + k*its_num_dims, its_center_coords))            
			{
				found = true;
				*p = its_pixel_offset + its_neighbor_offsets[k];
				if (idx != NULL)
				{
					*idx = k;
				}
				its_next_neighbor_idx = k + 1;
				break;
			}
		}

		if (! found)
		{
			/*
			 * Make sure this walker can't be used again until
			 * nhSetWalkerLocation is called.
			 */
			its_pixel_offset = -1;
		}
			
		return found;
	}


	 // Get an array containing linear neighbor offsets, computed
	 // with respect to a given image size.
	 
	V3DLONG *getWalkerNeighborOffsets()
	{
		return(its_neighbor_offsets);
	}
	 

	// Get the num of neighbors in the walker
	V3DLONG getNumNeighbors()
	{
		return(its_num_neighbors);
	}
	
	 // test if a pixel is reached before the center pixel in a linear
	 // scan of image pixels
	bool isLeadingNeighbor(V3DLONG *coords, V3DLONG in_num_dims)
	{
		return (sub_to_relative_ind(coords, in_num_dims) > 0);
	}

	// test if a pixel  reached after the center pixel in a linear
	// scan of image pixels
	bool isTrailingNeighbor(V3DLONG *coords, V3DLONG in_num_dims)
	{
		return (sub_to_relative_ind(coords, in_num_dims) < 0);
	}

	// test if the coordinate is the neighborhood center
	bool isNeighborhoodCenter(V3DLONG *coords, V3DLONG in_num_dims)
	{
		V3DLONG k;
		bool result = true;
		
		for (k = 0; k < in_num_dims; k++)
		{
			if (coords[k] != 0)
			{
				result = false;
				break;
			}
		}
		
		return result;
	}
			
	~NeighborhoodWalker()
	{
		if (its_array_coords) { delete [] its_array_coords; its_array_coords = 0;}
		if (its_neighbor_offsets) { delete [] its_neighbor_offsets; its_neighbor_offsets = 0;}
		if (its_image_size) { delete [] its_image_size; its_image_size = 0;}
//		if (array_sz) { delete [] array_sz; array_sz = 0;}
		if (its_center_coords) { delete [] its_center_coords; its_center_coords = 0;}
		if (its_cumprod) { delete [] its_cumprod; its_cumprod = 0;}
		if (its_use) { delete [] its_use; its_use = 0;}		
	}
	
};

#endif