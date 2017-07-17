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

struct branchNodeProfile
{
	NeuronSWC branchingNode;
	double dist;
	int brachingNum;
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

	bool circle;
	bool forward;
	NeuronTree inputSWC;
	QVector< QVector<V3DLONG> > childsTable;
	QVector<long int> toBeBrokenLoc;

	QVector< QVector<V3DLONG> > childIndexTable(NeuronTree& nt);
	void findPath(QVector< QVector<V3DLONG> >& childList, NeuronTree& nt, long int wishedSomaID, long int excludedSomaID, QList< QList<NeuronSWC> >& outputPathList);
	void downwardBFS(QList<NeuronSWC>& tracedSWC, NeuronTree& nt, NeuronSWC& start, NeuronSWC& end, QVector< QVector<V3DLONG> >& childList);
	void backwardPath(QList<NeuronSWC>& tracedSWC, NeuronTree& nt, NeuronSWC& start, NeuronSWC& end);
	long int findLatestAncestor(bool& circle, NeuronTree& nt, QVector< QVector<V3DLONG> >& childList, NeuronSWC& wishedSoma, NeuronSWC& excludedSoma, bool& forward);
	void breakPathMorph(NeuronTree& nt, QList<NeuronSWC>& path, QVector< QVector<V3DLONG> >& childList, NeuronSWC wishedSoma, NeuronSWC excludedSoma);
};

#endif

