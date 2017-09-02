/* Neuron_Separator_plugin.h
 * This is a test plugin, you can use it as a demo.
 * 2017-7-12 : by MK
 */
 
#ifndef __NEURON_SEPARATOR_PLUGIN_H__
#define __NEURON_SEPARATOR_PLUGIN_H__

#include <QtGui>
#include <v3d_interface.h>
#include <qstringlist.h>
#include <qstring.h>
#include "basic_surf_objs.h"
#include <qvector.h>
#include <vector>
#include <unordered_map>

using namespace std;

struct branchNodeProfile
{
	NeuronSWC branchingNode, soma;
	double dist;
	int branchingNum;
	bool toBeBroken;
	double dirIndex;
	double somaBranchNorm[3];
	long int loc, childLoc;
	bool head;
};

struct somaNode
{
	NeuronSWC node;
	vector<somaNode*> childrenSomas;
	vector<somaNode*> parent;
	int level;
	bool branch, headSoma, tailSoma, middleSoma;

	somaNode::somaNode() {branch = false; headSoma = false; tailSoma = false; middleSoma = false;}
};

class neuronSeparator : public QObject, public V3DPluginInterface2_1
{
	Q_OBJECT
	Q_INTERFACES(V3DPluginInterface2_1);

public:
	float getPluginVersion() const {return 1.1f;}

	QStringList menulist() const;
	void domenu(const QString &menu_name, V3DPluginCallback2 &callback, QWidget *parent);

	QStringList funclist() const ;
	bool dofunc(const QString &func_name, const V3DPluginArgList &input, V3DPluginArgList &output, V3DPluginCallback2 &callback, QWidget *parent);
	
	// -----------------------------------------------------------------------------------------------------------------------------------------
	QVector<long int> toBeBrokenLoc;
	QList<NeuronSWC> extractedNeuron;
	
	vector<long int> somaIDs;
	QList<NeuronSWC> somaPath;
	NeuronTree somaSWCTree;
	QVector< QVector<V3DLONG> > childsTable;
	QHash<long int, bool> locLabel;

	QVector< QVector<V3DLONG> > childIndexTable(NeuronTree& nt);
	QList<NeuronSWC> findPath(QVector< QVector<V3DLONG> >& childList, NeuronTree& nt, long int wishedSomaID, long int excludedSomaID);
	void backwardPath(QList<NeuronSWC>& tracedSWC, NeuronTree& nt, NeuronSWC& start, NeuronSWC& end);
	long int findLatestCommonAncestor(bool& circle, NeuronTree& nt, QVector< QVector<V3DLONG> >& childList, NeuronSWC& wishedSoma, NeuronSWC& excludedSoma);
	void getSomaPath(QList<NeuronSWC>& somaPath, QHash<long int, bool>& locLabel, QList< QList<NeuronSWC> >& paths, NeuronTree& nt);
	
	QVector< QVector<V3DLONG> > mkChildTableScratch(QList<NeuronSWC>&);
	vector<long int> mkPaTableScratch(QList<NeuronSWC>&);
	
	void downwardBFS(QList<NeuronSWC>& tracedSWC, NeuronTree& nt, NeuronSWC& start, NeuronSWC& end, QVector< QVector<V3DLONG> >& childList);
	void breakPathMorph(NeuronTree& nt, QList<NeuronSWC>& path, QVector< QVector<V3DLONG> >& childList, NeuronSWC wishedSoma, NeuronSWC excludedSoma);
	void extractBFS(QList<NeuronSWC>& tracedSWC, QList<NeuronSWC>& inputList, NeuronSWC& start);
	void breakSomaPathMorph();
	
protected:
	void buildSomaTree();

private:
	bool circle;
	bool branchAncestor;
	int forward;
	NeuronTree inputSWCTree;
	QList< QList<NeuronSWC> > paths;

	unordered_map<size_t, somaNode*> crucialNodeHash;
	vector<somaNode> crucialNodes;
	somaNode* somaTreePtr;
	
	vector<branchNodeProfile> branchToBeCut;

};

#endif

