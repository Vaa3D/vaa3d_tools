#include "neuron_dist_gui.h"
SelectNeuronDlg::SelectNeuronDlg(QWidget * parent) : QDialog(parent)
{
	line_edit1 = new QLineEdit(QObject::tr("choose neuron 1 .swc file here"));
	line_edit2 = new QLineEdit(QObject::tr("choose neuron 2 .swc file here"));
	line_edit1->setFixedWidth(400);
	line_edit2->setFixedWidth(400);

	QPushButton *push_button_openFile1 = new QPushButton(QObject::tr("..."));
	QPushButton *push_button_openFile2 = new QPushButton(QObject::tr("..."));
	QPushButton *push_button_ok = new QPushButton("OK");
	push_button_ok->setDefault(true);
	QPushButton *push_button_cancel = new QPushButton("Cancel");

	connect(push_button_openFile1, SIGNAL(clicked()), this, SLOT(_slots_openFileDlg1()));
	connect(push_button_openFile2, SIGNAL(clicked()), this, SLOT(_slots_openFileDlg2()));
	connect(push_button_ok,        SIGNAL(clicked()), this, SLOT(_slots_runPlugin()));
	connect(push_button_cancel,    SIGNAL(clicked()), this, SLOT(reject()));

	QGridLayout *layout_openFile = new QGridLayout();
	layout_openFile->addWidget(line_edit1, 1, 1);
	layout_openFile->addWidget(push_button_openFile1, 1, 2);
	layout_openFile->addWidget(line_edit2, 2, 1);
	layout_openFile->addWidget(push_button_openFile2, 2, 2);
	
	QHBoxLayout *layout_buttons = new QHBoxLayout;
	layout_buttons->addWidget(push_button_ok);
	layout_buttons->addWidget(push_button_cancel);

	QVBoxLayout *layout = new QVBoxLayout;
	layout->addLayout(layout_openFile);
	layout->addLayout(layout_buttons);

	setLayout(layout);
	setWindowTitle(QObject::tr("Choose Neurons ..."));

}

void SelectNeuronDlg::_slots_openFileDlg1()
{
	name_nt1 = QFileDialog::getOpenFileName(this, QObject::tr("Open Neuron 1"), "", QObject::tr("Neuron Structure files (*.swc *.eswc)"));
	line_edit1->setText(name_nt1);
}

void SelectNeuronDlg::_slots_openFileDlg2()
{
	name_nt2 = QFileDialog::getOpenFileName(this, QObject::tr("Open Neuron 2"), "", QObject::tr("Neuron Structure files (*.swc *.eswc)"));
	line_edit2->setText(name_nt2);
}

void SelectNeuronDlg::_slots_runPlugin()
{
	nt1 = readSWC_file(line_edit1->text());
	nt2 = readSWC_file(line_edit2->text());
	accept();
}
