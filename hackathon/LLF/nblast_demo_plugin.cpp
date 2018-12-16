/* nblast_demo_plugin.cpp
 * The main file of plugin nblast_demo
 * 2018-10-25 by Longfei Li.
 */

#include "nblast_demo_plugin.h"
#include <QtGlobal>

Q_EXPORT_PLUGIN2(nblast_demo, NblastDemoPlugin);


QStringList NblastDemoPlugin::menulist() const
{
	return QStringList()
		<< tr("pre_processing")
		<< tr("global_retrieve")
		<< tr("swcs2ano")
		<< tr("count_accuracy")
		<< tr("about");
}

QStringList NblastDemoPlugin::funclist() const
{
	return QStringList()
		<< tr("help");
}

void NblastDemoPlugin::domenu(const QString &menu_name, V3DPluginCallback2 &callback, QWidget *parent)
{
	if (menu_name == tr("pre_processing"))
	{
		pre_processing();
	}
	else if (menu_name == tr("global_retrieve"))
	{
		global_retrieve();
	}
	else if (menu_name == tr("swcs2ano"))
	{
		swcs2ano();
	}
	else if (menu_name == tr("count_accuracy"))
	{
		count_accuracy();
	}
	else if (menu_name == tr("about"))
	{
		v3d_msg(tr("wooo!"));
	}
}

bool NblastDemoPlugin::dofunc(const QString & func_name, const V3DPluginArgList & input, V3DPluginArgList & output, V3DPluginCallback2 & callback, QWidget * parent)
{
	if (func_name == tr("help"))
	{
		printHelp();
		return true;
	}
	return false;
}

void printHelp() 
{
	cout << "We are in dofunc->printHelp() now." << endl;

	return;
}

/************************************************************************/
/* pre processing part                                                  */
/************************************************************************/

//pre processing main
void pre_processing()
{
	NeuronTree neuron;//contain a NeuronSWC list.
	NeuronSWC *p_cur = 0;//a NeuronSWC is a neuron node.

	QStringList filenames = QFileDialog::getOpenFileNames(0, 0, "", "Supported file (*.swc)" ";;Neuron structure(*.swc)", 0, 0);
	
	if (filenames.isEmpty())
	{
		v3d_msg("You don't open any SWC file.");
		return;
	}


	for (V3DLONG i = 0; i < filenames.size(); i++)
	{
		//v3d_msg(filenames[i]);
		neuron = readSWC_file(filenames[i]);

		/************************************************************************/
		/* begin prunning                                                       */
		/************************************************************************/
		NeuronTree neuron_pruned;
		prune_branch(neuron, neuron_pruned);

		QString outfilename_pruned = filenames[i] + "_pruned.swc";
		save_neuron_tree(outfilename_pruned, neuron_pruned);

		v3d_msg("prune finish.");
		cout<<"prune finish."<<endl;
		
		/************************************************************************/
		/* begin resampling                                                     */
		/************************************************************************/
		NeuronTree neuron_resampled;
		double resample_distance = 100;//100 is alright~
		resample(neuron_pruned, resample_distance, neuron_resampled);

		//out put
		QString outfileName_resampled = filenames[i] + "_reampled.swc";
		save_neuron_tree(outfileName_resampled,neuron_resampled);
		
		v3d_msg("resampling finish.");

	}
}

//prune_branch in blastneuron way
bool prune_branch(NeuronTree nt, NeuronTree& nt_pruned) 
{
	V3DLONG siz = nt.listNeuron.size();
	vector<V3DLONG> branches(siz, 0);//record branches
	for (V3DLONG i = 0; i < siz; i++)
	{
		if (nt.listNeuron[i].pn < 0) continue;//root
		V3DLONG pid = nt.hashNeuron.value(nt.listNeuron[i].pn);//hash:node no.to list no.
		branches[pid]++;
	}

	vector<vector<double> > longest_path(siz, vector<double>(2, 0));//record the length of path which includes the node																	
	vector<V3DLONG> chd(siz, -1);//immediate child of the current longest path
	//calculate the longest diameter(add two longest path in one node)
	for (V3DLONG i = 0; i < siz; i++)
	{
		if (branches[i] != 0) continue;//when branches is 0,that means end of a path
		V3DLONG cur = i;//current no.
		V3DLONG pid;//parent id in list
		//going back to root to calculate the longest path
		while(cur!=0)
		{
			//cout << "calculate at point " << cur << endl;
			NeuronSWC s = nt.listNeuron[cur];
			pid = nt.hashNeuron.value(s.pn);//for parrent_id;
			double dist = DIST(s, nt.listNeuron[pid]) + longest_path[cur][0];//dis of s and it's parent + the longest length from s to the end;
			if (dist > longest_path[pid][0])//bigger than parent longest means another path;
			{
				chd[pid] = cur;
				longest_path[pid][0] = dist;//update
			}
			else if (dist > longest_path[pid][1] && chd[pid] != cur)//next longest
				longest_path[pid][1] = dist;
			cur = pid;//going back
			if(branches[cur]==0)
				break;
		}
	}

	double diameter = -1;//get the diameter for prune
	for (V3DLONG i = 0; i < siz; i++)
	{
		if (longest_path[i][0] + longest_path[i][1] > diameter)
			diameter = longest_path[i][0] + longest_path[i][1];
	}

	double prune_size = diameter * 0.05;//prune those smaller than longest's 5%
	vector<bool> to_prune(siz, false);
	for (V3DLONG i = 0; i < siz; i++)
	{
		if (branches[i] != 0) continue;//get end points
		V3DLONG cur = i;
		V3DLONG pid;
		vector<V3DLONG> segments;
		double edge_length = 0;
		while (cur!=0)
		{
			//cout << "prune at point " << cur << endl;
			NeuronSWC s = nt.listNeuron[cur];
			segments.push_back(cur);
			pid = nt.hashNeuron.value(s.pn);
			edge_length += DIST(s, nt.listNeuron[pid]);
			cur = pid;
			if (branches[pid]!=1)
				break;
		}
		if (edge_length < prune_size) 
		{
			for (int j = 0; j < segments.size(); j++)
			{
				to_prune[segments[j]] = true;
			}
		}
	}

	nt_pruned.listNeuron.clear();
	nt_pruned.hashNeuron.clear();
	for (V3DLONG i = 0; i < siz; i++)
	{
		if (!to_prune[i])
		{
			NeuronSWC s = nt.listNeuron[i];
			nt_pruned.listNeuron.append(s);
			nt_pruned.hashNeuron.insert(nt.listNeuron[i].n, nt_pruned.listNeuron.size() - 1);
		}
	}
	
	return true;
}	

//resample on a neuronTree by a resample_distance
void resample(NeuronTree &neuron_pruned, double resample_distance, NeuronTree &resampled)
{
	V3DLONG siz = neuron_pruned.listNeuron.size();
	Tree tree;//get the neurontree
	//construct every point
	for(V3DLONG i=0;i<siz;i++)
	{
		NeuronSWC s= neuron_pruned.listNeuron[i];
		Point* p = new Point;
		p->x = s.x;
		p->y = s.y;
		p->z = s.z;
		p->r = s.r;
		p->type = s.type;
		p->p = NULL;
		p->childNum = 0;
		tree.push_back(p);
	}
	cout<<"points constructed."<<endl;
	//update for parent or child information
	for (V3DLONG i=0;i<siz;i++)
	{
		if (neuron_pruned.listNeuron[i].pn<0) 
			continue;
		V3DLONG pid = neuron_pruned.hashNeuron.value(neuron_pruned.listNeuron[i].pn);	
		tree[i]->p = tree[pid];
		tree[pid]->childNum++;
	}
	cout<<"updating parent info over."<<endl;
	//construct segments
	vector<Segment*> segment_list;
	for(V3DLONG i=0;i<siz;i++)
	{
		if (tree[i]->childNum!=1)//means an end point or branch point
		{
			Segment* seg = new Segment;
			Point* cur = tree[i];
			seg->push_back(cur);
			cur = cur->p;
			while(cur && cur->childNum==1)
			{
				seg->push_back(cur);
				cur = cur->p;
			}
			segment_list.push_back(seg);
		}
	}
	cout<<"segments constructed."<<endl;
	//resample at every segment
	for (V3DLONG i=0;i<segment_list.size();i++)
	{
		//cout<<"resample at segment no."<<i<<endl;
		resample_segment(segment_list[i],resample_distance);
	}
	cout<<"resample over."<<endl;
	//get a new tree;
	tree.clear();
	map<Point*, V3DLONG> index_map;
	for(V3DLONG i=0;i<segment_list.size();i++)
	{
		for(V3DLONG j=0;j<segment_list[i]->size();j++)
		{
			tree.push_back(segment_list[i]->at(j));
			index_map.insert(pair<Point*, V3DLONG>(segment_list[i]->at(j),tree.size()));
		}
	}
	cout<<"new tree constructed"<<endl;
	//from the new tree construct NeuronTree
	for (V3DLONG i=0;i<tree.size();i++)
	{
		NeuronSWC s;
		Point* p = tree[i];
		s.n = i+1;//no.
		if(p->p==NULL) s.pn = -1;
		else
			s.pn = index_map[p->p];
		s.x = p->x;
		s.y = p->y;
		s.z = p->z;
		s.r = p->r;
		s.type = p->type;

		resampled.listNeuron.push_back(s);
		resampled.hashNeuron.insert(resampled.listNeuron[i].n, i);
	}
	cout<<"new NeuronTree constructed."<<endl;
	//free place
	for (V3DLONG i=0;i<tree.size();i++)
	{
		if (tree[i]) {delete tree[i]; tree[i]=NULL;}
	}
	for (V3DLONG j=0;j<segment_list.size();j++)
	{
		if (segment_list[j]) {delete segment_list[j]; segment_list[j] = NULL;}
	}
	cout<<"free place."<<endl;
}

//resample from a segment get one resample point every resample_distance
void resample_segment(Segment* seg,double resample_distance)
{
	Segment seg_r;//a segment is a vector of Points
	Point* start = seg->at(0);//segment end
	Point* seg_parent = seg->back()->p;//segment's parent

	seg_r.push_back(start);

	V3DLONG count = 0;
	double path_length=0;

	while (count < seg->size() && start && start->p)
	{
		//double dis = DISTP(start,start->p);
		//cout<<dis<<endl;
		path_length += DISTP(start,start->p);
		//if segment length < res threshhold, don't need resample
		if(path_length<=seg_r.size()*resample_distance)
		{
			start = start->p;
			count++;
		}
		else//else need to resample
		{
			path_length -= DISTP(start,start->p);
			Point* pt = new Point;
			//the rate so that won't exceed threshhold
			double rate = (seg_r.size()*resample_distance-path_length)/DISTP(start,start->p);
			pt->x = start->x + rate*(start->p->x-start->x);
			pt->y = start->y + rate*(start->p->y-start->y);
			pt->z = start->z + rate*(start->p->z-start->z);
			pt->r = start->r*(1-rate) + start->p->r*rate;//intepolate the radius
			pt->type = start->type;
			pt->p = start->p;
			seg_r.back()->p = pt;
			seg_r.push_back(pt);
			path_length += DISTP(start,pt);
			start = pt;
		}
	}
	seg_r.back()->p = seg->back()->p;
	*seg = seg_r;

}

void save_neuron_tree(QString outfileName, NeuronTree &neuron)
{
	FILE* fp;
	fp = fopen(qPrintable(outfileName), "w");
	fprintf(fp, "##n,type,x,y,z,radius,parent\n");
	for (int i = 0; i < neuron.listNeuron.size(); i++)
	{
		NeuronSWC curr = neuron.listNeuron.at(i);
		fprintf(fp, "%d %d %.2f %.2f %.2f %.2f %d\n", curr.n, curr.type, curr.x, curr.y, curr.z, curr.r, curr.pn);
	}
	fclose(fp);
}

/************************************************************************/
/* gloal retrieve part                                                  */
/************************************************************************/

//global retrieve main
void global_retrieve()
{
	QStringList name_list;//record neuron's name
	QList<double*> features_list;
	QString database_path = "C:/Users/Administrator/Desktop/database.txt";
	//load all features form database
	load_features(database_path,features_list, name_list);
	
	V3DLONG neuron_num = name_list.size();
	int *similarity_index = new int[neuron_num];//record the index of max similar neurons
	double *similarity_score = new double[neuron_num];//record similiarity
	QString query_neuron_path = "C:/Users/Administrator/Desktop/query.txt";//query neuron's file path
	//QString query_neuron = QFileDialog::getOpenFileNames(0, 0, "", "Supported file (*.txt)" ";;Neuron features(*.txt)", 0, 0)[0];
	//NeuronTree query_neuron = readSWC_file(query_neuron_path);
	vector<V3DLONG> sorted_index_vec;//record the most similar's index
	
	//retrieve the similar neurons with query by compare query's feature with feature_list
	neuron_retrieve(query_neuron_path, features_list, sorted_index_vec);
	const char* output_path = "C:/Users/Administrator/Desktop/output.txt";
	print_result(sorted_index_vec, output_path, name_list, qPrintable(database_path), qPrintable(query_neuron_path));//output result

	v3d_msg("retrieving finish.");
}

//load feratures
void load_features(QString database_path,QList<double*> &features_list, QStringList &name_list)
{
	cout<<"loading features."<<endl;
	name_list.clear();
	features_list.clear();
	QFile database_file(database_path);
	if (!database_file.open(QIODevice::ReadOnly | QIODevice::Text))
	{
		cout<<"fail to open file "<<qPrintable(database_path)<<endl;
		return;
	}
	QTextStream in(&database_file);
	QString line;
	QStringList string_features;
	line = in.readLine();
	while(!line.isNull())
	{		
		name_list.append(line.split("\t")[0]);//name
		double* features = new double[FEATURE_NUM];//feature string to double
		for(int i=0;i<FEATURE_NUM;i++)
		{
			string_features = line.split("\t");//features
			features[i] = string_features[2].toDouble();
			//cout<<features[i]<<endl;
			line = in.readLine();
		}
		features_list.append(features);
		
	}
	cout<<features_list.size()<<endl;
}

void load_features(QString feature_path,double* features)
{
	QFile database_file(feature_path);
	if (!database_file.open(QIODevice::ReadOnly | QIODevice::Text))
	{
		cout<<"fail to open file "<<qPrintable(feature_path)<<endl;
		return;
	}
	QTextStream in(&database_file);
	QString line;
	QStringList string_features;
	line = in.readLine();
	for(int i=0;i<FEATURE_NUM;i++)
	{
		string_features = line.split("\t");//features
		features[i] = string_features[2].toDouble();
		line = in.readLine();
	}
}
//
void neuron_retrieve(QString query_neuron, QList<double*> &features_list, vector<V3DLONG> &sorted_index_vec)
{
	cout<<"retrieve begins."<<endl;
	int neuron_num = features_list.size();
//	NeuronTree query_preprocessed = pre_process(query_neuron);
	double *features;
	load_features(query_neuron,features);
	//compute_features(query_neuron, features);//get features from query neuron
	
	int *similarity_top_index = new int[5];
	double* similarity_top_score;
	int top_similar_num = 5;

	//use feature compare with feature_list get a similarity sequence(decrease order)
	retrieve_similarity(features,features_list,similarity_top_index,similarity_top_score,top_similar_num);
	for(int i=0;i<top_similar_num;i++)
	{
		sorted_index_vec.push_back(similarity_top_index[i]);
		cout<<similarity_top_index[i]<<endl;
	}
	cout<<"retrieve finish."<<endl;

 	for (V3DLONG i=0;i<neuron_num;i++)
 	{
		if (features_list[i]) {delete []features_list[i]; features_list[i]=NULL;}
 	}
}

//calculate similarity
void retrieve_similarity(double *features, QList<double*> &features_list,int *similarity_top_index,double *similarity_top_score,int top_similar_num)
{
	cout<<"calculating similarity with the query one."<<endl;
	int neuron_num = features_list.size();
	double* similarity_score = new double[neuron_num];
	//for normalization, calculate the max and min of features
	QList<double> min_feature_list;
	QList<double> max_feature_list;
	for (int i=0;i<FEATURE_NUM;i++)
	{
		double min_f =  -VOID;
		double max_f = VOID;
		for(int j=0;j<neuron_num;j++)
		{
			double* cur_features = features_list.at(j);
			if(min_f>cur_features[i]) 
				min_f=cur_features[i];
			if(max_f<cur_features[i]) 
				max_f=cur_features[i];
		}
		min_feature_list.push_back(min_f);
		max_feature_list.push_back(max_f);
	}

	for (int i=0;i<neuron_num;i++)
	{
		similarity_score[i] = 0;
		double* cur_features = features_list.at(i);
		for (int j=0;j<FEATURE_NUM;j++)
		{
			double df=max_feature_list[j]-min_feature_list[j];
			if(df==0) df=1;//don't divide by 0
			similarity_score[i] += ((cur_features[j] - features[j])/df) * ((cur_features[j] - features[j])/df);
			
		}
		//the bigger the similarer
		similarity_score[i] = -sqrt(similarity_score[i]);
	}

	//sort and make index
	int position;
	for (int i=0;i<top_similar_num;i++)
	{
		similarity_top_index[i] = -1;
		similarity_top_score[i] = VOID;
	}
	for (int i=0;i<neuron_num;i++)
	{
		position = bin_search(similarity_top_score,similarity_score[i],0,top_similar_num-1);
		if(position!=VOID)
		{
			//move the right part to insert
			for (int j=top_similar_num-1;j>position;j--)
			{
				similarity_top_score[j] = similarity_top_score[j-1];
				similarity_top_index[j] = similarity_top_index[j-1];
			}
			similarity_top_score[position] = similarity_score[i];
			similarity_top_index[position] = i;
		}
	}

	if (similarity_score)
	{
		delete []similarity_score;similarity_score=NULL;
	}

}

//get the position of a value in the max list(decrease order)
int bin_search(double* list, double v, int start, int end)
{
	if (v<list[end]) return VOID;//too small
	if (v>list[start]) return 0;
	if (end==start+1) return end;
	if (end<=start) return start;

	int mid = ((end+start)%2==0)?((end+start)/2):((end+start+1)/2);
	if (v<=list[mid])
		return bin_search(list, v, mid, end);
	else
		return bin_search(list, v, start, mid);
}

//print result
void print_result(vector<V3DLONG> result, const char* output_path, QStringList name_list, const char* database_name, const char* query_neuron_name)
{
	cout<<"print result begin"<<endl;
	FILE * fp;
	fp = fopen(output_path, "w");
	if (fp==NULL)
	{
		cout<<"ERROR! can't open file "<<output_path<<endl;
	}
	fprintf(fp,"#database:          %s\n",database_name);
	fprintf(fp,"#query neuron:      %s\n",query_neuron_name);
	fprintf(fp,"#num of candidates: %d\n",result.size());
	for (int i=0;i<result.size();i++)
	{
		cout<<result[i]<<endl;
		fprintf(fp,qPrintable(name_list[result[i]]));
		fprintf(fp,"\n");
	}
	cout<<"printed result in file "<<output_path<<endl;
	fclose(fp);
}

//a tool for making swcs to ano file
void swcs2ano()
{
	//QStringList swc_file_list = QFileDialog::getOpenFileNames(0, 0, "", "Supported file (*.swc)" ";;Neuron structure(*.swc)", 0, 0);
	//QString path = "C:/Users/Administrator/Desktop/database/no-kenyon/CNG version";
	QString path = "C:/Users/Administrator/Desktop/test_projection/database";
	QFileInfoList file_info_list = GetFileInfoList(path);
	cout<<file_info_list.size()<<endl;
	QStringList swc_file_list;
	for (int i = 0; i < file_info_list.size(); ++i) 
	{
		swc_file_list.push_back(file_info_list.at(i).absoluteFilePath());
	}
	QString  output_ano_file = "C:/Users/Administrator/Desktop/test_projection/database.ano";
	//QString  output_ano_file = "C:/Users/Administrator/Desktop/database.ano";
	P_ObjectFileType fnList;
	fnList.swc_file_list = swc_file_list;
	saveAnoFile(output_ano_file,fnList);
	
	v3d_msg("transform over.");

}

//Get files in a dir
QFileInfoList GetFileInfoList(QString path)
{
	QDir dir(path);
	QStringList nameFilters;
	nameFilters << "*.swc" ;
	QFileInfoList file_list = dir.entryInfoList(nameFilters,QDir::Files | QDir::Hidden | QDir::NoSymLinks);
	QFileInfoList folder_list = dir.entryInfoList(QDir::Dirs | QDir::NoDotAndDotDot);

	for(int i = 0; i != folder_list.size(); i++)
	{
		QString name = folder_list.at(i).absoluteFilePath();
		QFileInfoList child_file_list = GetFileInfoList(name);
		file_list.append(child_file_list);
	}

	return file_list;
}

//a tool count results' accuracy
void count_accuracy()
{
	//QString path = "C:/Users/Administrator/Desktop/query/CNGversion-kenyon";
	QString path = "C:/Users/Administrator/Desktop/test_projection/query_axon";
	QDir dir(path);
	QStringList nameFilters;
	nameFilters << "*.ano" ;
	QFileInfoList file_info_list = dir.entryInfoList(nameFilters, QDir::Files|QDir::Readable, QDir::Name);
	double count=0;
	for(int i=0;i<file_info_list.size();i++)
	{
		cout<<"count no."<<i<<" "<<qPrintable(file_info_list.at(i).absoluteFilePath())<<endl;
		int file_count = 5;
		QFile ano_file(file_info_list.at(i).absoluteFilePath());
		if (!ano_file.open(QIODevice::ReadOnly | QIODevice::Text))
		{
			cout<<"fail to open file "<<endl;
			return;
		}
		QTextStream in(&ano_file);
		QString line;
		line = in.readLine();
		while(!line.isNull())
		{
			//cout<<qPrintable(line)<<endl;
			if(line.at(0)=='#')
			{
				line = in.readLine();
				continue;
			}
			if(line.contains("/unprojection/"))
				--file_count;
			line = in.readLine();
		}
		if(file_count>=2)
			count++;
	}

	//cout<<"The accuracy is "<<DOUBLE(count/file_info_list.size())<<endl;
	v3d_msg("The accuracy is " + QString::number(count/file_info_list.size()));
}