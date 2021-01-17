//-------------------------------------------------------------------------------------------------------
// VST Plug-Ins SDK
//
// Category     : VST 2.x SDK Compressore
// Filename     : CompzocchioGui.cpp
// Created by   : Mattia Mazzocchio
// Description  : Compressore mono
//
//-------------------------------------------------------------------------------------------------------

#ifndef __CompzocchioGui__
#include "CompzocchioGui.h"
#endif

#ifndef __czeditor__
#include "editor/czeditor.h"
#endif

#include <string.h>
#include <stdio.h>

//-------------------------------------------------------------------------------------------------------
AudioEffect* createEffectInstance (audioMasterCallback audioMaster)
{
	return new CompzocchioGui (audioMaster);
}

//-----------------------------------------------------------------------------
CompzocchioGui::CompzocchioGui (audioMasterCallback audioMaster)
: Compzocchio (audioMaster)
{
	setUniqueID ('CzoG');	// this should be unique, use the Steinberg web page for plugin Id registration

	// create the editor
	editor = new CZEditor (this);

	resume ();
}

//------------------------------------------------------------------------
void CompzocchioGui::resume ()
{

}

//------------------------------------------------------------------------
void CompzocchioGui::processReplacing (float** inputs, float** outputs, VstInt32 sampleframes)
{
	Compzocchio::processReplacing (inputs, outputs, sampleframes);
}

//-----------------------------------------------------------------------------
void CompzocchioGui::setParameter (VstInt32 index, float value)
{
	Compzocchio::setParameter (index, value);

	if (editor)
		((AEffGUIEditor*)editor)->setParameter (index, value);
}
