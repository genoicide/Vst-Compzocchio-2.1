//------------------------------------------------------------------------------------------------------
// VST Plug-Ins SDK
//
// Category     : VST 2.x SDK Compressore
// Filename     : Compzocchio.cpp
// Created by   : Mattia Mazzocchio
// Description  : Compressore mono
//
//------------------------------------------------------------------------------------------------------

#ifndef __czeditor__
#include "czeditor.h"
#endif

#ifndef __Compzocchio__
#include "../Compzocchio.h"
#endif

#include <stdio.h>
#include <math.h>

//-----------------------------------------------------------------------------
// Id univoci degli oggetti grafici
enum {
	// Bitmaps
	kBackgroundId = 128, //Immagine background 128
	kknobs, //Immagine manopola potenziometri
	kmeter_animation, //Immagine VU Meter
	kswitch, //Immagine Switch On/Off
	klamp, //Immagine Lampada On/Off
};

//-----------------------------------------------------------------------------
// Funzioni di conversione per display
void TresholdConvert (float value, char* string);
void TresholdConvert (float value, char* string)
{
	if (value <= 0)
		vst_strncpy (string, "-oo", 5);
	else
		sprintf (string, "%2.1f", 20 * log10(value) );
}

void AtkTConvert (float value, char* string);
void AtkTConvert (float value, char* string)
{
	int sizeAtkT = 49;
	sprintf (string, "%2.0f", value * sizeAtkT + 1);
}

void RelTConvert (float value, char* string);
void RelTConvert (float value, char* string)
{
	int sizeRelT = 499;
	sprintf (string, "%2.0f", value * sizeRelT + 1);
}

void CompFConvert (float value, char* string);
void CompFConvert (float value, char* string)
{
	int sizeCompF = 9;
	sprintf (string, "%2.1f", value * sizeCompF + 1);
}

void KneeConvert (float value, char* string);
void KneeConvert (float value, char* string)
{
	float sizeKnee = 12;
	sprintf (string, "%2.1f", value * sizeKnee);
}

void OutGConvert (float value, char* string);
void OutGConvert (float value, char* string)
{
	float sizeOutG = 24;
	sprintf (string, "%2.1f", (value * sizeOutG - 12));
}

//-----------------------------------------------------------------------------
// CZEditor class implementation
//-----------------------------------------------------------------------------
CZEditor::CZEditor (AudioEffect *effect)
 : AEffGUIEditor (effect) 
{
	// Init
	RelTKnob = 0;
    AtkTKnob = 0;
    CthKnob = 0;
    CompFKnob = 0;
	KneeKnob = 0;
    OutGKnob = 0;
    LthKnob = 0;
	GRedVu = 0;
	OnOffSwitch = 0;
	OnOffLamp = 0;

	OnOffLampMeter = 1;

	hknobs = 0;
	hmeter_animation = 0;
	hswitch = 0;
	hlamp = 0;

    // Caricamento dell'immagine di sfondo
	hBackground = new CBitmap (kBackgroundId);

	// Inizializzazione della dimensione della GUI
	rect.left   = 0;
	rect.top    = 0;
	rect.right  = (short)hBackground->getWidth ();
	rect.bottom = (short)hBackground->getHeight ();
}

//-----------------------------------------------------------------------------
CZEditor::~CZEditor ()
{
	if (hBackground)
		hBackground->forget ();
	hBackground = 0;
}

//-----------------------------------------------------------------------------
bool CZEditor::open (void *ptr)
{
	// !!! always call this !!!
	AEffGUIEditor::open (ptr);
	
	// Caricamento delle immagini necessarie alla GUI
	hknobs = new CBitmap(kknobs);
	hmeter_animation = new CBitmap(kmeter_animation);
	hswitch = new CBitmap(kswitch);
	hlamp = new CBitmap(klamp);

	//--Init background frame-----------------------------------------------
	// We use a local CFrame object so that calls to setParameter won't call into objects which may not exist yet. 
	// If all GUI objects are created we assign our class member to this one. See bottom of this method.
	CRect size (0, 0, hBackground->getWidth (), hBackground->getHeight ());
	CFrame* lFrame = new CFrame (size, ptr, this);
	lFrame->setBackground (hBackground);

	//--Init Oggetti grafici GUI----------------------------------------------

	CPoint point,xy; //Offset e posizionamento degli oggetti
	CCoord w, h; //Larghezza e altezza degli oggetti

	point(0, 0);

	//--OnOffSwitch--------
	w = hswitch->getWidth();
    h = hswitch->getHeight() / 2;   // 2 sottoimmagini
	xy (179, 67);
    size(xy.x, xy.y, xy.x + w, xy.y + h);
    OnOffSwitch = new COnOffButton(size, this, kOnOff, hswitch);
    OnOffSwitch->setValue(effect->getParameter(kOnOff));
    OnOffSwitch->setTransparency(1);
    lFrame->addView(OnOffSwitch);

	//--OnOffLamp--------
	w = hlamp->getWidth();
    h = hlamp->getHeight() / 11;   // 11 sottoimmagini
	xy (463, 67);
    size(xy.x, xy.y, xy.x + w, xy.y + h);
    OnOffLamp = new CMovieBitmap(size, this, kOnOff, 11, 29, hlamp, point);
    OnOffLamp->setValue(effect->getParameter(kOnOff));
    OnOffLamp->setTransparency(1);
    lFrame->addView(OnOffLamp);

	//--Init Knob------------------------------------------------
	w = hknobs->getWidth();
    h = hknobs->getHeight() / 103;   // 103 sottoimmagini
	//--RelTKnob--------
	xy (382, 260);
    size(xy.x, xy.y, xy.x + w, xy.y + h);
    RelTKnob = new CAnimKnob(size, this, kRelT, 103, 88, hknobs, point);
    RelTKnob->setValue(effect->getParameter(kRelT));
    RelTKnob->setTransparency(1);
    lFrame->addView(RelTKnob);

    //--AtkTKnob--------
	xy (230, 260);
    size(xy.x, xy.y, xy.x + w, xy.y + h);
    AtkTKnob = new CAnimKnob(size, this, kAtkT, 103, 88, hknobs, point);
    AtkTKnob->setValue(effect->getParameter(kAtkT));
    AtkTKnob->setTransparency(1);
    lFrame->addView(AtkTKnob);

    //--CthKnob--------
    xy (56, 260);
    size(xy.x, xy.y, xy.x + w, xy.y + h);
    CthKnob = new CAnimKnob(size, this, kCth, 103, 88, hknobs, point);
    CthKnob->setValue(effect->getParameter(kCth));
    CthKnob->setTransparency(1);
    lFrame->addView(CthKnob);

    //--CompFKnob--------
    xy (535, 260);
    size(xy.x, xy.y, xy.x + w, xy.y + h);
    CompFKnob = new CAnimKnob(size, this, kCompF, 103, 88, hknobs, point);
    CompFKnob->setValue(effect->getParameter(kCompF));
    CompFKnob->setTransparency(1);
    lFrame->addView(CompFKnob);

	//--KneeKnob--------
    xy (535, 370);
    size(xy.x, xy.y, xy.x + w, xy.y + h);
    KneeKnob = new CAnimKnob(size, this, kKnee, 103, 88, hknobs, point);
    KneeKnob->setValue(effect->getParameter(kKnee));
    KneeKnob->setTransparency(1);
    lFrame->addView(KneeKnob);

    //--OutGKnob--------
    xy (230, 370);
    size(xy.x, xy.y, xy.x + w, xy.y + h);
    OutGKnob = new CAnimKnob(size, this, kOutG, 103, 88, hknobs, point);
    OutGKnob->setValue(effect->getParameter(kOutG));
    OutGKnob->setTransparency(1);
    lFrame->addView(OutGKnob);

    //--LthKnob--------
    xy (382, 370);
    size(xy.x, xy.y, xy.x + w, xy.y + h);
    LthKnob = new CAnimKnob(size, this, kLth, 103, 88, hknobs, point);
    LthKnob->setValue(effect->getParameter(kLth));
    LthKnob->setTransparency(1);
    lFrame->addView(LthKnob);

	//--Init display------------------------------------------------
	w = 30;
    h = 14;
	// Treshold
	xy (145, 245);
	size(xy.x, xy.y, xy.x + w, xy.y + h);
	CthDisplay = new CParamDisplay (size, 0, kCenterText);
	CthDisplay->setFont (kNormalFontSmall);
	CthDisplay->setFontColor (kBlackCColor);
	CthDisplay->setTransparency(1);
	CthDisplay->setValue (effect->getParameter (kCth));
	CthDisplay->setStringConvert (TresholdConvert);
	lFrame->addView (CthDisplay);

	// Attack time
	xy (320, 245);
	size(xy.x, xy.y, xy.x + w, xy.y + h);
	AtkTDisplay = new CParamDisplay (size, 0, kCenterText);
	AtkTDisplay->setFont (kNormalFontSmall);
	AtkTDisplay->setFontColor (kBlackCColor);
	AtkTDisplay->setTransparency(1);
	AtkTDisplay->setValue (effect-> getParameter (kAtkT));
	AtkTDisplay->setStringConvert (AtkTConvert);
	lFrame->addView (AtkTDisplay);
	
	// Release time
	xy (475, 245);
	size(xy.x, xy.y, xy.x + w, xy.y + h);
	RelTDisplay = new CParamDisplay (size, 0, kCenterText);
	RelTDisplay->setFont (kNormalFontSmall);
	RelTDisplay->setFontColor (kBlackCColor);
	RelTDisplay->setTransparency(1);
	RelTDisplay->setValue (effect->getParameter (kRelT));
	RelTDisplay->setStringConvert (RelTConvert);
	lFrame->addView (RelTDisplay);

	// Compression ratio
	xy (625, 245);
	size(xy.x, xy.y, xy.x + w, xy.y + h);
	CompFDisplay = new CParamDisplay (size, 0, kCenterText);
	CompFDisplay->setFont (kNormalFontSmall);
	CompFDisplay->setFontColor (kBlackCColor);
	CompFDisplay->setTransparency(1);
	CompFDisplay->setValue (effect->getParameter (kCompF));
	CompFDisplay->setStringConvert (CompFConvert);
	lFrame->addView (CompFDisplay);

	// Knee
	xy (625, 360);
	size(xy.x, xy.y, xy.x + w, xy.y + h);
	KneeDisplay = new CParamDisplay (size, 0, kCenterText);
	KneeDisplay->setFont (kNormalFontSmall);
	KneeDisplay->setFontColor (kBlackCColor);
	KneeDisplay->setTransparency(1);
	KneeDisplay->setValue (effect->getParameter (kKnee));
	KneeDisplay->setStringConvert (KneeConvert);
	lFrame->addView (KneeDisplay);

	// Make-up gain
	xy (320, 360);
	size(xy.x, xy.y, xy.x + w, xy.y + h);
	OutGDisplay = new CParamDisplay (size, 0, kCenterText);
	OutGDisplay->setFont (kNormalFontSmall);
	OutGDisplay->setFontColor (kBlackCColor);
	OutGDisplay->setTransparency(1);
	OutGDisplay->setValue (effect->getParameter (kOutG));
	OutGDisplay->setStringConvert (OutGConvert);
	lFrame->addView (OutGDisplay);

	// Limiter treshold
	xy (475, 360);
	size(xy.x, xy.y, xy.x + w, xy.y + h);
	LthDisplay = new CParamDisplay (size, 0, kCenterText);
	LthDisplay->setFont (kNormalFontSmall);
	LthDisplay->setFontColor (kBlackCColor);
	LthDisplay->setTransparency(1);
	LthDisplay->setValue (effect->getParameter (kLth));
	LthDisplay->setStringConvert (TresholdConvert);
	lFrame->addView (LthDisplay);

	//--Init VuMeter------------------------------------------------
	w = hmeter_animation->getWidth();
    h = hmeter_animation->getHeight() / 141;   // 141 sottoimmagini
	//--GRedVu--------
	xy (275, 88);
	size(xy.x, xy.y, xy.x + w, xy.y + h);
    GRedVu = new CMovieBitmap(size, this, kGRed, 141, 79, hmeter_animation, point);
    GRedVu->setValue(effect->getParameter(kGRed));
    lFrame->addView(GRedVu);

	// Version Label
	w = 100;
    h = 10; 
	xy (570, 1);
    size(xy.x, xy.y, xy.x + w, xy.y + h);
    PluginVersion = new CTextLabel(size, "v 2.10");
    PluginVersion->setBackColor(kTransparentCColor);
    PluginVersion->setFontColor(kBlackCColor);
    PluginVersion->setFrameColor(kTransparentCColor);
    PluginVersion->setHoriAlign(kRightText);
    PluginVersion->setTransparency(true);
    PluginVersion->setFont(kNormalFontVerySmall);
    lFrame->addView(PluginVersion);

    setKnobMode(kLinearMode);

    if(hknobs)
        hknobs->forget();

	if(hmeter_animation)
        hmeter_animation->forget();

	if(hswitch)
        hswitch->forget();

	if(hlamp)
        hlamp->forget();

    frame = lFrame;
	return true;
}

//-----------------------------------------------------------------------------
void CZEditor::close ()
{
	delete frame;
	frame = 0;
}

//-----------------------------------------------------------------------------
void CZEditor::setParameter (VstInt32 index, float value)
{
	if (frame == 0)
		return;

	// called from CompzocchioEdit
	switch (index)
	{
		case kCth:
			if (CthKnob)
				CthKnob->setValue(effect->getParameter(index));
			if (CthDisplay)
				CthDisplay->setValue (effect->getParameter (index));
			break;

		case kAtkT:
			if (AtkTKnob)
				AtkTKnob->setValue(effect->getParameter(index));
			if (AtkTDisplay)
				AtkTDisplay->setValue (effect->getParameter (index));
			break;

		case kRelT:
			if (RelTKnob)
				RelTKnob->setValue(effect->getParameter(index));
			if (RelTDisplay)
				RelTDisplay->setValue (effect->getParameter (index));
			break;

		case kCompF:
			if (CompFKnob)
				CompFKnob->setValue(effect->getParameter(index));
			if (CompFDisplay)
				CompFDisplay->setValue (effect->getParameter (index));
			break;

		case kKnee:
			if (KneeKnob)
				KneeKnob->setValue(effect->getParameter(index));
			if (KneeDisplay)
				KneeDisplay->setValue (effect->getParameter (index));
			break;

		case kOutG:
			if (OutGKnob)
				OutGKnob->setValue(effect->getParameter(index));
			if (OutGDisplay)
				OutGDisplay->setValue (effect->getParameter (index));
			break;

		case kLth:
			if (LthKnob)
				LthKnob->setValue(effect->getParameter(index));
			if (LthDisplay)
				LthDisplay->setValue (effect->getParameter (index));
			break;

		case kOnOff:
			if (OnOffSwitch)
				OnOffSwitch->setValue(effect->getParameter(index));
			break;
	}
}

//-----------------------------------------------------------------------------
void CZEditor::valueChanged (CDrawContext* context, CControl* control)
{
	long tag = control->getTag ();
	switch (tag)
	{
		case kCth:
		case kAtkT:
		case kRelT:
		case kCompF:
		case kKnee:
		case kOutG:
		case kLth:
		case kOnOff:
			effect->setParameterAutomated (tag, control->getValue ());
			control->setDirty (); // Aggiornamento
		break;
	}
}

//--------------------------------
void CZEditor::idle()
{
    static const float max_level_vu = 1.4125;		// VU meter max +3 db
    static const float min_level_vu = 0.2;			// VU meter lineare fra 20% e 100%
    static const float min_meter_vu = 13.0 / 140.0;   // risoluzione VuMeter: 20% immagine #13, 100% imagine #140
	
	float meter_value = effect->getParameter(kGRed);

	if (meter_value > max_level_vu)
		meter_value = max_level_vu;
	if (meter_value > min_level_vu)
		meter_value = (meter_value - min_level_vu) / (max_level_vu - min_level_vu) * (1.0 - min_meter_vu) + min_meter_vu;
	else
	{
		if (meter_value < 0)
			meter_value = 0;
		meter_value = meter_value / min_level_vu * min_meter_vu;
	}
	
	//Set nuovo livello VuMeter 
	GRedVu->setValue(meter_value);
	GRedVu->setDirty(); // Aggiornamento

	const float min_meter_lamp = 1.0 / 11.0; // risoluzione Lamp (11 sottoimmagini)
	float OnOff_value = effect->getParameter(kOnOff);
	if (OnOff_value > 0 && OnOffLampMeter <= OnOff_value)
		OnOffLampMeter = OnOffLampMeter + min_meter_lamp;
	if (OnOff_value < 1 && OnOffLampMeter >= OnOff_value)
		OnOffLampMeter = OnOffLampMeter - min_meter_lamp;

	//Set nuovo livello Lamp 
	OnOffLamp->setValue(OnOffLampMeter);
	OnOffLamp->setDirty(); // Aggiornamento


	AEffGUIEditor::idle();
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
