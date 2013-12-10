/* tz_voxel_graphics.c
 *
 * Initial write: Ting Zhao
 */

typedef enum { 
  VOXEL_GRAPH_X, VOXEL_GRAPH_Y, VOXEL_GRAPH_Z 
} Voxel_Graph_Axis_e;

typedef enum { 
  FORWARD, BACKWARD 
} Voxel_Graph_Direction_e; 

typedef struct tagVoxel_Graph_Conversion {
  int control_axis;
  int order;
} Voxel_Graph_Conversion;

void voxel_graph_line_standarize(Voxel_t start, Voxel_t end, 
				 Voxel_Graph_Conversion *vgc)
{
  int delta_x = abs(end[0] - start[0]);
  int delta_y = abs(end[1] - start[1]);
  int delta_z = abs(end[2] - start[2]);

  if (delta_x < delta_y) {
    if (delta_y < delta_z) {
      vgc->control_axis = VOXEL_GRAPH_Z;
    } else {
      vgc->control_axis = VOXEL_GRAPH_Y;
    }
  } else if (delta_x < delta_z) {
    vgc->control_axis = VOXEL_GRAPH_Z;
  } else {
    vgc->control_axis = VOXEL_GRAPH_X;
  }

  switch (vgc->control_axis) {
  case VOXEL_GRAPH_Y:
    switchint(&(start[0]), &(start[1]));
    switchint(&(end[0]), &(end[1]));
    break;
  case VOXEL_GRAPH_Z:
    switchint(&(start[0]), &(start[2]));
    switchint(&(end[0]), &(end[2]));
    break;
  default:
    break;
  }

  if (start[0] > end[0]) {
    int i;
    for (i = 0; i < 3; i++) {
      switchint(&(start[i]), &(end[i]));
    }
    vgc->order = BACKWARD;
  }
}

void voxel_graph_line_inverse_conversion(Voxel_t v, 
					 Voxel_Graph_Conversion *vgc)
{
  switch (vgc->control_axis) {
  case VOXEL_GRAPH_Y:
    switchint(&(v[0]), &(v[1]));
    break;
  case VOXEL_GRAPH_Z:
    switchint(&(v[0]), &(v[2]));
    break;
  default:
    break;
  }  
}

void voxel_graph_line_inverse_conversion_object(Object_3d *obj, 
						Voxel_Graph_Conversion *vgc)
{
  int i;
  switch (vgc->control_axis) {
  case VOXEL_GRAPH_Y:
    for (i = 0; i < obj->size; i++) {
      switchint(&(obj->voxels[i][0]), &(obj->voxels[i][1]));
    }
    break;
  case VOXEL_GRAPH_Z:
    for (i = 0; i < obj->size; i++) {
      switchint(&(obj->voxels[i][0]), &(obj->voxels[i][2]));
    }
    break;
  default:
    break;
  }
}
