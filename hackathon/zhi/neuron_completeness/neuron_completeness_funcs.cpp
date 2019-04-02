/* neuron_completeness_plugin.cpp
 * This is a test plugin, you can use it as a demo.
 * 2019-2-25 : by Zhi Zhou
 */
 
#include "v3d_message.h"
#include <algorithm>
#include "neuron_completeness_funcs.h"
#include "../../../released_plugins/v3d_plugins/sort_neuron_swc/sort_swc.h"

#include "neuron_completeness_plugin.h"


map<size_t, set<size_t> > seg2SegsMap;
V_NeuronSWC_list segList;
set<vector<size_t> > detectedLoops;
set<set<size_t> > detectedLoopsSet;
set<set<size_t> > finalizedLoopsSet;
set<set<size_t> > nonLoopErrors;
multimap<string, size_t> segEnd2segIDmap;
map<size_t, set<size_t> > segTail2segIDmap;
size_t rcCount;
size_t testCount;

QStringList importSWCFileList(const QString & curFilePath)
{
    QStringList myList;
    myList.clear();

    // get the image files namelist in the directory
    QStringList imgSuffix;
    imgSuffix<<"*.swc"<<"*.eswc"<<"*.SWC"<<"*.ESWC";


    QDir dir(curFilePath);
    if (!dir.exists())
    {
        qWarning("Cannot find the directory");
        return myList;
    }

    foreach (QString file, dir.entryList(imgSuffix, QDir::Files, QDir::Name))
    {
        myList += QFileInfo(dir, file).absoluteFilePath();
    }

    // print filenames
    foreach (QString qs, myList)  qDebug() << qs;

    return myList;
}


double minDist(QList<NeuronSWC> & neuron1, QList<NeuronSWC> & neuron2)
{
    double dis=MAX_DOUBLE;
    for(int i=0;i<neuron1.size();i++)
    {
        for(int j=0;j<neuron2.size();j++)
        {
            if(NTDIS(neuron1[i],neuron2[j])<dis)
                dis = NTDIS(neuron1[i],neuron2[j]);
        }
    }
    return dis;
}

void calComplete(NeuronTree nt, QList<NEURON_METRICS> & scores)
{
    QList<NeuronSWC> sorted_neuron;
    SortSWC(nt.listNeuron, sorted_neuron ,VOID, 0);

    LandmarkList markerlist;

    QHash<int,int> map_type;
    QMultiMap<int, QList<NeuronSWC> > multi_neurons;
    int first = 0;
    for(V3DLONG i=0; i<sorted_neuron.size();i++)
    {
        if(sorted_neuron.at(i).pn == -1)
        {
            if(i!=0)
            {
                multi_neurons.insert(i-first,sorted_neuron.mid(first,i-first));
            }
            first = i;

        }
        if(!map_type.count(sorted_neuron.at(i).type))
        {
            map_type[sorted_neuron.at(i).type] = 1;
        }
    }
    multi_neurons.insert(sorted_neuron.size()-first,sorted_neuron.mid(first,sorted_neuron.size()-first));
    QList<double> dist;
    QVector<QVector<double> > matrix_dis(multi_neurons.size(),QVector<double>(multi_neurons.size(),MAX_DOUBLE));
    int row =0;
    for (QMultiMap<int, QList<NeuronSWC> >::iterator it1 = multi_neurons.end()-1; it1 != multi_neurons.begin()-1; --it1)
    {
        QList<NeuronSWC> neuron1= it1.value();
        double minD = MAX_DOUBLE;
        int col=0;
        for (QMultiMap<int, QList<NeuronSWC> >::iterator it2 = multi_neurons.end()-1; it2 != multi_neurons.begin()-1; --it2)
        {
            if(it1 != it2)
            {
                if(matrix_dis[row][col] != MAX_DOUBLE)
                    minD = MIN(minD,matrix_dis[row][col]);
                else
                {
                    QList<NeuronSWC> neuron2 = it2.value();
                    double cur_dis = minDist(neuron1,neuron2);
                    matrix_dis[row][col] = cur_dis;
                    matrix_dis[col][row] = cur_dis;
                    minD = MIN(minD,cur_dis);
                }
            }
            col++;
        }
        row++;
        dist.push_back(minD);
        if(minD>2 && minD!=MAX_DOUBLE)
        {
            LocationSimple t;
            t.x = neuron1.at(0).x;
            t.y = neuron1.at(0).y;
            t.z = neuron1.at(0).z;
            t.color.r = 0;t.color.g = 0; t.color.b = 255;
            markerlist.push_back(t);
        }
    }

    NEURON_METRICS tmp;
    tmp.numTrees = multi_neurons.size();
    tmp.numTypes = map_type.size();
    tmp.numSegs = markerlist.size();
    tmp.numRoots = map_type.count(1)?map_type[1]:0;
    scores.push_back(tmp);
    return;
}

void exportComplete(NeuronTree nt,QList<NeuronSWC>& sorted_neuron, LandmarkList& markerlist, QMultiMap<int, QList<NeuronSWC> >& multi_neurons,QHash<int,int>& map_type,QList<double>& dist)
{
    SortSWC(nt.listNeuron, sorted_neuron ,VOID, 0);

    int first = 0;
    int cur_type = -1;
    for(V3DLONG i=0; i<sorted_neuron.size();i++)
    {
        if(sorted_neuron.at(i).pn == -1)
        {
            if(i!=0)
            {
                multi_neurons.insert(i-first,sorted_neuron.mid(first,i-first));
            }
            first = i;

        }
        if(sorted_neuron.at(i).type==1)
        {
            map_type[sorted_neuron.at(i).type]++;
        }
        else if(!map_type.count(sorted_neuron.at(i).type))
        {
            map_type[sorted_neuron.at(i).type] = 1;
//            if(sorted_neuron.at(i).type!=1 && sorted_neuron.at(i).type!=2 && sorted_neuron.at(i).type!=3)
//            {
//                LocationSimple t;
//                t.x = sorted_neuron.at(i).x;
//                t.y = sorted_neuron.at(i).y;
//                t.z = sorted_neuron.at(i).z;
//                t.color.r = 255;t.color.g = 0; t.color.b = 0;
//                markerlist.push_back(t);
//            }
        }/*else
        {
            if(sorted_neuron.at(i).type!=1 && sorted_neuron.at(i).type!=2 && sorted_neuron.at(i).type!=3 && sorted_neuron.at(i).type !=cur_type)
            {
                LocationSimple t;
                t.x = sorted_neuron.at(i).x;
                t.y = sorted_neuron.at(i).y;
                t.z = sorted_neuron.at(i).z;
                t.color.r = 255;t.color.g = 0; t.color.b = 0;
                markerlist.push_back(t);
            }
        }*/
        cur_type = sorted_neuron.at(i).type;
    }
    multi_neurons.insert(sorted_neuron.size()-first,sorted_neuron.mid(first,sorted_neuron.size()-first));
    QVector<QVector<double> > matrix_dis(multi_neurons.size(),QVector<double>(multi_neurons.size(),MAX_DOUBLE));
    int row =0;
    QMultiMap<double, LocationSimple> greenSeg_list;
    for (QMultiMap<int, QList<NeuronSWC> >::iterator it1 = multi_neurons.end()-1; it1 != multi_neurons.begin()-1; --it1)
    {
        QList<NeuronSWC> neuron1= it1.value();
        double minD = MAX_DOUBLE;
        int col=0;
        for (QMultiMap<int, QList<NeuronSWC> >::iterator it2 = multi_neurons.end()-1; it2 != multi_neurons.begin()-1; --it2)
        {
            if(it1 != it2)
            {
                if(matrix_dis[row][col] != MAX_DOUBLE)
                    minD = MIN(minD,matrix_dis[row][col]);
                else
                {
                    QList<NeuronSWC> neuron2 = it2.value();
                    double cur_dis = minDist(neuron1,neuron2);
                    matrix_dis[row][col] = cur_dis;
                    matrix_dis[col][row] = cur_dis;
                    minD = MIN(minD,cur_dis);
                }
            }
            col++;
        }
        row++;
        dist.push_back(minD);
        LocationSimple t;
        t.x = neuron1.at(0).x;
        t.y = neuron1.at(0).y;
        t.z = neuron1.at(0).z;
        if (minD>2 && minD!=MAX_DOUBLE)
        {
            greenSeg_list.insert(minD,t);
        }
    }
    for(QMultiMap<double, LocationSimple>::iterator it = greenSeg_list.end()-1; it != greenSeg_list.begin()-1; --it)
    {
        LocationSimple t = it.value();
        t.color.r = 0;t.color.g = 255; t.color.b = 0;t.comments = QString("gap: %1").arg(QString::number(it.key(),'f',2)).toStdString();
        markerlist.push_back(t);
    }
    return;

}

void markerlist_before_sorting(QList<NeuronSWC>ori_tree1swc,LandmarkList &markerlist,int & numofwrongtype){

    vector<int> suspoint_before;
    QList<int> plist;
    QList<int> alln;
    int N=ori_tree1swc.size();
    for(int i=0; i<N; i++){
        plist.append(ori_tree1swc.at(i).pn);
        alln.append(ori_tree1swc.at(i).n);
      }
//find the root of wrong type which is not 1,2,3 neither 4
    int numofduplicated=0;
    for (int i=0;i<ori_tree1swc.size();i++)
     {
        if(ori_tree1swc.at(i).type != 1 && ori_tree1swc.at(i).type != 2 && ori_tree1swc.at(i).type != 3 && ori_tree1swc.at(i).type != 4)
        {
            if(ori_tree1swc.at(i).pn == -1) continue;//{suspoint_before.push_back(i);numofwrongtype++;}
            else {
                   int step_index=i;
                   while(ori_tree1swc.at(step_index).pn !=-1){

                       int index_of_pn=alln.indexOf(ori_tree1swc.at(step_index).pn);
                       step_index=index_of_pn;
                   }
                   if (find(suspoint_before.begin(),suspoint_before.end(),step_index)==suspoint_before.end()) {suspoint_before.push_back(step_index);numofwrongtype++;}
                 }
        }
    }
    cout<<"------------------suspoint_before.size:"<<suspoint_before.size()<<endl;
//find the dumplicated nodes which have diffrent types before sorting

    vector<int> all_index;QList<int>allj;
        for (int i=0;i<ori_tree1swc.size();i++)
        {
            if (i!=ori_tree1swc.size()-1 && allj.count(i) == 0 )
            {
              for (int j=i+1;j<ori_tree1swc.size();j++)
                {
                    float x1,x2,y1,y2,z1,z2;
                    int type1,type2,pars1,pars2,n1,n2;
                    x1=ori_tree1swc.at(i).x;x2=ori_tree1swc.at(j).x;
                    y1=ori_tree1swc.at(i).y;y2=ori_tree1swc.at(j).y;
                    z1=ori_tree1swc.at(i).z;z2=ori_tree1swc.at(j).z;
                    type1=ori_tree1swc.at(i).type;type2=ori_tree1swc.at(j).type;
                    pars1=ori_tree1swc.at(i).pn;pars2=ori_tree1swc.at(j).pn;
                    n1=ori_tree1swc.at(i).n;n2=ori_tree1swc.at(j).n;
                    if(x1==x2 && y1==y2 && z1==z2 && type1 != type2)
                    {
                        all_index.push_back(i);
                        allj.push_back(j);
                        numofduplicated++;
                    }}
            }}
        suspoint_before.insert(suspoint_before.end(),all_index.begin(),all_index.end());
// delete duplicated nodes with the same cordinates
        LocationSimple temp;
        for(int i=0;i<suspoint_before.size();i++)
           {
               if(suspoint_before.at(i)!=VOID && i!=suspoint_before.size()-1){
               temp.x=ori_tree1swc.at(suspoint_before.at(i)).x;
               temp.y=ori_tree1swc.at(suspoint_before.at(i)).y;
               temp.z=ori_tree1swc.at(suspoint_before.at(i)).z;
               for(int j=i+1;j<suspoint_before.size();j++){

                  if(suspoint_before.at(j)!=VOID && temp.x == ori_tree1swc.at(suspoint_before.at(j)).x &&
                          temp.y == ori_tree1swc.at(suspoint_before.at(j)).y && temp.z==ori_tree1swc.at(suspoint_before.at(j)).z)
                          suspoint_before.at(j)=VOID;

               }}}

        LocationSimple before_sort;
        for(int i=0;i<suspoint_before.size();i++)
           {
               if(suspoint_before.at(i)!=VOID){
               before_sort.x=ori_tree1swc.at(suspoint_before.at(i)).x;
               before_sort.y=ori_tree1swc.at(suspoint_before.at(i)).y;
               before_sort.z=ori_tree1swc.at(suspoint_before.at(i)).z;
               before_sort.color.r = 255;before_sort.color.g = 255;before_sort.color.b = 255;
               markerlist.push_back(before_sort);
             }
           }
           cout<<"------------------markerlist.size:"<<markerlist.size()<<endl;
           return;
}

void markerlist_after_sorting(QList<NeuronSWC>sorted_neuron,LandmarkList &markerlist,QVector<QVector<V3DLONG> > childs,int &numofwrongplace){

//find wrong types based on parent-child connection after sorting
                 QList<int>suspoint_after;
                 for (int i=0;i<sorted_neuron.size();i++)
                     {
                         if(childs[i].size()!=0)
                         {
                            for(int j=0;j<childs[i].size();j++)
                            {
                               if(sorted_neuron.at(i).type != sorted_neuron.at(childs[i].at(j)).type) {suspoint_after.push_back(i);numofwrongplace++;break;}
                            }
                         }
                     }
                 cout<<"----------------suspoint_after.size:"<<suspoint_after.size()<<endl;//2 wei
//delete duplicated input markers and set all markers back to terafly
                 QList<int> plist_sorted;
                 QList<int> alln_sorted;
                 int N_sorted=sorted_neuron.size();
                 for(int i=0; i<N_sorted; i++){
                     plist_sorted.append(sorted_neuron.at(i).pn);
                     alln_sorted.append(sorted_neuron.at(i).n);
                   }
                 float x1,y1,z1,x1_pn,y1_pn,z1_pn;int curr_pn;

                 if(suspoint_after.size()!=0)
                 {

                    for(int i=0;i<suspoint_after.size();i++)
                    {
                        if(suspoint_after.at(i)!=VOID){
                        int contain=suspoint_after.at(i);
                        x1=sorted_neuron.at(contain).x;y1=sorted_neuron.at(contain).y;z1=sorted_neuron.at(contain).z;
                        bool not_root=FALSE;

                        curr_pn=sorted_neuron.at(contain).pn;
                        if (curr_pn!=-1){

                           x1_pn=sorted_neuron.at(alln_sorted.indexOf(curr_pn)).x;
                           y1_pn=sorted_neuron.at(alln_sorted.indexOf(curr_pn)).y;
                           z1_pn=sorted_neuron.at(alln_sorted.indexOf(curr_pn)).z;
                           not_root=TRUE;
                           int pn_index=alln_sorted.indexOf(curr_pn);
                           if(suspoint_after.count(pn_index) != 0) suspoint_after[pn_index]=VOID;
                        }

                        if(markerlist.size()!=0)
                        {
                          for(int j=0;j<markerlist.size();j++){

                               bool check_pn=FALSE;bool check_curr=FALSE;
                               if(x1 == markerlist.at(j).x && y1 == markerlist.at(j).y && z1==markerlist.at(j).z) check_curr=TRUE;
                               if(not_root && x1_pn == markerlist.at(j).x && y1_pn == markerlist.at(j).y && z1_pn==markerlist.at(j).z) check_pn=TRUE;
                               if (check_curr || check_pn) {suspoint_after[i]=VOID;break;}
                           }

                        if(childs[contain].size()!=0){

                               for(int k=0;k<childs[contain].size();k++){

                                   for(int j=0;j<markerlist.size();j++){

                                       if(sorted_neuron.at(childs[contain].at(k)).x == markerlist.at(j).x  &&  sorted_neuron.at(childs[contain].at(k)).y == markerlist.at(j).y
                                          && sorted_neuron.at(childs[contain].at(k)).z==markerlist.at(j).z) {suspoint_after[i]=VOID;break;}
                           }}}
                        }}}}

                     LocationSimple m;
                     if(suspoint_after.size()!=0){
                       for(int i=0;i<suspoint_after.size();i++)
                        {
                           if (suspoint_after[i]!=VOID){
                            m.x=sorted_neuron.at(suspoint_after.at(i)).x;
                            m.y=sorted_neuron.at(suspoint_after.at(i)).y;
                            m.z=sorted_neuron.at(suspoint_after.at(i)).z;
                            m.color.r = 255;m.color.g = 255;m.color.b = 255;
                            markerlist.push_back(m);
                         }
                       }
                     }
                     return;
}

vector<NeuronSWC> loopDetection(V_NeuronSWC_list inputSegList)
{
	vector<NeuronSWC> outputErroneousPoints;
	outputErroneousPoints.clear();

	segList = inputSegList;
	seg2SegsMap.clear();
	segTail2segIDmap.clear();
	map<string, set<size_t> > wholeGrid2segIDmap;
	set<size_t> subtreeSegs;
	size_t segCount = 0;
	for (vector<V_NeuronSWC>::iterator segIt = segList.seg.begin(); segIt != segList.seg.end(); ++segIt)
	{
		for (vector<V_NeuronSWC_unit>::iterator nodeIt = segIt->row.begin(); nodeIt != segIt->row.end(); ++nodeIt)
		{
			int xLabel = nodeIt->x / GRID_LENGTH;
			int yLabel = nodeIt->y / GRID_LENGTH;
			int zLabel = nodeIt->z / GRID_LENGTH;
			QString gridKeyQ = QString::number(xLabel) + "_" + QString::number(yLabel) + "_" + QString::number(zLabel);
			string gridKey = gridKeyQ.toStdString();
			wholeGrid2segIDmap[gridKey].insert(size_t(segIt - segList.seg.begin()));
		}
		subtreeSegs.insert(segCount);
		++segCount;
	}

	for (set<size_t>::iterator it = subtreeSegs.begin(); it != subtreeSegs.end(); ++it)
	{
		//cout << *it << ":";
		set<size_t> connectedSegs;
		connectedSegs.clear();
		if (segList.seg[*it].row.size() <= 1)
		{
			segList.seg[*it].to_be_deleted = true;
			continue;
		}
		else if (segList.seg[*it].to_be_deleted) continue;

		for (vector<V_NeuronSWC_unit>::iterator nodeIt = segList.seg[*it].row.begin(); nodeIt != segList.seg[*it].row.end(); ++nodeIt)
		{
			int xLabel = nodeIt->x / GRID_LENGTH;
			int yLabel = nodeIt->y / GRID_LENGTH;
			int zLabel = nodeIt->z / GRID_LENGTH;
			QString gridKeyQ = QString::number(xLabel) + "_" + QString::number(yLabel) + "_" + QString::number(zLabel);
			string gridKey = gridKeyQ.toStdString();

			set<size_t> scannedSegs = wholeGrid2segIDmap[gridKey];
			if (!scannedSegs.empty())
			{
				for (set<size_t>::iterator scannedIt = scannedSegs.begin(); scannedIt != scannedSegs.end(); ++scannedIt)
				{
					if (*scannedIt == *it || segList.seg[*scannedIt].to_be_deleted) continue;
					if (segList.seg[*scannedIt].row.size() == 1) continue;

					if (segList.seg[*scannedIt].row.begin()->x == nodeIt->x && segList.seg[*scannedIt].row.begin()->y == nodeIt->y && segList.seg[*scannedIt].row.begin()->z == nodeIt->z)
					{
						connectedSegs.insert(*scannedIt);
						set<size_t> reversed;
						reversed.insert(*it);
						if (!seg2SegsMap.insert(pair<size_t, set<size_t> >(*scannedIt, reversed)).second) seg2SegsMap[*scannedIt].insert(*it);
						if (!segTail2segIDmap.insert(pair<size_t, set<size_t> >(*scannedIt, reversed)).second) segTail2segIDmap[*scannedIt].insert(*it);
					}
					else if ((segList.seg[*scannedIt].row.end() - 1)->x == nodeIt->x && (segList.seg[*scannedIt].row.end() - 1)->y == nodeIt->y && (segList.seg[*scannedIt].row.end() - 1)->z == nodeIt->z)
					{
						connectedSegs.insert(*scannedIt);
						set<size_t> reversed;
						reversed.insert(*it);
						if (!seg2SegsMap.insert(pair<size_t, set<size_t> >(*scannedIt, reversed)).second) seg2SegsMap[*scannedIt].insert(*it);
					}
				}
			}
		}
		if (!seg2SegsMap.insert(pair<size_t, set<size_t> >(*it, connectedSegs)).second)
		{
			for (set<size_t>::iterator otherSegIt = connectedSegs.begin(); otherSegIt != connectedSegs.end(); ++otherSegIt)
				seg2SegsMap[*it].insert(*otherSegIt);
		}
	}

//	cout << endl << "Starting loop detection.. " << endl;
	clock_t begin = clock();
	detectedLoopsSet.clear();
	finalizedLoopsSet.clear();
	nonLoopErrors.clear();
	rcCount = 0;
	testCount = 0;
	for (map<size_t, set<size_t> >::iterator it = seg2SegsMap.begin(); it != seg2SegsMap.end(); ++it)
	{
		if (it->second.size() <= 2) continue;

		vector<size_t> loops2ThisSeg;
		loops2ThisSeg.clear();

//		cout << "Starting segment: " << it->first << " ==> ";
		//for (set<size_t>::iterator seg2SegsIt = seg2SegsMap[it->first].begin(); seg2SegsIt != seg2SegsMap[it->first].end(); ++seg2SegsIt)
		//	cout << *seg2SegsIt << " ";
		//cout << endl;

		int loopCount = finalizedLoopsSet.size();

		rc_loopPathCheck(it->first, loops2ThisSeg);

//		if (finalizedLoopsSet.size() - loopCount == 0) cout << " -- no loops detected with this starting seg." << endl;
//		else cout << finalizedLoopsSet.size() - loopCount << " loops detected with seg " << it->first << endl << endl;
	}
	//cout << "rc count: " << rcCount << endl;
	//cout << "test count: " << testCount << endl;
	clock_t end = clock();
	float elapsed_secs = double(end - begin) / CLOCKS_PER_SEC;
	cout << "TIME ELAPSED: " << elapsed_secs << " SECS" << endl << endl;

	if (finalizedLoopsSet.empty()) return outputErroneousPoints;
	else
	{
		for (set<set<size_t> >::iterator loopIt = finalizedLoopsSet.begin(); loopIt != finalizedLoopsSet.end(); ++loopIt)
		{
			set<size_t> thisLoop = *loopIt;
			for (set<size_t>::iterator it = thisLoop.begin(); it != thisLoop.end(); ++it)
			{
				//cout << *it << " ";
				for (vector<V_NeuronSWC_unit>::iterator unitIt = segList.seg[*it].row.begin(); unitIt != segList.seg[*it].row.end(); ++unitIt)
				{
					unitIt->type = 15;				
					
					if (unitIt->parent == -1)
					{
						NeuronSWC problematicNode;
						problematicNode.x = unitIt->x;
						problematicNode.y = unitIt->y;
						problematicNode.z = unitIt->z;
						problematicNode.type = 15;
						problematicNode.parent = unitIt->parent;
						problematicNode.n = unitIt->n;
						outputErroneousPoints.push_back(problematicNode);
					}
				}
			}
			//cout << endl << endl;
		}
		cout << "LOOPS NUMBER (set): " << finalizedLoopsSet.size() << endl << endl;

		if (!nonLoopErrors.empty())
		{
			while (1)
			{
				for (set<set<size_t> >::iterator setCheckIt1 = nonLoopErrors.begin(); setCheckIt1 != nonLoopErrors.end(); ++setCheckIt1)
				{
					for (set<set<size_t> >::iterator setCheckIt2 = nonLoopErrors.begin(); setCheckIt2 != nonLoopErrors.end(); ++setCheckIt2)
					{
						if (setCheckIt1 == setCheckIt2) continue;
						else
						{
							int segNum = 0;
							for (set<size_t>::iterator segCheck1 = setCheckIt1->begin(); segCheck1 != setCheckIt1->end(); ++segCheck1)
								if (setCheckIt2->find(*segCheck1) != setCheckIt2->end()) ++segNum;

							if (segNum == setCheckIt1->size())
							{
								nonLoopErrors.erase(setCheckIt1);
								goto SET_ERASED;
							}
						}
					}
				}
				break;

			SET_ERASED:
				continue;
			}

			for (set<set<size_t> >::iterator loopIt = nonLoopErrors.begin(); loopIt != nonLoopErrors.end(); ++loopIt)
			{
				map<string, set<size_t> > headCountMap;
				map<string, set<size_t> > tailCountMap;
				bool head = true, tail = false;

				set<size_t> thisLoop = *loopIt;
				for (set<size_t>::iterator it = thisLoop.begin(); it != thisLoop.end(); ++it)
				{
					string headLabel = to_string((segList.seg[*it].row.end() - 1)->x) + " " + to_string((segList.seg[*it].row.end() - 1)->y) + " " + to_string((segList.seg[*it].row.end() - 1)->z);
					string tailLabel = to_string(segList.seg[*it].row.begin()->x) + " " + to_string(segList.seg[*it].row.begin()->y) + " " + to_string(segList.seg[*it].row.begin()->z);
					headCountMap[headLabel].insert(*it);
					tailCountMap[tailLabel].insert(*it);					
				}

				int overlapCount = 0;
				size_t segNum;
				for (map<string, set<size_t> >::iterator countIt = headCountMap.begin(); countIt != headCountMap.end(); ++countIt)
				{
					if (countIt->second.size() > overlapCount)
					{
						overlapCount = countIt->second.size();
						segNum = *countIt->second.begin();
					}
				}
				for (map<string, set<size_t> >::iterator countIt = tailCountMap.begin(); countIt != tailCountMap.end(); ++countIt)
				{
					if (countIt->second.size() > overlapCount)
					{
						overlapCount = countIt->second.size();
						segNum = *countIt->second.begin();
						head = false;
						tail = true;
					}
				}

				// Zhi: The following identifies the node that has the most duplication in 3 way fork situ. Please make changes you need here. 
				if (head) 
				{
					NeuronSWC problematicNode;
					problematicNode.x = (segList.seg[segNum].row.end() - 1)->x;
					problematicNode.y = (segList.seg[segNum].row.end() - 1)->y;
					problematicNode.z = (segList.seg[segNum].row.end() - 1)->z;
					problematicNode.type = 20;
					outputErroneousPoints.push_back(problematicNode);
//					cout << problematicNode.x << " " << problematicNode.y << " " << problematicNode.z << endl;
				}
				else if (tail)
				{
					NeuronSWC problematicNode;
					problematicNode.x = segList.seg[segNum].row.begin()->x;
					problematicNode.y = segList.seg[segNum].row.begin()->y;
					problematicNode.z = segList.seg[segNum].row.begin()->z;
					problematicNode.type = 20;
					outputErroneousPoints.push_back(problematicNode);
//					cout << problematicNode.x << " " << problematicNode.y << " " << problematicNode.z << endl;
				}	
			}
		}
	}

	return outputErroneousPoints;
}

void rc_loopPathCheck(size_t inputSegID, vector<size_t> curPathWalk)
{
	//++rcCount;

	if (seg2SegsMap[inputSegID].size() < 2)	return;
	
	//cout << "  input seg num: " << inputSegID << " ";
	curPathWalk.push_back(inputSegID);
	/*for (vector<size_t>::iterator curPathIt = curPathWalk.begin(); curPathIt != curPathWalk.end(); ++curPathIt)
	cout << *curPathIt << " ";
	cout << endl << endl;*/

	for (set<size_t>::iterator it = seg2SegsMap[inputSegID].begin(); it != seg2SegsMap[inputSegID].end(); ++it)
	{
		if (segTail2segIDmap.find(*it) == segTail2segIDmap.end())
		{	
			//++testCount;
			continue;
		}

		if (curPathWalk.size() >= 2 && *it == *(curPathWalk.end() - 2))
		{
			V_NeuronSWC_unit headUnit = *(segList.seg[*it].row.end() - 1);
			V_NeuronSWC_unit tailUnit = *segList.seg[*it].row.begin();

			bool headCheck = false, tailCheck = false;
			for (vector<V_NeuronSWC_unit>::iterator it = segList.seg[*(curPathWalk.end() - 1)].row.begin(); it != segList.seg[*(curPathWalk.end() - 1)].row.end(); ++it)
			{
				if (it->x == headUnit.x && it->y == headUnit.y && it->z == headUnit.z) headCheck = true;
				if (it->x == tailUnit.x && it->y == tailUnit.y && it->z == tailUnit.z) tailCheck = true;
			}

			if (headCheck == true && tailCheck == true)
			{
				set<size_t> detectedLoopPathSet;
				detectedLoopPathSet.clear();
				for (vector<size_t>::iterator loopIt = find(curPathWalk.begin(), curPathWalk.end(), *it); loopIt != curPathWalk.end(); ++loopIt)
					detectedLoopPathSet.insert(*loopIt);

				if (detectedLoopsSet.insert(detectedLoopPathSet).second)
				{
					nonLoopErrors.insert(detectedLoopPathSet);
					continue;
				}
				else return;
			}
			else continue;
		}

		if (find(curPathWalk.begin(), curPathWalk.end(), *it) == curPathWalk.end()) 
		{
			rc_loopPathCheck(*it, curPathWalk);
		}
		else
		{
			// a loop is found
			set<size_t> detectedLoopPathSet;
			detectedLoopPathSet.clear();
			for (vector<size_t>::iterator loopIt = find(curPathWalk.begin(), curPathWalk.end(), *it); loopIt != curPathWalk.end(); ++loopIt)
				detectedLoopPathSet.insert(*loopIt);

			if (detectedLoopsSet.insert(detectedLoopPathSet).second)
			{
				// pusedoloop by fork intersection check
//				cout << "pusedoloop check.." << endl;

				if (*(curPathWalk.end() - 3) == *it)
				{
					if (seg2SegsMap[*(curPathWalk.end() - 2)].find(*it) != seg2SegsMap[*(curPathWalk.end() - 2)].end())
					{
						vector<V_NeuronSWC_unit> forkCheck;
						forkCheck.push_back(*(segList.seg[*(curPathWalk.end() - 1)].row.end() - 1));
						forkCheck.push_back(*segList.seg[*(curPathWalk.end() - 1)].row.begin());
						forkCheck.push_back(*(segList.seg[*(curPathWalk.end() - 2)].row.end() - 1));
						forkCheck.push_back(*segList.seg[*(curPathWalk.end() - 2)].row.begin());

						int headConnectedCount = 0;
						for (vector<V_NeuronSWC_unit>::iterator checkIt = forkCheck.begin(); checkIt != forkCheck.end(); ++checkIt)
						{
							if (checkIt->x == (segList.seg[*it].row.end() - 1)->x && checkIt->y == (segList.seg[*it].row.end() - 1)->y && checkIt->z == (segList.seg[*it].row.end() - 1)->z)
								++headConnectedCount;
						}

						int tailConnectedCount = 0;
						for (vector<V_NeuronSWC_unit>::iterator checkIt = forkCheck.begin(); checkIt != forkCheck.end(); ++checkIt)
						{
							if (checkIt->x == segList.seg[*it].row.begin()->x && checkIt->y == segList.seg[*it].row.begin()->y && checkIt->z == segList.seg[*it].row.begin()->z)
								++tailConnectedCount;
						}

						if (!(headConnectedCount == 1 && tailConnectedCount == 1))
						{
                            cout << "  -> 3 seg intersection detected, exluded from loop candidates. (" << *it << ") ";
                            for (set<size_t>::iterator thisLoopIt = detectedLoopPathSet.begin(); thisLoopIt != detectedLoopPathSet.end(); ++thisLoopIt)
								cout << *thisLoopIt << " ";
							cout << endl;
							continue;
						}
						else
						{
							finalizedLoopsSet.insert(detectedLoopPathSet);
                            cout << "  Loop from 3 way detected ----> (" << *it << ") ";
                            for (set<size_t>::iterator thisLoopIt = detectedLoopPathSet.begin(); thisLoopIt != detectedLoopPathSet.end(); ++thisLoopIt)
								cout << *thisLoopIt << " ";
							cout << endl;
							return;
						}
					}
				}
				else if (curPathWalk.size() == 4)
				{
					if ((*curPathWalk.end() - 4) == *it)
					{
						if (seg2SegsMap[*(curPathWalk.end() - 2)].find(*it) != seg2SegsMap[*(curPathWalk.end() - 2)].end() &&
							seg2SegsMap[*(curPathWalk.end() - 3)].find(*it) != seg2SegsMap[*(curPathWalk.end() - 3)].end())
						{
							if (seg2SegsMap[*(curPathWalk.end() - 2)].find(*(curPathWalk.end() - 1)) != seg2SegsMap[*(curPathWalk.end() - 2)].end() &&
								seg2SegsMap[*(curPathWalk.end() - 3)].find(*(curPathWalk.end() - 2)) != seg2SegsMap[*(curPathWalk.end() - 3)].end() &&
								seg2SegsMap[*(curPathWalk.end() - 4)].find(*(curPathWalk.end() - 3)) != seg2SegsMap[*(curPathWalk.end() - 4)].end())
							{
								cout << "  -> 4 seg intersection detected, exluded from loop candidates. (" << *it << ")" << endl;
								continue;
							}
						}
					}
				}
				else
				{
					finalizedLoopsSet.insert(detectedLoopPathSet);
					cout << "  Loop detected ----> (" << *it << ") ";
					for (set<size_t>::iterator thisLoopIt = detectedLoopPathSet.begin(); thisLoopIt != detectedLoopPathSet.end(); ++thisLoopIt)
						cout << *thisLoopIt << " ";
					cout << endl << endl;

					while (1)
					{
						for (set<set<size_t> >::iterator setCheckIt1 = finalizedLoopsSet.begin(); setCheckIt1 != finalizedLoopsSet.end(); ++setCheckIt1)
						{
							for (set<set<size_t> >::iterator setCheckIt2 = finalizedLoopsSet.begin(); setCheckIt2 != finalizedLoopsSet.end(); ++setCheckIt2)
							{
								if (setCheckIt1 == setCheckIt2) continue;
								else
								{
									int segNum = 0;
									for (set<size_t>::iterator segCheck1 = setCheckIt1->begin(); segCheck1 != setCheckIt1->end(); ++segCheck1)
										if (setCheckIt2->find(*segCheck1) != setCheckIt2->end()) ++segNum;

									if (segNum == setCheckIt1->size())
									{
										finalizedLoopsSet.erase(setCheckIt1);
										goto SET_ERASED;
									}
								}
							}
						}
						break;

					SET_ERASED:
						continue;
					}
				}
			}
			else return;
		}
	}

	curPathWalk.pop_back();
	//cout << endl;
}

vector<V_NeuronSWC_list> showConnectedSegs(const V_NeuronSWC_list& inputSegList)
{
	V_NeuronSWC_list inputList = inputSegList;
	vector<V_NeuronSWC_list> outputTreeList;
	set<size_t> singleTreeSegs;
	vector<size_t> singleTreeSegsVec;

	while (inputList.seg.size() > 0)
	{
		singleTreeSegsVec.clear();
		singleTreeSegs.clear();
		singleTreeSegs.insert(0);

		segEnd2segIDmap.clear();
		for (vector<V_NeuronSWC>::iterator it = inputList.seg.begin(); it != inputList.seg.end(); ++it)
		{
			double xLabelTail = it->row.begin()->x;
			double yLabelTail = it->row.begin()->y;
			double zLabelTail = it->row.begin()->z;
			double xLabelHead = (it->row.end() - 1)->x;
			double yLabelHead = (it->row.end() - 1)->y;
			double zLabelHead = (it->row.end() - 1)->z;
			QString key1Q = QString::number(xLabelTail) + "_" + QString::number(yLabelTail) + "_" + QString::number(zLabelTail);
			string key1 = key1Q.toStdString();
			QString key2Q = QString::number(xLabelHead) + "_" + QString::number(yLabelHead) + "_" + QString::number(zLabelHead);
			string key2 = key2Q.toStdString();

			segEnd2segIDmap.insert(pair<string, size_t>(key1, size_t(it - inputList.seg.begin())));
			segEnd2segIDmap.insert(pair<string, size_t>(key2, size_t(it - inputList.seg.begin())));
		}

		rc_findConnectedSegs(inputList, singleTreeSegs, 0, segEnd2segIDmap);
		singleTreeSegsVec.insert(singleTreeSegsVec.begin(), singleTreeSegs.begin(), singleTreeSegs.end());
		sort(singleTreeSegsVec.rbegin(), singleTreeSegsVec.rend());
		V_NeuronSWC_list currTreeSegs;
		for (vector<size_t>::iterator segIt = singleTreeSegsVec.begin(); segIt != singleTreeSegsVec.end(); ++segIt)
		{
			vector<V_NeuronSWC_unit> currSegUnits;
			for (vector<V_NeuronSWC_unit>::iterator unitIt = inputList.seg[*segIt].row.begin(); unitIt != inputList.seg[*segIt].row.end(); ++unitIt)
				currSegUnits.push_back(*unitIt);
			V_NeuronSWC currSeg;
			currSeg.row = currSegUnits;
			currTreeSegs.seg.push_back(currSeg);
			inputList.seg.erase(inputList.seg.begin() + *segIt);
		}

		outputTreeList.push_back(currTreeSegs);
		cout << "number of segs in this tree: " << currTreeSegs.seg.size() << endl;
	}

	return outputTreeList;
}

void rc_findConnectedSegs(V_NeuronSWC_list& inputSegList, set<size_t>& singleTreeSegs, size_t inputSegID, multimap<string, size_t>& segEnd2segIDmap)
{
	size_t curSegNum = singleTreeSegs.size();

	/* --------- Find segments that are connected in the middle of input segment --------- */
	if (inputSegList.seg[inputSegID].row.size() > 2)
	{
		for (vector<V_NeuronSWC_unit>::iterator unitIt = inputSegList.seg[inputSegID].row.begin() + 1; unitIt != inputSegList.seg[inputSegID].row.end() - 1; ++unitIt)
		{
			double middleX = unitIt->x;
			double middleY = unitIt->y;
			double middleZ = unitIt->z;
			QString middleNodeKeyQ = QString::number(middleX) + "_" + QString::number(middleY) + "_" + QString::number(middleZ);
			string middleNodeKey = middleNodeKeyQ.toStdString();

			pair<multimap<string, size_t>::iterator, multimap<string, size_t>::iterator> middleRange = segEnd2segIDmap.equal_range(middleNodeKey);
			for (multimap<string, size_t>::iterator middleIt = middleRange.first; middleIt != middleRange.second; ++middleIt)
			{
				if (middleIt->second == inputSegID) continue;
				else if (singleTreeSegs.find(middleIt->second) != singleTreeSegs.end())
				{
					//cout << "  --> already picked, move to the next." << endl;
					continue;
				}
				else if (middleIt->first == middleNodeKey)
				{
					//cout << "  Found a segment in the middle of the route, adding it to the recursive searching process:" << middleNodeKey << " " << middleIt->second << endl;
					if (inputSegList.seg[middleIt->second].to_be_deleted) continue;
					singleTreeSegs.insert(middleIt->second);
					rc_findConnectedSegs(inputSegList, singleTreeSegs, middleIt->second, segEnd2segIDmap);
				}
			}
		}
	}
	/* ------- END of [Find segments that are connected in the middle of input segment] ------- */

	/* --------- Find segments that are connected to the head or tail of input segment --------- */
	set<size_t> curSegEndRegionSegs;
	curSegEndRegionSegs.clear();
	curSegEndRegionSegs = segEndRegionCheck(inputSegList, inputSegID);
	//cout << curSegEndRegionSegs.size() << endl;
	if (!curSegEndRegionSegs.empty())
	{
		for (set<size_t>::iterator regionSegIt = curSegEndRegionSegs.begin(); regionSegIt != curSegEndRegionSegs.end(); ++regionSegIt)
		{
			if (*regionSegIt == inputSegID) continue;
			else if (singleTreeSegs.find(*regionSegIt) != singleTreeSegs.end())
			{
				//cout << "  --> already picked, move to the next." << endl;
				continue;
			}
			else
			{
				//cout << "    ==> segs at the end region added:" << *regionSegIt << endl;
				if (inputSegList.seg[*regionSegIt].to_be_deleted) continue;

				singleTreeSegs.insert(*regionSegIt);
				rc_findConnectedSegs(inputSegList, singleTreeSegs, *regionSegIt, segEnd2segIDmap);
			}
		}
	}
	/* ------- END of [Find segments that are connected to the head or tail of input segment] ------- */

	if (singleTreeSegs.size() == curSegNum) return;
}

set<size_t> segEndRegionCheck(V_NeuronSWC_list& inputSegList, size_t inputSegID)
{
	set<size_t> otherConnectedSegs;
	otherConnectedSegs.clear();

	map<string, set<size_t> > wholeGrid2segIDmap;
	set<size_t> subtreeSegs;
	size_t segCount = 0;
	for (vector<V_NeuronSWC>::iterator segIt = inputSegList.seg.begin(); segIt != inputSegList.seg.end(); ++segIt)
	{
		for (vector<V_NeuronSWC_unit>::iterator nodeIt = segIt->row.begin(); nodeIt != segIt->row.end(); ++nodeIt)
		{
			int xLabel = nodeIt->x / GRID_LENGTH;
			int yLabel = nodeIt->y / GRID_LENGTH;
			int zLabel = nodeIt->z / GRID_LENGTH;
			QString gridKeyQ = QString::number(xLabel) + "_" + QString::number(yLabel) + "_" + QString::number(zLabel);
			string gridKey = gridKeyQ.toStdString();
			wholeGrid2segIDmap[gridKey].insert(size_t(segIt - inputSegList.seg.begin()));
		}
		subtreeSegs.insert(segCount);
		++segCount;
	}

	int xHead = (inputSegList.seg[inputSegID].row.end() - 1)->x / GRID_LENGTH;
	int yHead = (inputSegList.seg[inputSegID].row.end() - 1)->y / GRID_LENGTH;
	int zHead = (inputSegList.seg[inputSegID].row.end() - 1)->z / GRID_LENGTH;
	int xTail = inputSegList.seg[inputSegID].row.begin()->x / GRID_LENGTH;
	int yTail = inputSegList.seg[inputSegID].row.begin()->y / GRID_LENGTH;
	int zTail = inputSegList.seg[inputSegID].row.begin()->z / GRID_LENGTH;
	QString gridKeyHeadQ = QString::number(xHead) + "_" + QString::number(yHead) + "_" + QString::number(zHead);
	string gridKeyHead = gridKeyHeadQ.toStdString();
	QString gridKeyTailQ = QString::number(xTail) + "_" + QString::number(yTail) + "_" + QString::number(zTail);
	string gridKeyTail = gridKeyTailQ.toStdString();

	set<size_t> headRegionSegs = wholeGrid2segIDmap[gridKeyHead];
	set<size_t> tailRegionSegs = wholeGrid2segIDmap[gridKeyTail];

	//cout << " Head region segs:";
	for (set<size_t>::iterator headIt = headRegionSegs.begin(); headIt != headRegionSegs.end(); ++headIt)
	{
		if (*headIt == inputSegID || inputSegList.seg[*headIt].to_be_deleted) continue;
		//cout << *headIt << " ";
		for (vector<V_NeuronSWC_unit>::iterator nodeIt = inputSegList.seg[*headIt].row.begin(); nodeIt != inputSegList.seg[*headIt].row.end(); ++nodeIt)
		{
			if (nodeIt->x == (inputSegList.seg[inputSegID].row.end() - 1)->x && nodeIt->y == (inputSegList.seg[inputSegID].row.end() - 1)->y && nodeIt->z == (inputSegList.seg[inputSegID].row.end() - 1)->z)
				otherConnectedSegs.insert(*headIt);
		}
	}
	//cout << endl << " Tail region segs:";
	for (set<size_t>::iterator tailIt = tailRegionSegs.begin(); tailIt != tailRegionSegs.end(); ++tailIt)
	{
		if (*tailIt == inputSegID || inputSegList.seg[*tailIt].to_be_deleted) continue;
		//cout << *tailIt << " ";
		for (vector<V_NeuronSWC_unit>::iterator nodeIt = inputSegList.seg[*tailIt].row.begin(); nodeIt != inputSegList.seg[*tailIt].row.end(); ++nodeIt)
		{
			if (nodeIt->x == inputSegList.seg[inputSegID].row.begin()->x && nodeIt->y == inputSegList.seg[inputSegID].row.begin()->y && nodeIt->z == inputSegList.seg[inputSegID].row.begin()->z)
				otherConnectedSegs.insert(*tailIt);
		}
	}
	//cout << endl;

	return otherConnectedSegs;
}
