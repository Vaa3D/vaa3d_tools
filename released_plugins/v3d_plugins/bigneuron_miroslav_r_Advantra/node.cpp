/*
Copyright (C) Erasmus MC. Permission to use this software and corresponding documentation for educational, research, and not-for-profit purposes, without a fee and without a signed licensing agreement, is granted, subject to the following terms and conditions.

IT IS NOT ALLOWED TO REDISTRIBUTE, SELL, OR LEASE THIS SOFTWARE, OR DERIVATIVE WORKS THEREOF, WITHOUT PERMISSION IN WRITING FROM THE COPYRIGHT HOLDER. THE COPYRIGHT HOLDER IS FREE TO MAKE VERSIONS OF THE SOFTWARE AVAILABLE FOR A FEE OR COMMERCIALLY ONLY.

IN NO EVENT SHALL THE COPYRIGHT HOLDER BE LIABLE TO ANY PARTY FOR DIRECT, INDIRECT, SPECIAL, INCIDENTAL, OR CONSEQUENTIAL DAMAGES, OF ANY KIND WHATSOEVER, ARISING OUT OF THE USE OF THIS SOFTWARE AND ITS DOCUMENTATION, EVEN IF ADVISED OF THE POSSIBILITY THEREOF.

THE COPYRIGHT HOLDER SPECIFICALLY DISCLAIMS ANY WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE EXPRESS OR IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE. THE SOFTWARE AND CORRESPONDING DOCUMENTATION IS PROVIDED "AS IS". THE COPYRIGHT HOLDER HAS NO OBLIGATION TO PROVIDE MAINTENANCE, SUPPORT, UPDATES, ENHANCEMENTS, OR MODIFICATIONS.
 */

#include "node.h"
#include <cfloat>

int Node::NOTHING = 0;
int Node::SOMA = 1;
int Node::AXON = 2;
int Node::BASAL_DENDRITE = 3;
int Node::APICAL_DENDRITE = 4;
int Node::FORK = 5;
int Node::END = 6;
int Node::UNDEFINED = 7;

Node::Node(float xn, float yn, float zn, float rn, int typ)
{
    x = xn;
    y = yn;
    z = zn;
    r = rn;
    vx = -FLT_MAX;
    vy = -FLT_MAX;
    vz = -FLT_MAX;
    corr = -FLT_MAX;
    type = typ;
}

Node::Node(float xn, float yn, float zn, float vx1, float vy1, float vz1, float corr1, float rn, int typ)
{
    x = xn;
    y = yn;
    z = zn;
    r = rn;
    vx = vx1;
    vy = vy1;
    vz = vz1;
    corr = corr1;
    type = typ;
}

Node::~Node(){}
