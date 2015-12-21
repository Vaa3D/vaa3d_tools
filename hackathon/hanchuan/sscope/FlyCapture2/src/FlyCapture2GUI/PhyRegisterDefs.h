//=============================================================================
// Copyright © 2008 Point Grey Research, Inc. All Rights Reserved.
//
// This software is the confidential and proprietary information of Point
// Grey Research, Inc. ("Confidential Information").  You shall not
// disclose such Confidential Information and shall use it only in
// accordance with the terms of the license agreement you entered into
// with PGR.
//
// PGR MAKES NO REPRESENTATIONS OR WARRANTIES ABOUT THE SUITABILITY OF THE
// SOFTWARE, EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
// IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR
// PURPOSE, OR NON-INFRINGEMENT. PGR SHALL NOT BE LIABLE FOR ANY DAMAGES
// SUFFERED BY LICENSEE AS A RESULT OF USING, MODIFYING OR DISTRIBUTING
// THIS SOFTWARE OR ITS DERIVATIVES.
//=============================================================================

#ifndef _PHYREGISTERDEFS_H_	
#define _PHYREGISTERDEFS_H_

#include "FlyCapture2.h"

namespace FlyCapture2
{

    const unsigned int k_basePage = 0;
    const unsigned int k_portPage = 0;
    const unsigned int k_vendorPage = 1;
    const unsigned int k_totalPortsField = 7;

    typedef struct FieldType
    {
        Glib::ustring name;
        unsigned int id;
    } Field;

    typedef struct RegisterType
    {
        char name[64];
        unsigned int addr;
        unsigned char mask;
    } Register;

    typedef struct NodeDescType
    {
        PGRGuid guid;
        Glib::ustring name;
    } NodeDesc;

    const Field k_vendorList[] = {
        { "Point Grey Research", 0x00B09D },
        { "Texas Instruments",   0x080028 }, 
        { "LSI/Agere Systems",   0x00053D },
        { "Lucent Technologies", 0x00601D },
        { "Node",                0x000000 }		// unknown vendor
    };

    const Register k_baseRegisterInfo[] = 
    {
        { "Physical_ID",           0, 0xFC },
        { "R",                     0, 0x02 },
        { "PS",                    0, 0x01 },
        { "RHB",                   1, 0x80 },
        { "IBR",                   1, 0x40 },
        { "Gap_count",             1, 0x3F },
        { "Extended",              2, 0xE0 },
        { "Total_Ports",		   2, 0x1F },
        { "Max_speed",             3, 0xE0 },
        { "Enable_standby",        3, 0x10 },
        { "Delay",                 3, 0x0F },
        { "LCtrl",                 4, 0x80 },
        { "Contender",             4, 0x40 },
        { "Jitter",                4, 0x38 },
        { "Pwr_class",             4, 0x07 },
        { "Watchdog",              5, 0x80 },
        { "ISBR",                  5, 0x40 },
        { "Loop",                  5, 0x20 },
        { "Pwr_fail",              5, 0x10 },
        { "Timeout",               5, 0x08 },
        { "Port_event",            5, 0x04 },
        { "Enab_accel",            5, 0x02 },
        { "Enab_multi",            5, 0x01 },
        { "Max_legacy_path_speed", 6, 0xE0 },
        { "B_link",                6, 0x10 },
        { "Bridge",                6, 0x0C },
        { "Page_select",           7, 0xE0 },
        { "Port_select",           7, 0x0F }
    };

    const Register k_portStatusInfo[] = {
        { "AStat",                 8,  0xC0 },
        { "BStat",                 8,  0x30 },
        { "Child",                 8,  0x08 },
        { "Connected",             8,  0x04 },
        { "Receive_OK",            8,  0x02 },
        { "Disabled",              8,  0x01 },
        { "Negotiated_speed",      9,  0xE0 },
        { "Int_enable",            9,  0x10 },
        { "Fault",                 9,  0x08 },
        { "Standby_fault",         9,  0x04 },
        { "Disable_scrambler",     9,  0x02 },
        { "Beta_mode_only_port",   9,  0x01 },
        { "DC_connected",          10, 0x80 },
        { "Max_port_speed",        10, 0x70 },
        { "LPP",                   10, 0x08 },
        { "Cable_speed",           10, 0x07 },
        { "Connection_unreliable", 11, 0x80 },
        { "Beta_mode",             11, 0x08 },
        { "Port_error",            12, 0xFF },
        { "Loop_diable",           13, 0x04 },
        { "In_standby",            13, 0x02 },
        { "Hard_disable",          13, 0x01 }
    };
}
#endif
