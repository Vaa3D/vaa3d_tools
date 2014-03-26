#include "tools.h"

V_NeuronSWC segmentPruning(V_NeuronSWC in_swc)
{
    // map swc's index --> vector's index & in/out link

    // code from function of "decompose_V_NeuronSWC" by RZC
    // construct link map,
    Link_Map link_map = get_link_map(in_swc);
    /*qDebug*/("link_map is created.");

              vector <V_NeuronSWC> out_swc_segs;
              out_swc_segs.clear();

              // nchild as processed counter
              for (V3DLONG i=0; i<in_swc.row.size(); i++)
              {
                  V_NeuronSWC_unit & cur_node = in_swc.row[i];
                  Node_Link & nodelink = link_map[V3DLONG(cur_node.n)];
                  cur_node.nchild = nodelink.nlink;
                  //qDebug("#%d nlink = %d, in %d, out %d", V3DLONG(cur_node.n), nodelink.nlink, nodelink.in_link.size(), nodelink.out_link.size());
              }

              for (;;)
              {
                  // check is all nodes processed
                  V3DLONG n_removed = 0;
                  for (V3DLONG i=0; i<in_swc.row.size(); i++)
                  {
                      V_NeuronSWC_unit & cur_node = in_swc.row[i];
                      if (cur_node.nchild <=0) // is removed node ////// count down to 0
                          n_removed ++;
                  }
                  if (n_removed >= in_swc.row.size())
                  {
                      //qDebug("split_V_NeuronSWC_segs is done.");
                      break; //over, all nodes have been labeled to remove
                  }

                  // find a tip/out-branch/pure-out point as start point
                  V3DLONG istart = -1;
                  V3DLONG n_left = 0;
                  V3DLONG i_left = -1;
                  for (V3DLONG i=0; i<in_swc.row.size(); i++)
                  {
                      V_NeuronSWC_unit & cur_node = in_swc.row[i];
                      Node_Link & nodelink = link_map[V3DLONG(cur_node.n)];

                      if (cur_node.nchild <=0)
                          continue; //skip removed point

                      n_left++; //left valid point
                      i_left = i;

                      if ((nodelink.nlink ==1 && nodelink.in_link.size()==0) // tip point (include single point)
                          || (nodelink.nlink >2 && nodelink.out_link.size() >0) // out-branch point
                          || (nodelink.nlink ==2 && nodelink.in_link.size()==0)) // pure-out point
                          {
                          istart = i;
                          qDebug("start from #%d", V3DLONG(cur_node.n));
                          break; //find a start point
                      }
                  }
                  if (istart <0) //not find a start point
                  {
                      if (n_left)
                      {
                          qDebug("split_V_NeuronSWC_segs cann't find start point (left %d points)", n_left);
                          istart = i_left;
                      }
                      else
                      {
                          qDebug("split_V_NeuronSWC_segs finished.");
                          break;
                      }
                  }

                  // extract a simple segment
                  V_NeuronSWC new_seg;
                  new_seg.clear();
                  //qDebug("decompose_V_NeuronSWC_segs: segment from node #%d", j);
                  float segLen = 0;
                  bool isTip = false;
                  V3DLONG inext = istart;
                  for (V3DLONG n=1; inext>=0; n++)
                  {
                      V_NeuronSWC_unit & cur_node = in_swc.row[inext];
                      Node_Link & nodelink = link_map[V3DLONG(cur_node.n)];
                      //qDebug("	link #%d", V3DLONG(cur_node.n));

                      V_NeuronSWC_unit new_node = cur_node;
                      new_node.n = n;
                      new_node.parent = n+1; // link order as original order
                      new_seg.row.push_back(new_node);

                      if (nodelink.nlink ==1 && nodelink.in_link.size()==0)
                      {   // tip point
                          isTip = true;
                          cur_node.nchild --;
                          break;
                      }

                      if(cur_node.parent < 0 )    // root point ////////////////////////////
                      {
                          //qDebug("decompose_V_NeuronSWC_segs: segment end at root #%d", V3DLONG(cur_node.n));
                          cur_node.nchild --;
                          break; //over, a simple segment
                      }
                      else if (n>1 && nodelink.nlink >2)  // branch point (in link_map) ///////////
                      {
                          //qDebug("decompose_V_NeuronSWC_segs: segment end at branch #%d", V3DLONG(cur_node.n));
                          cur_node.nchild --;
                          break; //over, a simple segment
                      }
                      else if (n>1 && inext==istart)  // i_left point (a loop) ///////////
                      {
                          //qDebug("decompose_V_NeuronSWC_segs: segment end at branch #%d", V3DLONG(cur_node.n));
                          cur_node.nchild --;
                          break; //over, a simple segment
                      }
                      else  //(nodelink.nlink==2)   // path node ///////////////////////////////////////////////
                      {
                          //qDebug("decompose_V_NeuronSWC_segs: node #%d", j);
                          V3DLONG parent = cur_node.parent;
                          inext = link_map[parent].i; //// next point in seg
                          cur_node.nchild = -1;  // label to remove
                      }
                  }

                  if ( isTip )
                  {   // measure length
                      segLen = 0;
                      float preX = new_seg.row.at(0).x;
                      float preY = new_seg.row.at(0).y;
                      float preZ = new_seg.row.at(0).z;

                      for (int i = 1; i < new_seg.row.size(); i++ )
                      {
                          float curX = new_seg.row.at(i).x;
                          float curY = new_seg.row.at(i).y;
                          float curZ = new_seg.row.at(i).z;
                          segLen += sqrt( (preX - curX)*(preX - curX) + (preY - curY)*(preY - curY) +(preZ - curZ)*(preZ - curZ) );
                      }

                      // compare the length and radius of parent branch point
                      if ( segLen < 2 * in_swc.row[istart].r )
                      {
                          continue;
                      }
                  }

                  if (new_seg.row.size()>0 )//>=2)//? single point
                  {
                      new_seg.row[new_seg.row.size()-1].parent = -1; // set segment end
                      char buf[10];
                      new_seg.name = sprintf(buf,"%d", out_swc_segs.size()+1);
                      new_seg.b_linegraph=true; //donot forget to do this //####################
                      out_swc_segs.push_back(new_seg);
                  }
              }

              // merge the segments
              V_NeuronSWC_list swcList;
              swcList.seg = out_swc_segs;
              V_NeuronSWC out_swc = merge_V_NeuronSWC_list (swcList);
              return out_swc;
          }


int otsu (int * ihist)
{
    unsigned char *np;     // ÍŒÏñÖžÕë
    int thresholdValue=10; // ãÐÖµ
    //    int ihist[256];         // ÍŒÏñÖ±·œÍŒ£¬256žöµã

    int i, j, k;       // various counters
    int n, n1, n2;
    double m1, m2, sum, csum, fmax, sb;

    // set up everything
    sum = csum = 0.0;
    n = 0;

    for (k = 0; k <= 255; k++) {
        sum += (double) k * (double) ihist[k];   /* x*f(x) ÖÊÁ¿ŸØ*/
        n   += ihist[k];                           /* f(x)   ÖÊÁ¿   */
    }

    if (!n) {
        // if n has no value, there is problems...
        fprintf (stderr, "NOT NORMAL thresholdValue = 160\n");
        return (160);
    }

    // do the otsu global thresholding method
    fmax = -1.0;
    n1 = 0;
    for (k = 0; k < 255; k++) {
        n1 += ihist[k];
        if (!n1) { continue; }
        n2 = n - n1;
        if (n2 == 0) { break; }
        csum += (double) k *ihist[k];
        m1 = csum / n1;
        m2 = (sum - csum) / n2;
        sb = (double) n1 *(double) n2 *(m1 - m2) * (m1 - m2);
        /* bbg: note: can be optimized. */
        if (sb > fmax) {
            fmax = sb;
            thresholdValue = k;
        }
    }
}
int otsu2 (int * ihist)
{
    int thresholdValue=10;
    int n, n1, n2;
    double m1, m2, sum, csum, fmax, sb;
 /*   for(int k =0;k<=255;k++)
    {
        float total =0;
        for(int t =-2;t<=2;t++)
        {
            int q = k+t;
            if(q<0)
                q=0;
            if(q>255)
                q=255;
            total =total+ihist[q];
        }
        ihist[k]=(int)(total/5.0+0.5);
    }*/
    sum =csum =0.0;
    n=0;
    for(int k=0;k<=255;k++)
    {
        sum+=(double)k*(double)ihist[k];
        n+=ihist[k];
    }
    fmax = -1.0;
    n1=0;
    for(int k=0;k<=255;k++)
    {
        n1+=ihist[k];
        if(n1==0){
            continue;
        }
        n2=n-n1;
        if(n2==0){
            break;
        }
        csum+=(double)k*ihist[k];
        m1=csum/n1;
        m2=(sum-csum)/n2;
        sb=(double)n1*(double)n2*(m1-m2)*(m1-m2);
        if(sb>fmax)
        {
            fmax =sb;
            thresholdValue = k;
        }

    }
    return thresholdValue;

}
