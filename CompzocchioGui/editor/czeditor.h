//-------------------------------------------------------------------------------------------------------
// VST Plug-Ins SDK
//
// Category     : VST 2.x SDK Compressore
// Filename     : Compzocchio.cpp
// Created by   : Mattia Mazzocchio
// Description  : Compressore mono
//
//-------------------------------------------------------------------------------------------------------

#ifndef __czeditor__
#define __czeditor__

// include VSTGUI
#ifndef __vstgui__
#include "vstgui.sf/vstgui/vstgui.h"
#endif

//-----------------------------------------------------------------------------
class CZEditor : public AEffGUIEditor, public CControlListener
{
public:
	CZEditor (AudioEffect* effect);
	virtual ~CZEditor ();

public:
	virtual bool open (void* ptr);
	virtual void close ();

	virtual void setParameter (VstInt32 index, float value);
	virtual void valueChanged (CDrawContext* context, CControl* control);
	virtual void idle();

private:
	//Controls
    CAnimKnob *RelTKnob;
    CAnimKnob *AtkTKnob;
    CAnimKnob *CthKnob;
    CAnimKnob *CompFKnob;
	CAnimKnob *KneeKnob;
    CAnimKnob *OutGKnob;
    CAnimKnob *LthKnob;

	CParamDisplay* CthDisplay;
	CParamDisplay* AtkTDisplay;
	CParamDisplay* RelTDisplay;
	CParamDisplay* CompFDisplay;
	CParamDisplay* KneeDisplay;
	CParamDisplay* OutGDisplay;
	CParamDisplay* LthDisplay;

	CMovieBitmap *GRedVu;
	CMovieBitmap *OnOffLamp;

	COnOffButton *OnOffSwitch;

	// Labels
    CTextLabel  *PluginVersion;

    // Bitmap
	CBitmap* hBackground;
    CBitmap *hknobs;
	CBitmap *hmeter_animation;
	CBitmap *hswitch;
	CBitmap *hlamp;

	float OnOffLampMeter;
};

#endif
