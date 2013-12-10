#include "parameterdialog.h"
#include "ui_parameterdialog.h"

#include <QtXml/QDomDocument>
#include <QtXml/QDomNodeList>
#include <QTextStream>
#include <QFile>

using namespace std;

ParameterDialog::ParameterDialog(QWidget *parent) :
  QDialog(parent),
  ui(new Ui::ParameterDialog)
{
  ui->setupUi(this);

  QDomDocument doc("configuration");

  QFile file("parameter_configuration.xml");
  if (file.open(QIODevice::ReadOnly)) {
    if (doc.setContent(&file)) {
      QDomElement docElem = doc.documentElement();
      QDomNodeList nodeList = docElem.elementsByTagName("parameter").at(0).
          toElement().elementsByTagName("string");
      if (nodeList.count() > 0) {
        QString parameter(nodeList.at(0).toElement().text());
        ui->parameterLineEdit->setText(parameter);
      }
    } else {
      file.close();
    }
  }
}

ParameterDialog::~ParameterDialog()
{
  QFile file("parameter_configuration.xml");
  if (file.open(QIODevice::WriteOnly)) {
    QTextStream stream(&file);
    stream << "<configuration>" << endl;
    stream << "<parameter>" << endl;
    stream << "<string>" << ui->parameterLineEdit->text() << "</string>" << endl;
    stream << "</parameter>" << endl;
    stream << "</configuration>" << endl;
  }

  delete ui;
}

QString ParameterDialog::parameter()
{
  return ui->parameterLineEdit->text();
}

void ParameterDialog::setParamterToolTip(const QString &str)
{
  ui->parameterLineEdit->setToolTip(str);
}
