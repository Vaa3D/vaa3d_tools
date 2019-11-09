/*
 * Copyright (c)2006-2010  Hanchuan Peng (Janelia Farm, Howard Hughes Medical Institute).
 * All rights reserved.
 */


/************
 ********* LICENSE NOTICE ************
 
 This folder contains all source codes for the V3D project, which is subject to the following conditions if you want to use it.
 
 You will ***have to agree*** the following terms, *before* downloading/using/running/editing/changing any portion of codes in this package.
 
 1. This package is free for non-profit research, but needs a special license for any commercial purpose. Please contact Hanchuan Peng for details.
 
 2. You agree to appropriately cite this work in your related studies and publications.
 
 Peng, H., Ruan, Z., Long, F., Simpson, J.H., and Myers, E.W. (2010) “V3D enables real-time 3D visualization and quantitative analysis of large-scale biological image data sets,” Nature Biotechnology, Vol. 28, No. 4, pp. 348-353, DOI: 10.1038/nbt.1612. ( http://penglab.janelia.org/papersall/docpdf/2010_NBT_V3D.pdf )
 
 Peng, H, Ruan, Z., Atasoy, D., and Sternson, S. (2010) “Automatic reconstruction of 3D neuron structures using a graph-augmented deformable model,” Bioinformatics, Vol. 26, pp. i38-i46, 2010. ( http://penglab.janelia.org/papersall/docpdf/2010_Bioinfo_GD_ISMB2010.pdf )
 
 3. This software is provided by the copyright holders (Hanchuan Peng), Howard Hughes Medical Institute, Janelia Farm Research Campus, and contributors "as is" and any express or implied warranties, including, but not limited to, any implied warranties of merchantability, non-infringement, or fitness for a particular purpose are disclaimed. In no event shall the copyright owner, Howard Hughes Medical Institute, Janelia Farm Research Campus, or contributors be liable for any direct, indirect, incidental, special, exemplary, or consequential damages (including, but not limited to, procurement of substitute goods or services; loss of use, data, or profits; reasonable royalties; or business interruption) however caused and on any theory of liability, whether in contract, strict liability, or tort (including negligence or otherwise) arising in any way out of the use of this software, even if advised of the possibility of such damage.
 
 4. Neither the name of the Howard Hughes Medical Institute, Janelia Farm Research Campus, nor Hanchuan Peng, may be used to endorse or promote products derived from this software without specific prior written permission.
 
 *************/


/*
 * basic_thread.h
 *
 *  Created on: Dec 20, 2010
 *      Author: Yang Yu
 */

#ifndef __BASIC_THREAD_H__
#define __BASIC_THREAD_H__

#include <QtGui>
#include <QMutex>
#include <QQueue>
#include <QThread>
#include <QWaitCondition>

#include "basic_triview.h"

// Communicating with the Main thread
class Transaction
{
public:
	virtual ~Transaction() { }
	
	virtual void update() = 0;
	virtual void reinit() = 0;
};

class UpdateTVTransaction : public Transaction
{
public:
	UpdateTVTransaction(TriviewControl *triviewControl)
	{
		if(triviewControl)
		{
			tvControl = triviewControl;
		}
		else
		{
			printf("The pointer to triview window is NULL!\n");
			tvControl = NULL;
		}
	}
	
	void update()
	{
		if(tvControl)
		{	
			// updateMinMax then changeFocus
			V3DLONG currslice = tvControl->getValidZslice();
			V3DLONG preslice = tvControl->getPreValidZslice();
			
			//qDebug()<<"the triview window exist ... ..."<<currslice<<preslice;
			
			if(currslice>preslice)
			{
				tvControl->updateMinMax(currslice-1);
				
				V3DLONG x, y, z; 
				tvControl->getFocusLocation( x, y, z);
				tvControl->setFocusLocation( x, y, currslice);
				
				tvControl->setPreValidZslice(currslice);
			}
		}
		else
		{
			printf("The pointer to triview window is NULL!\n");
			return;
		}
	}
	
	void reinit()
	{
		if(tvControl)
		{
			// reinit
			tvControl->setFocusLocation2Center();
		}
		else
		{
			printf("The pointer to triview window is NULL!\n");
			return;
		}
	}
	
private:
	TriviewControl *tvControl;
};

// define End Transaction
Transaction * const EndTransaction = 0;

// transactionthread class
class TransactionThread : public QThread
{
	Q_OBJECT
	
public:
	TransactionThread()
	{
		start();
	}
	~TransactionThread()
	{
		{
			QMutexLocker locker(&mutex);
			while (!transactions.isEmpty())
				delete transactions.dequeue();
			transactions.enqueue(EndTransaction);
			transactionAdded.wakeOne();
		}
		
		wait();
	}
	
	void addTransaction(Transaction *transact)
	{
		QMutexLocker locker(&mutex);
		transactions.enqueue(transact);
		transactionAdded.wakeOne();
	}
	
signals:
	void transactionStarted();
	void allTransactionsDone();
	
protected:
	void run()
	{
		Transaction *transact = 0;
		
		forever {
			{
				QMutexLocker locker(&mutex);
				
				if (transactions.isEmpty())
					transactionAdded.wait(&mutex);
				transact = transactions.dequeue();
				if (transact == EndTransaction)
					break;
				
				//
				transact->update();
				qDebug()<<"priority of updating GUI ... ... "<<this->priority();
			}
			
			//emit transactionStarted();
			//transact->update();
			delete transact;
			
			{
				QMutexLocker locker(&mutex);
				
				if (transactions.isEmpty())
				{
					emit allTransactionsDone();
				}
				
			}
		}
	}
	
private:
	QQueue<Transaction *> transactions;
	QWaitCondition transactionAdded;
	QMutex mutex;
};

#endif
