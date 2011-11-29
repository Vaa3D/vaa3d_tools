#include "finetuningdialog.h"
#include "ui_finetuningdialog.h"

#include <cassert>

QString static getColorStr(unsigned int color);
QString static hex2str(int v);
vector<int> static getRandColors(int colorNum);

FineTuningDialog::FineTuningDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::FineTuningDialog)
{
    ui->setupUi(this);
	connect(ui->thresholdScrollBar, SIGNAL(valueChanged(int)), this, SLOT(onThreshold(int)));
	connect(ui->createTreeButton, SIGNAL(clicked()), this, SLOT(onCreateTree()));
	connect(ui->loadTreeButton, SIGNAL(clicked()), this, SLOT(onLoadTree()));
	connect(ui->chooseButton, SIGNAL(clicked()), this, SLOT(onChoose()));
	connect(ui->resetButton, SIGNAL(clicked()), this, SLOT(onReset()));
	m_tree = NULL;
	m_mainLabel = -1;
	initCellWidget(1);
	initGLWidget();
	connect(m_glWidget, SIGNAL(mouseClicked(float, float)), this, SLOT(onClickedNode(float,float)));
}

FineTuningDialog::~FineTuningDialog()
{
    delete ui;
}

/// becareful set m_colors
int FineTuningDialog::getLabel() const
{
	return m_mainLabel;
}

void FineTuningDialog::setParameters(ComponentTree* tree, int label)
{
	assert(tree != NULL);
	m_tree = tree;
	m_colors = getRandColors(tree->nodeNum());
	m_mainLabel = label;
	if(m_mainLabel == -1) m_mainLabel = tree->root()->getLabel();
	m_labels.clear();
	m_labels.push_back(m_mainLabel);
	int level = tree->getNode(label)->getLowestLevel();
	ui->thresholdScrollBar->setValue(level);
	updateCellWidget();
	updateGLWidget();
	updateInfoWidget();
}

void FineTuningDialog::changeEvent(QEvent *e)
{
    QDialog::changeEvent(e);
    switch (e->type()) {
    case QEvent::LanguageChange:
        ui->retranslateUi(this);
        break;
    default:
        break;
    }
}

int FineTuningDialog::numCells()
{
	return m_labels.size();
}
void FineTuningDialog::initCellWidget(int cellnum)
{
	assert(cellnum > 0);
	m_checkers.resize(cellnum);
	m_editors.resize(cellnum);
	QVBoxLayout* layout = new QVBoxLayout();
	for(int i = 0; i < cellnum;i++)
	{
		QHBoxLayout* child_layout = new QHBoxLayout();
		QCheckBox* checker = new QCheckBox();
		connect(checker, SIGNAL(stateChanged(int)), this, SLOT(onCheckCell()));
		QTextEdit* editor = new QTextEdit();
		editor->setReadOnly(true);
		editor->setFixedHeight(20);
		editor->setFixedWidth(120);

		child_layout->addWidget(checker);
		child_layout->addWidget(editor);
		m_checkers[i] = checker;
		m_editors[i] = editor;
		layout->addLayout(child_layout);
	}	
	ui->cellWidget->setLayout(layout);
}

void FineTuningDialog::updateCellWidget()
{
	assert(m_checkers.size() == m_editors.size());
	int old_cellnum = m_checkers.size();
	int new_cellnum = m_labels.size();
	QVBoxLayout* layout = (QVBoxLayout*) ui->cellWidget->layout();
	if(new_cellnum > old_cellnum)
	{
		//m_checkers.resize(new_cellnum);
		//m_editors.resize(new_cellnum);
		for(int i = old_cellnum; i < new_cellnum; i++)
		{
			QHBoxLayout* child_layout = new QHBoxLayout();
			QCheckBox* checker = new QCheckBox();
			connect(checker, SIGNAL(stateChanged(int)), this, SLOT(onCheckCell()));
			QTextEdit* editor = new QTextEdit();
			editor->setReadOnly(true);
			editor->setFixedHeight(20);
			editor->setFixedWidth(120);

			child_layout->addWidget(checker);
			child_layout->addWidget(editor);
			m_checkers.push_back(checker);
			m_editors.push_back(editor);
			layout->addLayout(child_layout);
		}
	}
	else if(new_cellnum < old_cellnum)
	{
		for(int i = new_cellnum; i < old_cellnum; i++)
		{
			m_checkers[i]->setVisible(false);
			m_editors[i]->setVisible(false);
		}
	}
	for(int i = 0 ;i < new_cellnum; i++)
	{
		int label = m_labels[i];
		int beta_size = m_tree->getNode(label)->getBetaSize();
		int color = m_colors[label];
		m_checkers[i]->setText(tr("%1").arg(label));
		m_editors[i]->setText(tr("<span style=\" color:#%1;\">%2</span>").arg(getColorStr((unsigned int)color)).arg(beta_size));

		m_checkers[i]->setVisible(true);
		m_editors[i]->setVisible(true);

		m_checkers[i]->setChecked(false);
	}
}

void FineTuningDialog::initGLWidget()
{
	m_glWidget = new GLWidget();
	ui->glWidget->setHidden(true);
	QGridLayout* layout = (QGridLayout*) ui->glGroupBox->layout();
	ui->glWidget->setHidden(true);
	layout->addWidget(m_glWidget, 0,0,1,2);
	m_glWidget->resize(200,200);
	m_glWidget->updateGL();
}

void FineTuningDialog::updateGLWidget()
{
	if(m_tree == NULL) return;
	int w = m_tree->width();
	int h = m_tree->height();
	int d = m_tree->depth();
	m_glWidget->loadTexture(this->getTexture(), w, h, d, 3);
	m_glWidget->updateGL();
}
void FineTuningDialog::updateInfoWidget()
{
	if(m_tree == NULL) return;
	int w = m_tree->width();
	int h = m_tree->height();
	int d = m_tree->depth();
	
	ui->minEditor->setText(tr("%1").arg(m_tree->getMinThresh()));
	ui->maxEditor->setText(tr("%1").arg(m_tree->getMaxThresh()));
	ui->singleEditor->setText(tr("%1").arg(m_tree->getSingleThresh()));

	ui->widthLabel2->setText(tr("%1").arg(w));
	ui->heightLabel2->setText(tr("%1").arg(h));
	ui->depthLabel2->setText(tr("%1").arg(d));
	
	ui->mainLabelLabel2->setText(tr("%1").arg(m_mainLabel));
	ui->levelLabel2->setText(tr("%1").arg(m_tree->getNode(m_mainLabel)->getLowestLevel()));

}
void FineTuningDialog::onCheckCell()
{
	if(m_tree == NULL) return;
	updateGLWidget();
}

void FineTuningDialog::onChoose()
{
	if(m_tree == NULL) return;
	int cellnum = m_labels.size();
	vector<int> labels;
	if(cellnum == 1) labels = m_labels;
	else
	{
		int i = 0;
		for(; i < cellnum; i++)
		{
			if(m_checkers[i]->isChecked()) labels.push_back(m_labels[i]);
		}
	}
	if(labels.empty())
	{
		QMessageBox::warning(this,"", "Please choose one item!");
	}
	else if(labels.size() > 1)
	{
		QMessageBox::warning(this,"","Please choose no more than one item!");
		return;
	}
	else
	{
		m_history.push_back(m_mainLabel);
		m_mainLabel = labels[0];
		m_labels = labels;
		updateCellWidget();
		updateGLWidget();
		updateInfoWidget();
		ui->thresholdScrollBar->setValue(m_tree->getNode(m_mainLabel)->getLowestLevel());
	}
}

void FineTuningDialog::onClickedNode(float posX, float posY)
{
	if(m_tree == NULL) return;
	int width = m_tree->width();
	int height = m_tree->height();
	int depth = m_tree->depth();
	int min_label = -1;
	float min_dist = -1.0;
	vector<int>::iterator it = m_labels.begin();
	while(it != m_labels.end())
	{
		int label = *it;
		float cx, cy, cz;
		double wx, wy, wz;
		m_tree->getNode(label)->getCenter(cx,cy,cx,width, height, depth);
		m_glWidget->getProjection(wx,wy,wz,(double)cx,(double)cy,(double)cz);
		float dist = (wx - posX)*(wx - posX) + (wy - posY) * (wy - posY);
		if(min_label == -1) min_label = label;
		if(min_dist < 0.0) min_dist = dist;
		if(dist < min_dist)
		{
			min_label = label;
			min_dist = dist;
		}
		it++;
	}
	int i = 0;
	for(; i < m_labels.size();i++)
	{
		if(min_label == m_labels[i]) break;
	}
	if(m_checkers[i]->isChecked())m_checkers[i]->setChecked(false);
	else m_checkers[i]->setChecked(true);
}

void FineTuningDialog::onReset()
{
	if(m_history.empty()) return;
	else 
	{
		m_mainLabel = m_history.back();
		m_history.pop_back();
	}
	int thresh_value = ui->thresholdScrollBar->value();
	onThreshold(thresh_value);
}

void FineTuningDialog::onThreshold(int thresh_value)
{
	if(m_tree == NULL) return;
	ComponentTree::Node* curNode = m_tree->getNode(m_mainLabel);
	int level = curNode->getLowestLevel();
	if(thresh_value <= level)
	{
		m_labels.clear();
		ComponentTree::Node* p = curNode;
		while(p->getParent()->getLowestLevel() > thresh_value)
		{
			p = p->getParent();
		}
		// to avoid dead loop
		if(p->getParent()->getLowestLevel() == thresh_value)
		{
			p = p->getParent();
		}
		m_labels.push_back(p->getLabel());
	}
	else if(thresh_value > level)
	{
		m_labels.clear();
		vector<ComponentTree::Node*> nodes = curNode->getBreadthFirstNodes();
		vector<ComponentTree::Node*>::iterator it = nodes.begin();
		while(it != nodes.end())
		{
			ComponentTree::Node* node = *it;
			if(node->getLowestLevel() >= thresh_value && node->getParent()->getLowestLevel() < thresh_value)
			{
				m_labels.push_back(node->getLabel());
			}
			it++;
		}
	}
	updateCellWidget();
	updateGLWidget();
	updateInfoWidget();
}



unsigned char* FineTuningDialog::getTexture()
{
	if(m_tree == NULL) return NULL;
	int width = m_tree->width();
	int height = m_tree->height();
	int depth = m_tree->depth();
	int size = width*height*depth*3;
	unsigned char* image = new unsigned char[size];
	for(int i = 0; i < size; i++) image[i] = 0;
	for(int i = 0; i < (int)m_labels.size(); i++)
	{
		int label = m_labels[i];
		int color = m_colors[label];
		unsigned char r = color % 256;
		unsigned char g = color / 256 % 256;
		unsigned char b = color / 256 /256  % 256;
		ComponentTree::Node* node = m_tree->getNode(label);
		vector<int> vertices = node->getBetaPoints();
		vector<int>::iterator itr = vertices.begin();
		while(itr != vertices.end())
		{
			int index = *itr;
			image[3*index] = r;
			image[3*index+1] = g;
			image[3*index+2] = b;
			itr++;
		}
		if(m_checkers[i]->isChecked())
		{
			r = 255 - r;
			g = 255 - g;
			b = 255 - b;
			vector<int> center_vertices = node->getCenterPoints(width, height, depth);
			itr = center_vertices.begin();
			while(itr != center_vertices.end())
			{
				int index = *itr;
				image[3*index] = r;
				image[3*index+1] = g;
				image[3*index+2] = b;
				itr++;
			}
		}
	}
	return image;
}

void FineTuningDialog::onLoadTree()
{
	//1. load tree
	QString tree_file = QFileDialog::getOpenFileName(this, tr("Open File"),  "",tr("Trees (*.tree)"));
	m_tree = new ComponentTree();
	m_tree->load((char*)tree_file.toStdString().c_str());
	// 2. refresh m_mainLabel to root node label
	if(m_mainLabel == -1) m_mainLabel = m_tree->root()->getLabel();
	// 3. add m_mainLabel to m_labels
	m_labels.clear();
	m_labels.push_back(m_mainLabel);
	m_colors = getRandColors(m_tree->nodeNum());
	// 4. update cell widget and GLWidget
	updateCellWidget();
	updateGLWidget();
	updateInfoWidget();
}

// becareful set m_colors
void FineTuningDialog::onCreateTree()
{
	//1. load tree
	QString img_file = QFileDialog::getOpenFileName(this, tr("Open File"),"",tr("Images (*.tiff *.tif)"));
	int min_thresh = atoi((char*)ui->minEditor->text().toStdString().c_str());
	int max_thresh = atoi((char*)ui->maxEditor->text().toStdString().c_str());
	int single_thresh = atoi((char*)ui->singleEditor->text().toStdString().c_str());
	cout<<"min : "<<min_thresh<<endl;
	cout<<"max : "<<max_thresh<<endl;
	cout<<"single : "<<single_thresh<<endl;
	m_tree = new ComponentTree();
	m_tree->create((char*)img_file.toStdString().c_str(), min_thresh, max_thresh, single_thresh);
	// 2. refresh m_mainLabel to root node label
	if(m_mainLabel == -1) m_mainLabel = m_tree->root()->getLabel();
	// 3. add m_mainLabel to m_labels
	m_labels.clear();
	m_labels.push_back(m_mainLabel);
	m_colors = getRandColors(m_tree->nodeNum());
	// 4. update cell widget and gl widget
	updateCellWidget();
	updateGLWidget();
	updateInfoWidget();
}

vector<int> getRandColors(int colorNum)
{
	vector<int> colors;
	int color_max = 256*256*256;
	for(int i = 0; i < colorNum; i++)
	{
		int color = rand() % color_max;
		colors.push_back(color);
	}
	return colors;
}
//
// color =  a*255^3 + b*255^2 + g*255 + r
QString getColorStr(unsigned int color)
{
    int high,low;
    QString out;
	int r = color % 256;
	int g = (color / 256) % 256;
	int b = (color / 256/ 256) % 256;
    high=r/16;
    low=r%16;
    out+=QObject::tr("%1%2").arg(hex2str(high)).arg(hex2str(low));
    high=g/16;
    low=g%16;
    out+=QObject::tr("%1%2").arg(hex2str(high)).arg(hex2str(low));
    high=b/16;
    low=b%16;
    out+=QObject::tr("%1%2").arg(hex2str(high)).arg(hex2str(low));
    return out;
}

QString hex2str(int v)
{
    if(v<0 || v>15)return QString("");
    else
    {
        switch(v)
        {
            case 10: return QString("a");
            case 11: return QString("b");
            case 12: return QString("c");
            case 13: return QString("d");
            case 14: return QString("e");
            case 15: return QString("f");
            default: return QObject::tr("%1").arg(v);
        }
    }
}
