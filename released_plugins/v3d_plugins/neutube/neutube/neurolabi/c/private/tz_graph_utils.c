#if defined NUMBER_OF_NEIGHBORS
#  undef NUMBER_OF_NEIGHBORS
#endif

#if defined NEIGHBOR_OF
#  undef NEIGHBOR_OF
#endif

#define NUMBER_OF_NEIGHBORS(v, neighbors) ((neighbors[v] == NULL) ? 0 : neighbors[v][0])
#define NEIGHBOR_OF(v, n, neighbors) neighbors[v][n]

static int extract_min(double *dist, int *checked, int nvertex,
		       Int_Arraylist *heap)
{
  int min_idx;
  double min;

  if (heap == NULL) {
    int i;
    min = Infinity;
    min_idx = -1;
    for (i = 0; i < nvertex; i++) {
      if (checked[i] > 1) {
	if (min > dist[i]) {
	  min = dist[i];
	  min_idx = i;
	}
      }
    }
  } else {  
    if (heap->length > 1) {
      min_idx = Int_Heap_Remove_I(heap, dist, checked);
      /*
      while (checked[min_idx] == 1) {
	if (heap->length <= 1) {
	  return -1;
	} else {
	  min_idx = Int_Heap_Remove_I(heap, dist);
	}
      }
      */
      min = dist[min_idx];
    } else {
      return -1;
    }
  }

#ifdef _DEBUG_2
  {
    int i;
    double tmp_min = Infinity;
    for (i = 0; i < nvertex; i++) {
      if (checked[i] > 1) {
	if (tmp_min > dist[i]) {
	  tmp_min = dist[i];
	}
      }
    }

    if (tmp_min != min) {
      printf("problem found: %g, %g\n", tmp_min, min);
      exit(1);
    }
  }
#endif

  if (min == Infinity) {
    min_idx = -1;
  } else {
    checked[min_idx] = 1;
  }

  return min_idx;
}

