//-------------------------------------------------------------------------------------------------------
// VST Plug-Ins SDK
//
// Category     : VST 2.x SDK Compressore
// Filename     : Compzocchio.cpp
// Created by   : Mattia Mazzocchio
// Description  : Compressore
//
//-------------------------------------------------------------------------------------------------------

#ifndef __Compzocchio__
#include "Compzocchio.h"
#endif

//-------------------------------------------------------------------------------------------------------
AudioEffect* createEffectInstance (audioMasterCallback audioMaster)
{
	return new Compzocchio (audioMaster);
}

