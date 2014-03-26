/**
 * To map [0,w1-1] to [0,w2-1]. idx have length w2
 */
static void map_index(int w1,int w2,int* idx)
{
  if (w2 == 1) {
    idx[0] = 0;
    return;
  }
  int i;
  for(i=0;i<w2;i++)
    idx[i] = (i*2*(w1-1)/(w2-1)+1)/2;
}
