#include "Point.h"

/**
  Rewritten in C++ for BigNeuron Hackathon.
  Edward Hottendorf

  Modification made by Jie Zhou for porting to Vaa3D.
  June 2015

**/

/**
* AnalyzeSkeleton_ plugin for ImageJ(C).
* Copyright (C) 2008,2009 Ignacio Arganda-Carreras
*
* This program is free software; you can redistribute it and/or
* modify it under the terms of the GNU General Public License
* as published by the Free Software Foundation (http://www.gnu.org/licenses/gpl.txt )
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program; if not, write to the Free Software
* Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*
*/



Point::Point(){
}


Point::Point(int newX, int newY, int newZ)
{
	this->x = newX;
	this->y = newY;
	this->z = newZ;
}

