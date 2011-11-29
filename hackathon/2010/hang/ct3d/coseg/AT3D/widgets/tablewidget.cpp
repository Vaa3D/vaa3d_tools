#include "tablewidget.h"
#include "ui_tablewidget.h"
#include <QFile>
#include <QIODevice>
#include <QFileDialog>

TableWidget::TableWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::TableWidget)
{
    ui->setupUi(this);
}

TableWidget::~TableWidget()
{
    delete ui;
}

QTableWidget* TableWidget::getTableWidget()
{
	return ui->tableWidget;
}

void TableWidget::changeEvent(QEvent *e)
{
    QWidget::changeEvent(e);
    switch (e->type()) {
    case QEvent::LanguageChange:
        ui->retranslateUi(this);
        break;
    default:
        break;
    }
}

void TableWidget::on_exportButton_clicked()
{
	QString fileName = QFileDialog::getSaveFileName(this, tr("Save File"),
			"",
			tr("Files (*.csv *.txt)"));
	QFile file(fileName);
	if(!file.open(QIODevice::WriteOnly | QIODevice::Text))
		         return;
	QTableWidget* tableWidget = getTableWidget();
	int num_row = tableWidget->rowCount();
	int num_column = tableWidget->columnCount();
	for(int i = 0; i < num_row; i++)
	{
		for(int j = 0; j < num_column; j++)
		{
			QTableWidgetItem* item = tableWidget->item(i,j);
			if(item != NULL)
			{
				file.write(item->text().toStdString().c_str());
			}
			if(j != num_column - 1) file.write(",");
			else file.write("\n");
		}
	}
	file.close();
}
