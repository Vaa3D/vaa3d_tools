/* private/tz_stack_bwmorph.c
 *
 * Initial write: Ting Zhao
 */

static int Se_In_Use = 0;

/* se_boundbox(): bounding box of a structure element.
 * 
 * Args: se - input structure element;
 *       corner - array to store the offset of the first corner;
 *       size - array to store the size of the box.
 *
 * Return: void.
 */
static void se_boundbox(const Struct_Element *se, 
			int *corner, int *size)
{
  int i, j;

  for (j = 0; j < 3; j++) {
    corner[j] = se->offset[0][j];
    size[j] = se->offset[0][j];
  }

  for (i = 1; i < se->size; i++) {
    for (j = 0; j < 3; j++) {
      if (corner[j] > se->offset[i][j]) {
	corner[j] = se->offset[i][j];
      } else if (size[j] < se->offset[i][j]) {
	size[j] = se->offset[i][j];
      }
    }
  }

  for (j = 0; j < 3; j++) {
    size[j] -= corner[j] - 1;
  }  
}

static void se_indices(const Struct_Element *se, 
		       int width, int height,
		       int *indices)
{
  int area = width * height;
  int i;
  for (i = 0; i <  se->size; i++) {
    indices[i] = se->offset[i][0] + se->offset[i][1] * width + 
      se->offset[i][2] * area;
  }
}

#if 0
static Struct_Element* se_expand(const Struct_Element *se)
{
  int corner[3];
  int size[3];
  se_boundbox(se, corner, size);
  
  Stack *stack = Make_Stack(GREY, size[0] * 2 - 1, size[1] * 2 - 1,
			    size[2] * 2 - 1);
  int center[3];
  int i, j, k;
  for (i = 0; i < 3; i++) {
    center[i] = size[i] - 1;
  }
  
  int offset[3];
  for (j = 0; j < se->size; j++) {
    for (i = 0; i < se->size; i++) {
      for (k = 0; k < 3; k++) {
	offset[k] = se->offset[i][k] - se->offset[j][k] + center[k];
      }
      Set_Stack_Pixel(stack, offset[0], offset[1], offset[2], 0, 1);
    }
  }

  return Stack_To_Se(stack, center, 1);
}
#endif

static Struct_Element* se_dilate(const Struct_Element *se)
{
  Struct_Element *se2 = New_Se(se->size);
  int i, j;
  for (i = 0; i < se->size; i++) {
    for (j = 0; j < 3; j++) {
      se2->offset[i][j] = - se->offset[i][j];
      se2->mask[i] = se->mask[i];
    }
  }
  
  return se2;
}

/* hit_out(): Test if a hit is outside of the stack boundary.
 *
 * Args: coord - coordinate of the pixel to test;
 *       nb_coord - neighbor coordinates;
 *       length - number fo the neighbor pixels;
 *       stack_size - array to store the result.
 *
 * Return: TRUE or FALSE.
 */
static inline int hit_out(Coordinate coord, Coordinate *nb_coord, 
			  int length, int stack_size[3])
{
  int i, j;
  int tmp_coord;

  for (i = 0; i < length; i++) {
    for (j = 0; j < 3; j++) {
      tmp_coord = coord[j] + nb_coord[i][j];
#if 0
      printf("coord[%d]: %d\n", j, tmp_coord);
#endif
      if ((tmp_coord < 0) || (tmp_coord >= stack_size[j])) {
	return TRUE;
      }
    }
  }
    
  return FALSE;
}

