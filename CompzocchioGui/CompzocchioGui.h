//-------------------------------------------------------------------------------------------------------
// VST Plug-Ins SDK
//
// Category     : VST 2.x SDK Compressore
// Filename     : CompzocchioGui.h
// Created by   : Mattia Mazzocchio
// Description  : Compressore mono
//
//-------------------------------------------------------------------------------------------------------

#ifndef __CompzocchioGui__
#define __CompzocchioGui__

#ifndef __Compzocchio__
#include "Compzocchio.h"
#endif

//------------------------------------------------------------------------
// CompzocchioGui declaration
//------------------------------------------------------------------------
class CompzocchioGui : public Compzocchio
{
public:
	CompzocchioGui (audioMasterCallback audioMaster);

	virtual void processReplacing (float** inputs, float** outputs, VstInt32 sampleframes);

	void setParameter (VstInt32 index, float value);

	// functions VST version 2
	virtual bool getVendorString (char* text) { if (text) strcpy (text, "Mattia Mazzocchio"); return true; }
	virtual bool getProductString (char* text) { if (text) strcpy (text, "CompzocchioGui"); return true; }
	virtual VstInt32 getVendorVersion () { return 2000; }

	virtual void resume ();
};

#endif
