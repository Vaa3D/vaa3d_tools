//------------------------------------------------------------------------------------------------
// Copyright (c) 2012  Alessandro Bria and Giulio Iannello (University Campus Bio-Medico of Rome).  
// All rights reserved.
//------------------------------------------------------------------------------------------------

/*******************************************************************************************************************************************************************************************
*    LICENSE NOTICE
********************************************************************************************************************************************************************************************
*    By downloading/using/running/editing/changing any portion of codes in this package you agree to this license. If you do not agree to this license, do not download/use/run/edit/change
*    this code.
********************************************************************************************************************************************************************************************
*    1. This material is free for non-profit research, but needs a special license for any commercial purpose. Please contact Alessandro Bria at a.bria@unicas.it or Giulio Iannello at 
*       g.iannello@unicampus.it for further details.
*    2. You agree to appropriately cite this work in your related studies and publications.
*
*       Bria, A., Iannello, G., "TeraStitcher - A Tool for Fast 3D Automatic Stitching of Teravoxel-sized Microscopy Images", (2012) BMC Bioinformatics, 13 (1), art. no. 316.
*
*    3. This material is provided by  the copyright holders (Alessandro Bria  and  Giulio Iannello),  University Campus Bio-Medico and contributors "as is" and any express or implied war-
*       ranties, including, but  not limited to,  any implied warranties  of merchantability,  non-infringement, or fitness for a particular purpose are  disclaimed. In no event shall the
*       copyright owners, University Campus Bio-Medico, or contributors be liable for any direct, indirect, incidental, special, exemplary, or  consequential  damages  (including, but not 
*       limited to, procurement of substitute goods or services; loss of use, data, or profits;reasonable royalties; or business interruption) however caused  and on any theory of liabil-
*       ity, whether in contract, strict liability, or tort  (including negligence or otherwise) arising in any way out of the use of this software,  even if advised of the possibility of
*       such damage.
*    4. Neither the name of University  Campus Bio-Medico of Rome, nor Alessandro Bria and Giulio Iannello, may be used to endorse or  promote products  derived from this software without
*       specific prior written permission.
********************************************************************************************************************************************************************************************/

#include "StackRestorer.h"
#include "Stack.h"
#include <fstream>
#include "S_config.h"

StackRestorer::StackRestorer(StackedVolume* stk_org)
{
	this->STK_ORG = stk_org;
	N_ROWS = stk_org->getN_ROWS();
	N_COLS = stk_org->getN_COLS();

	this->STKS_DESCRIPTORS = new vol_descr_t*[N_ROWS];
	for(int i=0; i < N_ROWS; i++)
		STKS_DESCRIPTORS[i] = new vol_descr_t[N_COLS];

	this->SUBSTKS_DESCRIPTORS = NULL;
}


StackRestorer::StackRestorer(StackedVolume* stk_org, char* file_path)
{
	this->STK_ORG = stk_org;
	N_ROWS = stk_org->getN_ROWS();
	N_COLS = stk_org->getN_COLS();

	this->STKS_DESCRIPTORS = new vol_descr_t*[N_ROWS];
	for(int i=0; i < N_ROWS; i++)
		STKS_DESCRIPTORS[i] = new vol_descr_t[N_COLS];

	this->SUBSTKS_DESCRIPTORS = NULL;

	load(file_path);
}

StackRestorer::StackRestorer(StackedVolume* stk_org, int D_subvols)
{
	this->STK_ORG = stk_org;
	N_ROWS = stk_org->getN_ROWS();
	N_COLS = stk_org->getN_COLS();

	this->STKS_DESCRIPTORS = new vol_descr_t*[N_ROWS];
	for(int i=0; i < N_ROWS; i++)
		STKS_DESCRIPTORS[i] = new vol_descr_t[N_COLS];

	SD_DIM_i = N_ROWS;
	SD_DIM_j = N_COLS;
	SD_DIM_k = D_subvols;

	this->SUBSTKS_DESCRIPTORS = new vol_descr_t**[SD_DIM_i];
	for(int i=0; i < SD_DIM_i; i++)
	{
		SUBSTKS_DESCRIPTORS[i] = new vol_descr_t *[SD_DIM_j];
		for(int j=0; j < SD_DIM_j; j++)
			SUBSTKS_DESCRIPTORS[i][j] = new vol_descr_t[SD_DIM_k];
	}
}

StackRestorer::~StackRestorer(void)
{
	if(SUBSTKS_DESCRIPTORS)
	{
		for(int i=0; i < SD_DIM_i; i++)
		{
			for(int j=0; j < SD_DIM_j; j++)
				delete[] SUBSTKS_DESCRIPTORS[i][j];
			delete[] SUBSTKS_DESCRIPTORS[i];
		}
		delete[] SUBSTKS_DESCRIPTORS;
	}
	if(STKS_DESCRIPTORS)
	{
		for(int i=0; i < N_ROWS; i++)
			delete[] STKS_DESCRIPTORS[i];
		delete[] STKS_DESCRIPTORS;
	}
}

void StackRestorer::computeSubvolDescriptors(real_t* data, Stack* stk_p, int subvol_idx, int subvol_D_dim) throw (MyException)
{
	int i = stk_p->getROW_INDEX();
	int j = stk_p->getCOL_INDEX();
	SUBSTKS_DESCRIPTORS[i][j][subvol_idx].init(stk_p, true, stk_p->getHEIGHT(), stk_p->getWIDTH(), subvol_D_dim);
	SUBSTKS_DESCRIPTORS[i][j][subvol_idx].computeSubvolDescriptors(data);
}

void StackRestorer::computeStackDescriptors(Stack* stk_p) throw (MyException)
{
	int i = stk_p->getROW_INDEX();
	int j = stk_p->getCOL_INDEX();	
	
	if(!SUBSTKS_DESCRIPTORS || !SUBSTKS_DESCRIPTORS[i] || !SUBSTKS_DESCRIPTORS[i][j])
	{
		char err_msg[1000];
		sprintf(err_msg, "in StackRestorer::computeStackDescriptors(Stack[%d,%d]): no computed subvol descriptors found.\n", i, j);
		throw MyException(err_msg);
	}

	int D_dim_acc = 0;
	for(int k=0; k< SD_DIM_k; k++)
		if(SUBSTKS_DESCRIPTORS[i][j][k].isSubvolDescriptor() && SUBSTKS_DESCRIPTORS[i][j][k].isFinalized())
			D_dim_acc+= SUBSTKS_DESCRIPTORS[i][j][k].D_dim;
		else
		{
			char err_msg[1000];
			sprintf(err_msg, "in StackRestorer::computeStackDescriptors(Stack[%d,%d]): no computed subvol descriptors found.\n", i, j);
			throw MyException(err_msg);
		}

	STKS_DESCRIPTORS[i][j].init(SUBSTKS_DESCRIPTORS[i][j][0].stk_p, false, SUBSTKS_DESCRIPTORS[i][j][0].V_dim, SUBSTKS_DESCRIPTORS[i][j][0].H_dim, D_dim_acc);
	STKS_DESCRIPTORS[i][j].computeStackDescriptors(SUBSTKS_DESCRIPTORS[i][j], SD_DIM_k);
}

void StackRestorer::finalizeAllDescriptors()
{
	for(int i=0; i<N_ROWS; i++)
		for(int j=0; j<N_COLS; j++)
			computeStackDescriptors(STK_ORG->getSTACKS()[i][j]);
}

void StackRestorer::printSubvolDescriptors(Stack* stk_p, int subvol_idx)
{
	int i = stk_p->getROW_INDEX();
	int j = stk_p->getCOL_INDEX();
	SUBSTKS_DESCRIPTORS[i][j][subvol_idx].print();
}

void StackRestorer::printVolDescriptors(Stack* stk_p)
{
	int i = stk_p->getROW_INDEX();
	int j = stk_p->getCOL_INDEX();
	STKS_DESCRIPTORS[i][j].print();
}

void StackRestorer::save(char* file_path)  throw (MyException)
{
	if(STKS_DESCRIPTORS)
	{
		//some checks
		bool ready_to_save = true;
		for(int i=0; i<N_ROWS && ready_to_save; i++)
			for(int j=0; j<N_COLS && ready_to_save; j++)
				ready_to_save = (!STKS_DESCRIPTORS[i][j].isSubvolDescriptor()) && STKS_DESCRIPTORS[i][j].isFinalized();
		if(!ready_to_save)
		{
			char err_msg[1000];
			sprintf(err_msg, "in StackRestorer::save(file_path[%s]): Stack descriptors not found.\n", file_path);
			throw MyException(err_msg);
		}

		//saving procedure
		std::ofstream file(file_path, std::ios::out | std::ios::binary);
		//int ID = STK_ORG->getID();
		//file.write((char*)&ID, sizeof(int));
		file.write((char*)&N_ROWS, sizeof(int));
		file.write((char*)&N_COLS, sizeof(int));
		for(int i=0; i<N_ROWS; i++)
			for(int j=0; j<N_COLS; j++)
			{
				file.write((char*)&(STKS_DESCRIPTORS[i][j].max_val), sizeof(real_t));
				file.write((char*)&(STKS_DESCRIPTORS[i][j].mean_val), sizeof(real_t));
				file.write((char*)&(STKS_DESCRIPTORS[i][j].V_dim), sizeof(int));
				file.write((char*)&(STKS_DESCRIPTORS[i][j].H_dim), sizeof(int));
				file.write((char*)&(STKS_DESCRIPTORS[i][j].D_dim), sizeof(int));
				for(int ii=0; ii<STKS_DESCRIPTORS[i][j].V_dim; ii++)
					file.write((char*)&(STKS_DESCRIPTORS[i][j].V_profile[ii]), sizeof(real_t));
				for(int ii=0; ii<STKS_DESCRIPTORS[i][j].H_dim; ii++)
					file.write((char*)&(STKS_DESCRIPTORS[i][j].H_profile[ii]), sizeof(real_t));
				for(int ii=0; ii<STKS_DESCRIPTORS[i][j].D_dim; ii++)
					file.write((char*)&(STKS_DESCRIPTORS[i][j].D_profile[ii]), sizeof(real_t));
				for(int ii=0; ii<STKS_DESCRIPTORS[i][j].V_dim; ii++)
					file.write((char*)&(STKS_DESCRIPTORS[i][j].V_MIP[ii]), sizeof(real_t));
				for(int ii=0; ii<STKS_DESCRIPTORS[i][j].H_dim; ii++)
					file.write((char*)&(STKS_DESCRIPTORS[i][j].H_MIP[ii]), sizeof(real_t));
				for(int ii=0; ii<STKS_DESCRIPTORS[i][j].D_dim; ii++)
					file.write((char*)&(STKS_DESCRIPTORS[i][j].D_MIP[ii]), sizeof(real_t));
			}

		file.close();
	}
	else
	{
		char err_msg[1000];
		sprintf(err_msg, "in StackRestorer::save(file_path[%s]): Stack descriptors not found.\n", file_path);
		throw MyException(err_msg);
	}
}

void StackRestorer::load(char* file_path)  throw (MyException)
{
	std::ifstream file (file_path, std::ios::in | std::ios::binary);
	if(!file)
	{
		char err_msg[1000];
		sprintf(err_msg, "in StackRestorer::load(file_path[%s]): unable to load given file.\n", file_path);
		throw MyException(err_msg);
	}
	int new_N_ROWS=-1, new_N_COLS=-1;
	file.read((char*)&new_N_ROWS, sizeof(int));
	file.read((char*)&new_N_COLS, sizeof(int));

	if(new_N_ROWS != N_ROWS || new_N_COLS != new_N_COLS)
	{
		file.close();
		char err_msg[1000];
		sprintf(err_msg, "in StackRestorer::load(file_path[%s]): the description file refers to a different acquisition.\n", file_path);
		throw MyException(err_msg);
	}

	for(int i=0; i<N_ROWS; i++)
		for(int j=0; j<N_COLS; j++)
		{
			file.read((char*)&(STKS_DESCRIPTORS[i][j].max_val), sizeof(real_t));
			file.read((char*)&(STKS_DESCRIPTORS[i][j].mean_val), sizeof(real_t));
			file.read((char*)&(STKS_DESCRIPTORS[i][j].V_dim), sizeof(int));
			file.read((char*)&(STKS_DESCRIPTORS[i][j].H_dim), sizeof(int));
			file.read((char*)&(STKS_DESCRIPTORS[i][j].D_dim), sizeof(int));

			STKS_DESCRIPTORS[i][j].stk_p=STK_ORG->getSTACKS()[i][j];
			STKS_DESCRIPTORS[i][j].V_profile = new real_t[STKS_DESCRIPTORS[i][j].V_dim];
			STKS_DESCRIPTORS[i][j].H_profile = new real_t[STKS_DESCRIPTORS[i][j].H_dim];
			STKS_DESCRIPTORS[i][j].D_profile = new real_t[STKS_DESCRIPTORS[i][j].D_dim];
			STKS_DESCRIPTORS[i][j].V_MIP	 = new real_t[STKS_DESCRIPTORS[i][j].V_dim];
			STKS_DESCRIPTORS[i][j].H_MIP	 = new real_t[STKS_DESCRIPTORS[i][j].H_dim];
			STKS_DESCRIPTORS[i][j].D_MIP	 = new real_t[STKS_DESCRIPTORS[i][j].D_dim];

			for(int ii=0; ii<STKS_DESCRIPTORS[i][j].V_dim; ii++)
				file.read((char*)&(STKS_DESCRIPTORS[i][j].V_profile[ii]), sizeof(real_t));
			for(int ii=0; ii<STKS_DESCRIPTORS[i][j].H_dim; ii++)
				file.read((char*)&(STKS_DESCRIPTORS[i][j].H_profile[ii]), sizeof(real_t));
			for(int ii=0; ii<STKS_DESCRIPTORS[i][j].D_dim; ii++)
				file.read((char*)&(STKS_DESCRIPTORS[i][j].D_profile[ii]), sizeof(real_t));
			for(int ii=0; ii<STKS_DESCRIPTORS[i][j].V_dim; ii++)
				file.read((char*)&(STKS_DESCRIPTORS[i][j].V_MIP[ii]), sizeof(real_t));
			for(int ii=0; ii<STKS_DESCRIPTORS[i][j].H_dim; ii++)
				file.read((char*)&(STKS_DESCRIPTORS[i][j].H_MIP[ii]), sizeof(real_t));
			for(int ii=0; ii<STKS_DESCRIPTORS[i][j].D_dim; ii++)
				file.read((char*)&(STKS_DESCRIPTORS[i][j].D_MIP[ii]), sizeof(real_t));
			STKS_DESCRIPTORS[i][j].is_subvol_descriptor = false;
			STKS_DESCRIPTORS[i][j].is_finalized = true;
		}
	file.close();
}

void StackRestorer::repairSlice(real_t* data, int slice_idx, Stack* stk_p, int direction)  throw (MyException)
{
	//some checks
	bool ready_to_repair = true;
	for(int i=0; i<N_ROWS && ready_to_repair; i++)
		for(int j=0; j<N_COLS && ready_to_repair; j++)
			ready_to_repair = STKS_DESCRIPTORS && (!STKS_DESCRIPTORS[i][j].isSubvolDescriptor()) && STKS_DESCRIPTORS[i][j].isFinalized();
	if(!ready_to_repair)
	{
		char err_msg[1000];
		sprintf(err_msg, "in repairSlice::repairSlice(...): Stack descriptors not found.\n");
		throw MyException(err_msg);
	}
	if(direction != S_RESTORE_V_DIRECTION && direction != S_RESTORE_H_DIRECTION)
	{
		char err_msg[1000];
		sprintf(err_msg, "in repairSlice::repairSlice(...): selected restoring direction (%d) is not supported.\n", direction);
		throw MyException(err_msg);
	}

	int i, j;
	real_t *p_scanpxl = data;
	real_t max_val = 0;
	real_t corr_fact;
	int row   = stk_p->getROW_INDEX();
	int col	  = stk_p->getCOL_INDEX();
	int v_dim = stk_p->getHEIGHT();
	int h_dim = stk_p->getWIDTH();
	vol_descr_t *vol_desc = &(STKS_DESCRIPTORS[row][col]);
	real_t *V_profile = vol_desc->V_profile;
	real_t *H_profile = vol_desc->H_profile;
	real_t *V_MIP	  = vol_desc->V_MIP;
	real_t *H_MIP	  = vol_desc->H_MIP;

	switch ( direction ) 
	{
		case S_RESTORE_V_DIRECTION:
		{
			//correct pixels
			for ( i=0; i<v_dim; i++ )
				for ( j=0; j<h_dim; j++, p_scanpxl++ )
					(*p_scanpxl) /= V_profile[i];

			//computing new max_val
			for ( i=0; i<v_dim; i++ )
				if(V_MIP[i] / V_profile[i] > max_val)
					max_val = V_MIP[i] / V_profile[i];
			break;
		}
		case S_RESTORE_H_DIRECTION:
		{
			//correct pixels
			for ( i=0; i<v_dim; i++ )
				for ( j=0; j<h_dim; j++, p_scanpxl++ )
					(*p_scanpxl) /= H_profile[j];

			//computing new max_val
			for ( i=0; i<h_dim; i++ )
				if(H_MIP[i] / H_profile[i] > max_val)
					max_val = H_MIP[i] / H_profile[i];
			break;
		}
	}

	// rescale Stack 
	corr_fact = ISR_MIN((vol_desc->mean_val),(1 / max_val));
	p_scanpxl = data;
	for ( i=0; i<v_dim; i++ )
		for ( j=0; j<h_dim; j++, p_scanpxl++ )
			(*p_scanpxl) *= corr_fact;

	#ifdef S_RESTORE_VZ
	real_t *D_profile = vol_desc->D_profile;
	p_scanpxl = data;
	for ( i=0; i<v_dim; i++ )
		for ( j=0; j<h_dim; j++, p_scanpxl++ )
			(*p_scanpxl) = ((*p_scanpxl)/D_profile[slice_idx])*vol_desc->mean_val;
	#endif
}

void StackRestorer::repairStack(real_t* data, Stack* stk_p, int direction)  throw (MyException)
{
	//some checks
	bool ready_to_repair = true;
	for(int i=0; i<N_ROWS && ready_to_repair; i++)
		for(int j=0; j<N_COLS && ready_to_repair; j++)
			ready_to_repair = STKS_DESCRIPTORS && (!STKS_DESCRIPTORS[i][j].isSubvolDescriptor()) && STKS_DESCRIPTORS[i][j].isFinalized();
	if(!ready_to_repair)
	{
		char err_msg[1000];
		sprintf(err_msg, "in repairSlice::repairStack(...): Stack descriptors not found.\n");
		throw MyException(err_msg);
	}	
	if(direction != S_RESTORE_V_DIRECTION && direction != S_RESTORE_H_DIRECTION)
	{
		char err_msg[1000];
		sprintf(err_msg, "in repairSlice::repairSlice(...): selected restoring direction (%d) is not supported.\n", direction);
		throw MyException(err_msg);
	}

	int i, j, k;
	real_t *p_scanpxl;
	real_t max_val = 0;
	real_t corr_fact;
	int row   = stk_p->getROW_INDEX();
	int col	  = stk_p->getCOL_INDEX();
	int v_dim = stk_p->getHEIGHT();
	int h_dim = stk_p->getWIDTH();
	int d_dim = stk_p->getDEPTH();
	vol_descr_t *vol_desc = &(STKS_DESCRIPTORS[row][col]);
	real_t *V_profile = vol_desc->V_profile;
	real_t *H_profile = vol_desc->H_profile;
	real_t *V_MIP	  = vol_desc->V_MIP;
	real_t *H_MIP	  = vol_desc->H_MIP;

	switch ( direction ) 
	{
		case S_RESTORE_V_DIRECTION:
		{
			//correct pixels
			for( k=0; k<d_dim; k++)
				for ( i=0; i<v_dim; i++ )
					for ( j=0; j<h_dim; j++, p_scanpxl++ )
						(*p_scanpxl) /= V_profile[i];

			//computing new max_val
			for(int k=0; k<d_dim; k++)
				for ( i=0; i<v_dim; i++ )
					if(V_MIP[i] / V_profile[i] > max_val)
						max_val = V_MIP[i] / V_profile[i];
			break;
		}
		case S_RESTORE_H_DIRECTION:
		{
			//correct pixels
			for( k=0; k<d_dim; k++)
				for ( i=0; i<v_dim; i++ )
					for ( j=0; j<h_dim; j++, p_scanpxl++ )
						(*p_scanpxl) /= H_profile[j];

			//computing new max_val
			for(int k=0; k<d_dim; k++)
				for ( i=0; i<h_dim; i++ )
					if(H_MIP[i] / H_profile[i] > max_val)
						max_val = H_MIP[i] / H_profile[i];
			break;
		}
	}

	// rescale Stack 
	corr_fact = ISR_MIN((vol_desc->mean_val),(1 / max_val));
	p_scanpxl = data;
	for( k=0; k<d_dim; k++)
		for ( i=0; i<v_dim; i++ )
			for ( j=0; j<h_dim; j++, p_scanpxl++ )
				(*p_scanpxl) *= corr_fact;
}

/******************************************************
 ********** SUBSTACK_DESCR_T DEFINITIONS **************
 ******************************************************/
StackRestorer::vol_descr_t::vol_descr_t()
{
	this->stk_p = NULL;

	this->max_val = 0;
	this->mean_val = 0;
	this->V_dim = -1;
	this->H_dim = -1;
	this->D_dim = -1;

	this->V_profile = NULL;
	this->H_profile = NULL;
	this->D_profile = NULL;

	this->V_MIP = NULL;
	this->H_MIP = NULL;
	this->D_MIP = NULL;

	this->is_subvol_descriptor = false;
	this->is_finalized = false;
}

void StackRestorer::vol_descr_t::init(Stack *new_stk_p, bool is_subvol_desc, int new_V_dim, int new_H_dim, int new_D_dim)
{
	this->stk_p = new_stk_p;

	this->V_dim = new_V_dim;
	this->H_dim = new_H_dim;
	this->D_dim = new_D_dim;

	this->V_profile = new real_t[V_dim];
	this->H_profile = new real_t[H_dim];
	this->D_profile = new real_t[D_dim];

	this->V_MIP = new real_t[V_dim];
	this->H_MIP = new real_t[H_dim];
	this->D_MIP = new real_t[D_dim];

	this->is_subvol_descriptor = is_subvol_desc;
}

StackRestorer::vol_descr_t::~vol_descr_t()
{
	if(V_profile)
		delete[] V_profile;
	if(H_profile)
		delete[] H_profile;
	if(D_profile)
		delete[] D_profile;
	if(V_MIP)
		delete[] V_MIP;
	if(H_MIP)
		delete[] H_MIP;
	if(D_MIP)
		delete[] D_MIP;
}

void StackRestorer::vol_descr_t::computeSubvolDescriptors(real_t *subvol)  throw (MyException)
{
	//initialization of MIPs and profiles
	for(int i=0; i<V_dim; i++)
		V_MIP[i] = V_profile[i] = 0;
	for(int j=0; j<H_dim; j++)
		H_MIP[j] = H_profile[j] = 0;
	for(int k=0; k<D_dim; k++)
		D_MIP[k] = D_profile[k] = 0;

	//one-shot computing of accumulation profiles and MIPs
	real_t *p_scanpxl = subvol;
	real_t val;
	for ( int k=0; k<D_dim; k++ )
		for ( int i=0; i<V_dim; i++ )
			for ( int j=0; j<H_dim; j++, p_scanpxl++ ) 
			{
				val = *p_scanpxl;

				//accumulations
				V_profile[i] += val;
				H_profile[j] += val;
				D_profile[k] += val;

				//maximum selection for maximum intensity projections
				if(val > V_MIP[i])
					V_MIP[i] = val;
				if(val > H_MIP[j])
					H_MIP[j] = val;
				if(val > D_MIP[k])
					D_MIP[k] = val;
			}

	//computing max value as the maximum of maxs
	for(int i=0; i<V_dim; i++)
		if(V_MIP[i] > max_val)
			max_val = V_MIP[i];
	for(int j=0; j<H_dim; j++)
		if(H_MIP[j] > max_val)
			max_val = H_MIP[j];
	for(int k=0; k<D_dim; k++)
		if(D_MIP[k] > max_val)
			max_val = D_MIP[k];

	this->is_finalized = true;
}


void StackRestorer::vol_descr_t::computeStackDescriptors(vol_descr_t *subvol_desc, int D_subvols)  throw (MyException)
{
	//initialization of MIPs and profiles (that are mean intensity projections)
	for(int i=0; i<V_dim; i++)
		V_MIP[i] = V_profile[i] = 0;
	for(int j=0; j<H_dim; j++)
		H_MIP[j] = H_profile[j] = 0;
	for(int k=0; k<D_dim; k++)
		D_MIP[k] = D_profile[k] = 0;

	//one-shot computing of accumulation profiles and MIPs
	int d_acc=0;
	for(int k=0; k<D_subvols; k++)
	{
		for(int i=0; i<V_dim; i++)
		{
			V_profile[i]+=subvol_desc[k].V_profile[i];
			if(subvol_desc[k].V_MIP[i] > V_MIP[i])
				V_MIP[i] = subvol_desc[k].V_MIP[i];
		}
		for(int j=0; j<H_dim; j++)
		{
			H_profile[j]+=subvol_desc[k].H_profile[j];
			if(subvol_desc[k].H_MIP[j] > H_MIP[j])
				H_MIP[j] = subvol_desc[k].H_MIP[j];
		}
		for(int kk=d_acc; kk<d_acc+subvol_desc[k].D_dim; kk++)
		{
			D_profile[kk] = subvol_desc[k].D_profile[kk-d_acc];
			D_MIP[kk]     = subvol_desc[k].D_MIP[kk-d_acc];
		}
		d_acc+=subvol_desc[k].D_dim;
	}

	//refining mean intensity projections
	for ( int i=0; i<V_dim; i++ ) 
	{
		mean_val+=V_profile[i];
		V_profile[i] /= D_dim*H_dim;
		if ( V_profile[i] == 0 )
			V_profile[i] = S_MIN_PROF_VAL;
	}
	for ( int j=0; j<H_dim; j++ )
	{
		H_profile[j] /= D_dim*V_dim;
		if ( H_profile[j] == 0 )
			H_profile[j] = S_MIN_PROF_VAL;
	}
	for ( int k=0; k<D_dim; k++ ) 
	{
		D_profile[k] /= V_dim*H_dim;
		if ( D_profile[k] == 0 )
			D_profile[k] = S_MIN_PROF_VAL;
	}

	//computing mean value
	mean_val /= (D_dim*H_dim*V_dim);

	//computing max value as the maximum of maxs
	for(int k=0; k<D_subvols; k++)
		if(subvol_desc[k].max_val > max_val)
			max_val = subvol_desc[k].max_val;

	this->is_finalized = true;
}

void StackRestorer::vol_descr_t::print()
{
	printf("\tvol_decr[%d,%d] PRINTING:\n", stk_p->getROW_INDEX(), stk_p->getCOL_INDEX());
	printf("\tDIMS = %d(V) x %d(H) x %d(D)\n", V_dim, H_dim, D_dim);
	printf("\tmean = %.5f\n", mean_val);
	printf("\tmax  = %.5f\n", max_val);
	printf("\tarray data:\n\n");
	for(int i=0; i<V_dim; i++)
		printf("\t\tV_profile[%d] = %.5f\n", i, V_profile[i]);
	for(int i=0; i<H_dim; i++)
		printf("\t\tH_profile[%d] = %.5f\n", i, H_profile[i]);
	for(int i=0; i<D_dim; i++)
		printf("\t\tD_profile[%d] = %.5f\n", i, D_profile[i]);
	for(int i=0; i<V_dim; i++)
		printf("\t\tV_MIP[%d] = %.5f\n", i, V_MIP[i]);
	for(int i=0; i<H_dim; i++)
		printf("\t\tH_MIP[%d] = %.5f\n", i, H_MIP[i]);
	for(int i=0; i<D_dim; i++)
		printf("\t\tD_MIP[%d] = %.5f\n", i, D_MIP[i]);
	printf("\n\n");
}
