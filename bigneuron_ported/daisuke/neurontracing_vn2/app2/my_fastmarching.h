#ifndef __MY_FASTMARCHING_H__
#define __MY_FASTMARCHING_H__

#include "fastmarching_dt.h"
#include "fastmarching_tree.h"
#ifdef USE_OPENMP
#include <omp.h>
#endif

#include <algorithm>

#ifndef SET_CLOCK
#define SET_CLOCK  clock_gettime(CLOCK_MONOTONIC, &(ts[clock_id])); printf("[ timer %d - %d ]\n", area_id, clock_id++);
#endif


template<class T> bool my_fastmarching_dt_XY(T * inimg1d, float * &phi, int sz0, int sz1, int sz2, int cnn_type = 3, int bkg_thresh = 0)
{
#ifdef NEB_DEBUG
  struct timespec ts[4];
  int clock_id=0, area_id=2;
  printf("-----start my fastmarching(sz0=%d, sz1=%d, sz2=%d)-----\n", sz0, sz1, sz2);
  SET_CLOCK
#endif
#ifdef USE_OPENMP
    //omp_set_num_threads(1);
    printf("OpenMP thread=%d / %d\n", omp_get_max_threads(), omp_get_num_procs());
#endif

  enum{ALIVE = -1, TRIAL = 0, FAR = 1};
	
  const long tol_sz = sz0 * sz1 * sz2;
  const long sz01 = sz0 * sz1;
  //int cnn_type = 3;  // ?
	
  if(phi == 0) phi = new float[tol_sz]; 
  char * state = new char[tol_sz];

  int bkg_count = 0;                          // for process counting
  int bdr_count = 0;                          // for process counting


  for(long i = 0; i < tol_sz; i++)
    {
      if(inimg1d[i] <= bkg_thresh)
	{
	  phi[i] = inimg1d[i];
	  state[i] = ALIVE;
	  //cout<<"+";cout.flush();
	  bkg_count++;
	}
      else
	{
	  phi[i] = INF;
	  state[i] = FAR;
	}
    }
  cout<<endl;

#ifdef NEB_DEBUG
  SET_CLOCK
#endif
	
    BasicHeap<HeapElem> heap;
  map<long, HeapElem*> elems;
	
  // init heap
  {
    //long i = -1, j = -1, k = -1;
    /*
      for(long ind = 0; ind < tol_sz; ind++)
      {
      long i = ind%sz0;
      long j = (ind/sz0)%sz1;
      long k = ind/(sz0*sz1);
    */
    /*
      i++; 
      if(i%sz0 == 0)
      {
      i=0; j++; 
      if(j%sz1==0){j=0; k++;}
      }
    */

    const long sz1sz0 =sz1*sz0;
#ifdef USE_OPENMP
    printf("start omp\n");
#pragma omp parallel for
#endif
    for(long k=0; k<sz2; k++){
      long nth = omp_get_thread_num();
      for(long j=0; j<sz1; j++){
	for(long i=0; i<sz0; i++){
	  long ind = k*sz1sz0 + j*sz0 + i;
		  
	  if(state[ind] == ALIVE){
	    for(int kk = 0; kk <= 0; kk++){
	      long k2 = k+kk;
	      if(k2 < 0 || k2 >= sz2) continue;
	      for(int jj = -1; jj <= 1; jj++){
		long j2 = j+jj;
		if(j2 < 0 || j2 >= sz1) continue;
		for(int ii = -1; ii <=1; ii++){
		  long i2 = i+ii;
		  if(i2 < 0 || i2 >= sz0) continue;
		  int offset = ABS(ii) + ABS(jj) + ABS(kk);
		  if(offset == 0 || offset > cnn_type) continue;
		  long ind2 = k2 * sz01 + j2 * sz0 + i2;

		  if(state[ind2] == FAR){
		    long min_ind = ind;

		    // get minimum Alive point around ind2
		    if(phi[min_ind] > 0.0){
		      //for(int kkk = 0; kkk <= 0; kkk++){
		      //long k3 = k2 + kkk;
		      //if(k3 >= sz2 || k3 < 0) continue;
		      {
			long j3, i3, ind3, jjj, iii, offset2;

			jjj=-1; iii=-1; offset2=2;
			j3=j2+jjj; i3=i2+iii; ind3 = k2 * sz01 + j3 * sz0 + i3;
			if(state[ind3] == ALIVE && phi[ind3] < phi[min_ind]){ if(!(i3 >= sz0 || i3 < 0 || j3 >= sz1 || j3 < 0 || offset2 > cnn_type)){ min_ind = ind3; } }
			jjj=-1; iii=0; offset2=1;
			j3=j2+jjj; i3=i2+iii; ind3 = k2 * sz01 + j3 * sz0 + i3;
			if(state[ind3] == ALIVE && phi[ind3] < phi[min_ind]){ if(!(i3 >= sz0 || i3 < 0 || j3 >= sz1 || j3 < 0 || offset2 > cnn_type)){ min_ind = ind3; } }
			jjj=-1; iii=1; offset2=2;
			j3=j2+jjj; i3=i2+iii; ind3 = k2 * sz01 + j3 * sz0 + i3;
			if(state[ind3] == ALIVE && phi[ind3] < phi[min_ind]){ if(!(i3 >= sz0 || i3 < 0 || j3 >= sz1 || j3 < 0 || offset2 > cnn_type)){ min_ind = ind3; } }

			jjj=0;  iii=-1; offset2=1;
			j3=j2+jjj; i3=i2+iii; ind3 = k2 * sz01 + j3 * sz0 + i3;
			if(state[ind3] == ALIVE && phi[ind3] < phi[min_ind]){ if(!(i3 >= sz0 || i3 < 0 || j3 >= sz1 || j3 < 0 || offset2 > cnn_type)){ min_ind = ind3; } }
			jjj=0;  iii=1; offset2=1;
			j3=j2+jjj; i3=i2+iii; ind3 = k2 * sz01 + j3 * sz0 + i3;
			if(state[ind3] == ALIVE && phi[ind3] < phi[min_ind]){ if(!(i3 >= sz0 || i3 < 0 || j3 >= sz1 || j3 < 0 || offset2 > cnn_type)){ min_ind = ind3; } }

			jjj=1;  iii=-1; offset2=2;
			j3=j2+jjj; i3=i2+iii; ind3 = k2 * sz01 + j3 * sz0 + i3;
			if(state[ind3] == ALIVE && phi[ind3] < phi[min_ind]){ if(!(i3 >= sz0 || i3 < 0 || j3 >= sz1 || j3 < 0 || offset2 > cnn_type)){ min_ind = ind3; } }
			jjj=1;  iii= 0; offset2=1;
			j3=j2+jjj; i3=i2+iii; ind3 = k2 * sz01 + j3 * sz0 + i3;
			if(state[ind3] == ALIVE && phi[ind3] < phi[min_ind]){ if(!(i3 >= sz0 || i3 < 0 || j3 >= sz1 || j3 < 0 || offset2 > cnn_type)){ min_ind = ind3; } }
			jjj=1;  iii=1; offset2=2;
			j3=j2+jjj; i3=i2+iii; ind3 = k2 * sz01 + j3 * sz0 + i3;
			if(state[ind3] == ALIVE && phi[ind3] < phi[min_ind]){ if(!(i3 >= sz0 || i3 < 0 || j3 >= sz1 || j3 < 0 || offset2 > cnn_type)){ min_ind = ind3; } }
			
		      }
		    }

		    // over
		    phi[ind2] = phi[min_ind] + inimg1d[ind2];
		    state[ind2] = TRIAL;
		    HeapElem * elem = new HeapElem(ind2, phi[ind2]);
#pragma omp critical
		    {
		      heap.insert(elem);
		      elems[ind2] = elem;
		      bdr_count++;
		    }
		  }
		}
	      }
	    }
	  }
	}
      }
    }
  }
	
  cout<<"bkg_count = "<<bkg_count<<" ("<<bkg_count/(double)tol_sz<<")"<<endl;
  cout<<"bdr_count = "<<bdr_count<<" ("<<bdr_count/(double)tol_sz<<")"<<endl;
  cout<<"elems.size() = "<<elems.size()<<endl;

#ifdef NEB_DEBUG
  SET_CLOCK
#endif
    // loop
    int time_counter = bkg_count;
  double process1 = 0;
  while(!heap.empty())
    {
      double process2 = (time_counter++)*100000.0/tol_sz;
      if(process2 - process1 >= 10) {cout<<"\r"<<((int)process2)/1000.0<<"%";cout.flush(); process1 = process2;
	//SAVE_PHI_IMAGE(phi, sz0, sz1, sz2, string("phi") + num2str((int)process1) + ".tif");
      }
		
      HeapElem* min_elem = heap.delete_min();
      elems.erase(min_elem->img_ind);
		
      long min_ind = min_elem->img_ind;
      delete min_elem;
		
      state[min_ind] = ALIVE;
      int i = min_ind % sz0; 
      int j = (min_ind/sz0) % sz1; 
      int k = (min_ind/sz01) % sz2;
		
      int w, h, d;
      for(int kk = 0; kk <= 0; kk++)
	{
	  d = k+kk;
	  if(d < 0 || d >= sz2) continue;
	  for(int jj = -1; jj <= 1; jj++)
	    {
	      h = j+jj;
	      if(h < 0 || h >= sz1) continue;
	      for(int ii = -1; ii <= 1; ii++)
		{
		  w = i+ii;
		  if(w < 0 || w >= sz0) continue;
		  int offset = ABS(ii) + ABS(jj) + ABS(kk);
		  if(offset == 0 || offset > cnn_type) continue;
		  long index = d*sz01 + h*sz0 + w;
					
		  if(state[index] != ALIVE)
		    {
		      float new_dist = phi[min_ind] + inimg1d[index] * sqrt(double(offset));
						
		      if(state[index] == FAR)
			{
			  phi[index] = new_dist;
			  HeapElem * elem = new HeapElem(index, phi[index]);
			  heap.insert(elem);
			  elems[index] = elem;
			  state[index] = TRIAL;
			}
		      else if(state[index] == TRIAL)
			{
			  if(phi[index] > new_dist)
			    {
			      phi[index] = new_dist;
			      HeapElem * elem = elems[index];
			      heap.adjust(elem->heap_id, phi[index]);
			    }
			}
		    }
		}
	    }
	}
    }
	
  assert(elems.empty());
  if(state) {
    printf("delete state\n");
    delete [] state; state = 0;
  }

#ifdef NEB_DEBUG
  SET_CLOCK
    printf("*************************************\n");
  for(int i; i<clock_id-1; i++){
    printf("* time(ts[%d - %d] - tm[%d - %d]) = %3.2f\n", area_id, i, area_id, i+1, (ts[i+1].tv_sec - ts[i].tv_sec) + 0.000000001*(ts[i+1].tv_nsec - ts[i].tv_nsec));
  }
  printf("*************************************\n");
#endif


  return true;
}

typedef struct _parent{
  long id;
  long parent;
} ParentList;

/*********************************************************************
 * Function : fastmarching_tree
 *
 * Features : 
 * 1. Create fast marcing tree from root marker only
 * 2. Background (intensity 0) will be ignored. 
 * 3. Graph augumented distance is used
 *
 * Input : root          root marker
 *         inimg1d       original 8bit image
 *
 * Output : tree         output swc
 *          phi          the distance for each pixels
 * *******************************************************************/
template<class T> bool my_fastmarching_tree(MyMarker root
					    , T * inimg1d
					    , vector<MyMarker*> &outtree
					    , long sz0
					    , long sz1
					    , long sz2
					    , int cnn_type = 3
					    , double bkg_thresh = 20
					    , bool is_break_accept = false
#ifdef REUSE_PHI
					    , float * &phi = 0
#endif
					    )
{
  enum{ALIVE = -1, TRIAL = 0, FAR = 1};
  
  long tol_sz = sz0 * sz1 * sz2;
  long sz01 = sz0 * sz1;
  long i;
  //int cnn_type = 3;  // ?
  
  //float * phi = 0;
  //long * parent = 0;
  char * state = 0;
  
#ifdef NEB_DEBUG
  struct timespec ts[10];
  int clock_id=0, area_id=3;
  printf("-----start my fastmarching tree(sz0=%d, sz1=%d, sz2=%d)-----\n", sz0, sz1, sz2);
  SET_CLOCK ;
#endif
    

  /*
  long *parent;
  long parent_index=0, parent_allocate_span=tol_sz/10;
  parent = (long *)malloc(parent_allocate_span * sizeof(long));
  */
  vector<ParentList> parentlist;
  parentlist.clear();
  
  try
  {
    if(phi==0){ phi = new float[tol_sz]; }
    //parent = new long[tol_sz];
    state = new char[tol_sz];
  }
  catch (...)
    {
      cout << "********* Fail to allocate memory. quit fastmarching_tree()." << endl;
      if (phi) {delete []phi; phi=0;}
      //if (parent) {delete []parent; parent=0;}
      if (state) {delete []state; state=0;}
      return false;
    }
    
#ifdef NEB_DEBUG
  SET_CLOCK ;
#endif
  fill(phi, &(phi[tol_sz]), INF);
  //fill(parent, &(parent[tol_sz]), -1);

  // change initialization value of parent to constant (i -> -1)
  //fill(parent, &(parent[tol_sz]), -1);

  //for(long i = 0; i < tol_sz; i++){ parent[i] = i; }
  memset(state, FAR, tol_sz);

  /*
    for(long i = 0; i < tol_sz; i++){
    phi[i] = INF;
    parent[i] = i;
    state[i] = FAR;
    }
  */

#ifdef NEB_DEBUG
  printf("INF=%f\n", (float)INF);
  for(long i=0; i<10; i++){ printf("%d : %f %d\n", i, phi[i], state[i]); }
  for(long i=tol_sz-1; i>tol_sz-10; i--){ printf("%d : %f %d\n", i, phi[i], state[i]); }
#endif    

    
#ifdef NEB_DEBUG
  SET_CLOCK ;
#endif

  // GI parameter min_int, max_int, li
  double max_int = 0; // maximum intensity, used in GI
  double min_int = INF;
  for(i = 0; i < tol_sz; i++) 
    {
      if (inimg1d[i] > max_int) max_int = inimg1d[i];
      else if (inimg1d[i] < min_int) min_int = inimg1d[i];
    }
  max_int -= min_int;
  double li = 10;
	
  // initialization

  // init state and phi for root
  long rootx = root.x + 0.5;
  long rooty = root.y + 0.5;
  long rootz = root.z + 0.5;

  long root_ind = rootz*sz01 + rooty*sz0 + rootx;
  state[root_ind] = ALIVE; 
  phi[root_ind] = 0.0;

  BasicHeap<HeapElemX> heap;
  map<long, HeapElemX*> elems;

  // init heap
  {
    long index = root_ind;
    HeapElemX *elem = new HeapElemX(index, phi[index]);
    elem->prev_ind = index;
    heap.insert(elem);
    elems[index] = elem;
  }


#ifdef NEB_DEBUG
  SET_CLOCK ;
#endif

  // loop
  int time_counter = 1;
  double process1 = 0;
    
  while(!heap.empty())
    {
      double process2 = (time_counter++)*10000.0/tol_sz;
      //cout<<"\r"<<((int)process2)/100.0<<"%";cout.flush();
      if(process2 - process1 >= 1) 
	{
	  cout<<"\r"<<((int)process2)/100.0<<"%";cout.flush(); process1 = process2;
	}

      HeapElemX* min_elem = heap.delete_min();
      elems.erase(min_elem->img_ind);

      long min_ind = min_elem->img_ind;
      long prev_ind = min_elem->prev_ind;
      delete min_elem;

      //change_parent_style
      //parent[min_ind] = prev_ind;

      ParentList tmp_plist;
      tmp_plist.id = min_ind;
      tmp_plist.parent = prev_ind;
      parentlist.push_back(tmp_plist);


      state[min_ind] = ALIVE;
      int i = min_ind % sz0; 
      int j = (min_ind/sz0) % sz1; 
      int k = (min_ind/sz01) % sz2;

      int w, h, d;
      for(int kk = -1; kk <= 1; kk++)
	{
	  d = k+kk;
	  if(d < 0 || d >= sz2) continue;
	  for(int jj = -1; jj <= 1; jj++)
	    {
	      h = j+jj;
	      if(h < 0 || h >= sz1) continue;
	      for(int ii = -1; ii <= 1; ii++)
		{
		  w = i+ii;
		  if(w < 0 || w >= sz0) continue;
		  int offset = ABS(ii) + ABS(jj) + ABS(kk);
		  if(offset == 0 || offset > cnn_type) continue;
		  double factor = (offset == 1) ? 1.0 : ((offset == 2) ? 1.414214 : ((offset == 3) ? 1.732051 : 0.0));
		  long index = d*sz01 + h*sz0 + w;
		  if (is_break_accept)
		    {
		      if(inimg1d[index] <= bkg_thresh &&
			 inimg1d[min_ind] <= bkg_thresh)
			continue;
		    }
		  else
		    {
		      if(inimg1d[index] <= bkg_thresh)
			continue;
		    }

		  if(state[index] != ALIVE)
		    {
		      double new_dist = phi[min_ind] + (GI(index) + GI(min_ind))*factor*0.5;
		      long prev_ind = min_ind;

		      if(state[index] == FAR)
			{
			  phi[index] = new_dist;
			  HeapElemX * elem = new HeapElemX(index, phi[index]);
			  elem->prev_ind = prev_ind;
			  heap.insert(elem);
			  elems[index] = elem;
			  state[index] = TRIAL;
			}
		      else if(state[index] == TRIAL)
			{
			  if (phi[index] > new_dist)
			    {
			      phi[index] = new_dist;
			      HeapElemX * elem = elems[index];
			      heap.adjust(elem->heap_id, phi[index]);
			      elem->prev_ind = prev_ind;
			    }
			}
		    }
		}
	    }
	}
    }
  printf("psize = %d\n", parentlist.size());
	
  
#ifdef NEB_DEBUG
  SET_CLOCK ;
#endif



  // save current swc tree
  if (1)
    {
      map<long, MyMarker*> tmp_map;

      const long sz1sz0 = sz1 * sz0;
      for(long k=0; k<sz2; k++){
	for(long j=0; j<sz1; j++){
	  for(long i=0; i<sz0; i++){
	    long ind = (k*sz1sz0) + (j*sz0) + i;

	    if(state[ind] != ALIVE) continue;
	    MyMarker * marker = new MyMarker(i,j,k);
	    tmp_map[ind] = marker;
	    outtree.push_back(marker);
	  }
	}
      }

      for(long i=0; i<tmp_map.size(); i++){
	if(state[i] != ALIVE) continue;
	tmp_map[i]->parent = 0;
      }
      for(long i=0; i<parentlist.size(); i++){
	if(state[parentlist[i].id] != ALIVE) continue;

	MyMarker * marker1 = tmp_map[parentlist[i].id];
	if(parentlist[i].parent>0 && parentlist[i].parent!=parentlist[i].id){
	  MyMarker * marker2 = tmp_map[parentlist[i].parent];
	  marker1->parent = marker2;
	}
      }

      /*
      long counter=0;
      for(long k=0; k<sz2; k++){
	for(long j=0; j<sz1; j++){
	  for(long i=0; i<sz0; i++){
	    long ind = (k*sz1sz0) + (j*sz0) + i;
	    if(state[ind] != ALIVE) continue;
	    
	    long ind2 = parent[ind];
	    MyMarker * marker1 = tmp_map[ind];
	    if(ind2 < 0 || marker1 == tmp_map[ind2]){ 
	      marker1->parent = 0;
	    }else{
	      MyMarker * marker2 = tmp_map[ind2];
	      marker1->parent = marker2;
	      counter++;
	      //tmp_map[ind]->parent = tmp_map[ind2];
	    }
	  }
	}
      }
      printf("counter=%d\n", counter);
      */
    }

#ifdef NEB_DEBUG
  SET_CLOCK ;
#endif

  // over
	
  map<long, HeapElemX*>::iterator mit = elems.begin();
  while (mit != elems.end())
    {
      HeapElemX * elem = mit->second; delete elem; mit++;
    }

  parentlist.clear();

  if(phi){delete [] phi; phi = 0;}
  //if(parent){delete [] parent; parent = 0;}
  if(state) {delete [] state; state = 0;}

#ifdef NEB_DEBUG
  SET_CLOCK
    printf("*************************************\n");
  for(int i; i<clock_id-1; i++){
    printf("* time(ts[%d - %d] - tm[%d - %d]) = %3.2f\n", area_id, i, area_id, i+1, (ts[i+1].tv_sec - ts[i].tv_sec) + 0.000000001*(ts[i+1].tv_nsec - ts[i].tv_nsec));
  }
  printf("*************************************\n");
#endif

  return true;
}



#endif /* __MY_FASTMARCHING_H__ */
