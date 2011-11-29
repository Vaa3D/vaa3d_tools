#include <QtGui>
#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include "RegistrationDlg.h"
#include "Registration.h"

RegistrationDlg::RegistrationDlg()
{
	QGroupBox *RegistrationGroup = new QGroupBox(tr("Registration Configure"));
	TransformLabel=new QLabel(tr("Transform:"));
	TransformMethod=new QComboBox;
	TransformMethod->addItem(tr("TranslationTransform"));
	TransformMethod->addItem(tr("ScaleTransform"));
	TransformMethod->addItem(tr("VersorTransform"));
	TransformMethod->addItem(tr("VersorRigid3DTransform"));	
	TransformMethod->addItem(tr("AffineTransform"));

	InterpolateLabel=new QLabel(tr("Interpolate:"));
	InterpolateMethod=new QComboBox;
        InterpolateMethod->addItem(tr("LinearInterpolate"));
        InterpolateMethod->addItem(tr("NearestNeighborInterpolate"));
        InterpolateMethod->addItem(tr("BSplineInterpolate"));

	MetricLabel=new QLabel(tr("Metric:"));
	MetricMethod=new QComboBox;
	if(TransformMethod->currentText() == "VersorRigid3DTransform" || TransformMethod->currentText() == "VersorTransform")
	{
		MetricMethod->addItem(tr("MeanSquaresMetric"));
		MetricMethod->addItem(tr("NormalizedCorrelationMetric"));
		MetricMethod->addItem(tr("MattesMutualInformationMetric"));
		MetricMethod->addItem(tr("MutualInformationMetric"));
	}
	else
	{
		MetricMethod->addItem(tr("MeanSquaresMetric"));
		MetricMethod->addItem(tr("NormalizedCorrelationMetric"));
		MetricMethod->addItem(tr("MatchCardinalityMetric"));
		MetricMethod->addItem(tr("MattesMutualInformationMetric"));
		MetricMethod->addItem(tr("MutualInformationMetric"));
	}

	OptimizerLabel=new QLabel(tr("Optimizer:"));
	OptimizerMethod=new QComboBox;
        if(TransformMethod->currentText() == "VersorRigid3DTransform")
            OptimizerMethod->addItem(tr("VersorRigid3DTransformOptimizer"));
	else if(TransformMethod->currentText() == "VersorTransform")
            OptimizerMethod->addItem(tr("VersorTransformOptimizer"));
	else if(MetricMethod->currentText() == "MatchCardinalityMetric")
	{
		OptimizerMethod->addItem(tr("AmoebaOptimizer"));
	}	
        else
        {
            OptimizerMethod->addItem(tr("RegularStepGradientDescentOptimizer"));
            OptimizerMethod->addItem(tr("GradientDescentOptimizer"));
            OptimizerMethod->addItem(tr("AmoebaOptimizer"));
            OptimizerMethod->addItem(tr("OnePlusOneEvolutionaryOptimizer"));
        }


        QGridLayout *grouplayout=new QGridLayout;
        grouplayout->setVerticalSpacing(12);
        grouplayout->addWidget(TransformLabel,1,0);
        grouplayout->addWidget(TransformMethod,1,1);
        grouplayout->addWidget(InterpolateLabel,2,0);
        grouplayout->addWidget(InterpolateMethod,2,1);
        grouplayout->addWidget(MetricLabel,3,0);
        grouplayout->addWidget(MetricMethod,3,1);
        grouplayout->addWidget(OptimizerLabel,4,0);
        grouplayout->addWidget(OptimizerMethod,4,1);
	RegistrationGroup->setLayout(grouplayout);

        RegSelectLabel=new QLabel(tr("Configure Result:"));
        RegSelectList=new QListWidget;
        QVBoxLayout *selectlayout=new QVBoxLayout;
        selectlayout->addWidget(RegSelectLabel);
        selectlayout->addWidget(RegSelectList);

	RegTypeLabel=new QLabel(tr("RegistrationType:"));
	RegType=new QComboBox;
        RegType->addItem(tr("ImageRegistrationMethod"));
        RegType->addItem(tr("MultiResolutionImageRegistrationMethod"));
	StartBtn=new QPushButton(tr("Start"));
        QGridLayout *reglayout=new QGridLayout;
	reglayout->addWidget(RegTypeLabel,0,0);
	reglayout->addWidget(RegType,0,1);
	reglayout->addWidget(StartBtn,1,1);


        QVBoxLayout *leftlayout = new QVBoxLayout;
        leftlayout->addWidget(RegistrationGroup);
        leftlayout->addLayout(selectlayout);
        leftlayout->addLayout(reglayout);

	QGroupBox *FinalParameterGroup = new QGroupBox(tr("Final Parameters:"));
	FinalParameterLabel=new QLabel(tr("Result:"));
	FinalParaList=new QListWidget;
	QVBoxLayout *resultlayout=new QVBoxLayout;
	resultlayout->addWidget(FinalParameterLabel);
        resultlayout->addWidget(FinalParaList);
        FinalParameterGroup->setLayout(resultlayout);

	SubtractBtn=new QPushButton(tr("Subtract"));
	ExitBtn=new QPushButton(tr("Exit"));
	QGridLayout* buttonlayout=new QGridLayout;
	buttonlayout->addWidget(SubtractBtn,0,1);
        buttonlayout->addWidget(ExitBtn,1,1);

        QVBoxLayout *rightlayout=new QVBoxLayout;
        rightlayout->addWidget(FinalParameterGroup);
        rightlayout->addLayout(buttonlayout);

        QHBoxLayout *layout=new QHBoxLayout;
        layout->addLayout(leftlayout);
        layout->addLayout(rightlayout);
        layout->setSpacing(20);
	this->setLayout(layout);    

        connect(StartBtn,SIGNAL(clicked()),this,SLOT(Start()));
	connect(SubtractBtn,SIGNAL(clicked()),this,SLOT(Subtract()));
        connect(ExitBtn,SIGNAL(clicked()),this,SLOT(close()));

        connect(TransformMethod,SIGNAL(currentIndexChanged(int)),this,SLOT(updateOptim(int)));
	connect(MetricMethod,SIGNAL(currentIndexChanged(int)),this,SLOT(updateIntOptim(int)));
 
	connect(TransformMethod,SIGNAL(currentIndexChanged(int)),this,SLOT(updateConfig()));
        connect(MetricMethod,SIGNAL(currentIndexChanged(int)),this,SLOT(updateConfig()));
        connect(InterpolateMethod,SIGNAL(currentIndexChanged(int)),this,SLOT(updateConfig()));
        connect(OptimizerMethod,SIGNAL(currentIndexChanged(int)),this,SLOT(updateConfig()));
	connect(RegType,SIGNAL(currentIndexChanged(int)),this,SLOT(updateConfig()));

        transform = getTransformType();
        interpolator =  getInterpolateType() + "ImageFunction";
        QString mtc = getMetricType();
        metric = mtc.mid(0,mtc.size()-6) + "ImageToImageMetric";
        optimizer = getOptimizerType();
        reg_str = RegType->currentText();

        RegSelectList->clear();
        QString str = "You have selected:";
        RegSelectList->addItem(str);
        RegSelectList->addItem(transform);
        RegSelectList->addItem(interpolator);
        RegSelectList->addItem(metric);
        RegSelectList->addItem(optimizer);
        RegSelectList->addItem(reg_str);
}
void RegistrationDlg::updateOptim(int i)
{
	if(TransformMethod->currentText() == "VersorRigid3DTransform")
	{
		OptimizerMethod->clear();
		OptimizerMethod->addItem(tr("VersorRigid3DTransformOptimizer"));
		MetricMethod->clear();
		MetricMethod->addItem(tr("MeanSquaresMetric"));
		MetricMethod->addItem(tr("NormalizedCorrelationMetric"));			
		MetricMethod->addItem(tr("MattesMutualInformationMetric"));
		MetricMethod->addItem(tr("MutualInformationMetric"));

	}
	else if(TransformMethod->currentText() == "VersorTransform")
	{
		OptimizerMethod->clear();
		OptimizerMethod->addItem(tr("VersorTransformOptimizer"));
		MetricMethod->clear();
		MetricMethod->addItem(tr("MeanSquaresMetric"));
		MetricMethod->addItem(tr("NormalizedCorrelationMetric"));			
		MetricMethod->addItem(tr("MattesMutualInformationMetric"));
		MetricMethod->addItem(tr("MutualInformationMetric"));
	}
	else
	{
		MetricMethod->clear();
		MetricMethod->addItem(tr("MeanSquaresMetric"));
		MetricMethod->addItem(tr("NormalizedCorrelationMetric"));
		MetricMethod->addItem(tr("MatchCardinalityMetric"));
		MetricMethod->addItem(tr("MattesMutualInformationMetric"));
		MetricMethod->addItem(tr("MutualInformationMetric"));

		OptimizerMethod->clear();		
		OptimizerMethod->addItem(tr("RegularStepGradientDescentOptimizer"));
		OptimizerMethod->addItem(tr("GradientDescentOptimizer"));
		OptimizerMethod->addItem(tr("AmoebaOptimizer"));
		OptimizerMethod->addItem(tr("OnePlusOneEvolutionaryOptimizer"));
	}
}
void RegistrationDlg::updateIntOptim(int i)
{
	if(MetricMethod->currentText() == "MatchCardinalityMetric")
	{
		InterpolateMethod->clear();
		InterpolateMethod->addItem(tr("NearestNeighborInterpolate"));

		OptimizerMethod->clear();
		OptimizerMethod->addItem(tr("AmoebaOptimizer"));
	}
	else if(MetricMethod->currentText() == "MutualInformationMetric")
	{
		OptimizerMethod->clear();
		OptimizerMethod->addItem(tr("GradientDescentOptimizer"));
	}
	else
	{
		InterpolateMethod->clear();
		InterpolateMethod->addItem(tr("LinearInterpolate"));
        	InterpolateMethod->addItem(tr("NearestNeighborInterpolate"));
        	InterpolateMethod->addItem(tr("BSplineInterpolate"));
	
		if(TransformMethod->currentText() == "VersorRigid3DTransform")
		{
			OptimizerMethod->clear();
			OptimizerMethod->addItem(tr("VersorRigid3DTransformOptimizer"));
		}
		else if(TransformMethod->currentText() == "VersorTransform")
		{
			OptimizerMethod->clear();
			OptimizerMethod->addItem(tr("VersorTransformOptimizer"));			
		}	
		else
		{
			OptimizerMethod->clear();
			OptimizerMethod->addItem(tr("RegularStepGradientDescentOptimizer"));
			OptimizerMethod->addItem(tr("GradientDescentOptimizer"));
			OptimizerMethod->addItem(tr("AmoebaOptimizer"));
			OptimizerMethod->addItem(tr("OnePlusOneEvolutionaryOptimizer"));
		}
	}
}
void RegistrationDlg::updateConfig()
{
	transform = getTransformType();
	interpolator =  getInterpolateType() + "ImageFunction";
	QString mtc = getMetricType();
	metric = mtc.mid(0,mtc.size()-6) + "ImageToImageMetric";
	optimizer = getOptimizerType();
	reg_str = RegType->currentText();

	RegSelectList->clear();
	QString str = "You have selected:";
	RegSelectList->addItem(str);
	RegSelectList->addItem(transform);
	RegSelectList->addItem(interpolator);
	RegSelectList->addItem(metric);
	RegSelectList->addItem(optimizer);
	RegSelectList->addItem(reg_str);
}
void RegistrationDlg::SetCallback( V3DPluginCallback &callback)
{
	this->callback=&callback;
}

QString RegistrationDlg::getTransformType()
{
	return TransformMethod->currentText();	 
}  
QString RegistrationDlg::getInterpolateType()
{
	return InterpolateMethod->currentText();	 
}
QString RegistrationDlg::getMetricType()
{
	return MetricMethod->currentText();	 
}
QString RegistrationDlg::getOptimizerType()
{
	return OptimizerMethod->currentText();
}
void RegistrationDlg::Start()
{
	PluginSpecialized<unsigned char> runner(this->callback,transform,interpolator ,metric,optimizer,reg_str);
	runner.Execute(0);
	QList<double> list = runner.GetParaList();
	SetFinalParameters(list);

}
void RegistrationDlg::SetFinalParameters(QList<double> list)
{
	FinalParaList->clear();
	QList<QString> channel;
	QString Rchannel = "R Channel Parameters:";
	QString Gchannel = "G Channel Parameters:";
	QString Bchannel = "B Channel Parameters:";
	channel<<Rchannel<<Gchannel<<Bchannel;
	unsigned int i;
	if(transform == "TranslationTransform")
	{
		QString iters;
		QString meVal;
		QString tranX;
		QString tranY;
		QString tranZ;
		for(i=0;i<3;i++)
		{
			FinalParaList->addItem(channel.at(i));
			iters = "Iterations: " + QString::number(list.at(i*5+0),'g',6);
			meVal = "Metric Value: " + QString::number(list.at(i*5+1),'g',6);
			tranX = "Translation along X: " + QString::number(list.at(i*5+2),'g',6);
			tranY = "Translation along Y: " + QString::number(list.at(i*5+3),'g',6);
			tranZ = "Translation along Z: " + QString::number(list.at(i*5+4),'g',6);

			FinalParaList->addItem(iters);
			FinalParaList->addItem(meVal);
			FinalParaList->addItem(tranX);
			FinalParaList->addItem(tranY);
			FinalParaList->addItem(tranZ);
		}
	}
	if(transform == "ScaleTransform")
	{
		QString iters;
		QString meVal;
		QString scaleX;
		QString scaleY;
		QString scaleZ;
		for(i=0;i<3;i++)
		{
			FinalParaList->addItem(channel.at(i));
			iters = "Iterations: " + QString::number(list.at(i*5+0),'g',6);
			meVal = "Metric Value: " + QString::number(list.at(i*5+1),'g',6);
			scaleX = "Scale along X: " + QString::number(list.at(i*5+2),'g',6);
			scaleY = "Scale along Y: " + QString::number(list.at(i*5+3),'g',6);
			scaleZ = "Scale along Z: " + QString::number(list.at(i*5+4),'g',6);

			FinalParaList->addItem(iters);
			FinalParaList->addItem(meVal);
			FinalParaList->addItem(scaleX);
			FinalParaList->addItem(scaleY);
			FinalParaList->addItem(scaleZ);
		}
	}
	if(transform == "VersorTransform")
	{
		QString iters;
		QString meVal;
		QString versorX;
		QString versorY;
		QString versorZ;
		for(i=0;i<3;i++)
		{
			FinalParaList->addItem(channel.at(i));
			iters = "Iterations: " + QString::number(list.at(i*5+0),'g',6);
			meVal = "Metric Value: " + QString::number(list.at(i*5+1),'g',6);
			versorX = "Versor X: " + QString::number(list.at(i*5+2),'g',6);
			versorY = "Versor Y: " + QString::number(list.at(i*5+3),'g',6);
			versorZ = "Versor Z: " + QString::number(list.at(i*5+4),'g',6);

			FinalParaList->addItem(iters);
			FinalParaList->addItem(meVal);
			FinalParaList->addItem(versorX);
			FinalParaList->addItem(versorY);
			FinalParaList->addItem(versorZ);
		}
	}	
	if(transform == "VersorRigid3DTransform")
	{
		QString iters;
		QString meVal;
		QString versorX;
		QString versorY;
		QString versorZ;
		QString tranX;
		QString tranY;
		QString tranZ;
		for(i=0;i<3;i++)
		{
			FinalParaList->addItem(channel.at(i));
			iters = "Iterations: " + QString::number(list.at(i*8+0),'g',6);
			meVal = "Metric Value: " + QString::number(list.at(i*8+1),'g',6);
			versorX = "Versor X: " + QString::number(list.at(i*8+2),'g',6);
			versorY = "Versor Y: " + QString::number(list.at(i*8+3),'g',6);
			versorZ = "Versor Z: " + QString::number(list.at(i*8+4),'g',6);
			tranX = "Translation X: " + QString::number(list.at(i*8+5),'g',6);
			tranY = "Translation Y: " + QString::number(list.at(i*8+6),'g',6);
			tranZ = "Translation Z: " + QString::number(list.at(i*8+7),'g',6);


			FinalParaList->addItem(iters);
			FinalParaList->addItem(meVal);
			FinalParaList->addItem(versorX);
			FinalParaList->addItem(versorY);
			FinalParaList->addItem(versorZ);
			FinalParaList->addItem(tranX);
			FinalParaList->addItem(tranY);
			FinalParaList->addItem(tranZ);
		}
	}	
	if(transform == "AffineTransform")
	{
		QString iters;
		QString meVal;
		QString matRow1;
		QString matRow2;
		QString matRow3;
		QString tranX;
		QString tranY;
		QString tranZ;
		QString mat = "Matrix = ";
		for(i=0;i<3;i++)
		{
			FinalParaList->addItem(channel.at(i));
			iters = "Iterations: " + QString::number(list.at(i*14+0),'g',6);
			meVal = "Metric Value: " + QString::number(list.at(i*14+1),'g',6);
			matRow1 = QString::number(list.at(i*14+2),'g',6)+QString::number(list.at(i*14+3),'g',6)+QString::number(list.at(i*14+4),'g',6);
			matRow2 = QString::number(list.at(i*14+5),'g',6)+QString::number(list.at(i*14+6),'g',6)+QString::number(list.at(i*14+7),'g',6);
			matRow3 = QString::number(list.at(i*14+8),'g',6)+QString::number(list.at(i*14+9),'g',6)+QString::number(list.at(i*14+10),'g',6);
			tranX = "Translation X: " + QString::number(list.at(i*14+11),'g',6);
			tranY = "Translation Y: " + QString::number(list.at(i*14+12),'g',6);
			tranZ = "Translation Z: " + QString::number(list.at(i*14+13),'g',6);

			FinalParaList->addItem(iters);
			FinalParaList->addItem(meVal);
			FinalParaList->addItem(mat);
			FinalParaList->addItem(matRow1);
			FinalParaList->addItem(matRow2);
			FinalParaList->addItem(matRow3);
			FinalParaList->addItem(tranX);
			FinalParaList->addItem(tranY);
			FinalParaList->addItem(tranZ);
		}
	}		

}
void RegistrationDlg::Subtract()
{
	PluginSpecialized<unsigned char> runner(this->callback);
	runner.ImageSubtract();
     
}

