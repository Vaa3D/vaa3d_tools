static double path_length(Int_Arraylist *path, int width, int height, double zscale)
{
  if (path->length < 2) {
    return 0.0;
  }

  coordinate_3d_t coord1, coord2;
  int sub[3];
  Stack_Util_Coord(path->array[0], width, height, sub, sub+1, sub+2);
  Set_Coordinate_3d(coord1, sub[0], sub[1], sub[2]);

  double dist = 0.0;
  int k;
  for (k = 1; k < path->length; k++) {
    Stack_Util_Coord(path->array[k], width, height, sub, sub+1,
	sub+2);
    Set_Coordinate_3d(coord2, sub[0], sub[1], sub[2]);
    dist += Coordinate_3d_Distance(coord1, coord2);
    Coordinate_3d_Copy(coord1, coord2);
  }

  return dist;
}

