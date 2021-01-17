//-------------------------------------------------------------------------------------------------------
// VST Plug-Ins SDK
//
// Category     : VST 2.x SDK Compressore
// Filename     : Compzocchio.cpp
// Created by   : Mattia Mazzocchio
// Description  : Compressore mono
//
//-------------------------------------------------------------------------------------------------------

#ifndef __Compzocchio__
#define __Compzocchio__

#include "public.sdk/source/vst2.x/audioeffectx.h"

enum
{
	// Global
	kNumPrograms = 5,

	// Parameters Tags
	kCth = 0,
	kAtkT,
	kRelT,
	kCompF,
	kKnee,
	kOutG,
	kLth,

	kNumParams,

	kGRed,
	kOnOff,
};

class Compzocchio;

//------------------------------------------------------------------------
class CompzocchioProgram
{
friend class Compzocchio;
public:
	CompzocchioProgram ();
	~CompzocchioProgram () {}

private:	
	float fCth;
	float fAtkT;
	float fRelT;
	float fCompF;
	float fKnee;
	float fOutG;
	float fLth;
	char name[24];
};

//------------------------------------------------------------------------
class Compzocchio : public AudioEffectX
{
public:
	Compzocchio (audioMasterCallback audioMaster);
	~Compzocchio ();

	//---Da AudioEffect-----------------------
	virtual void processReplacing (float** inputs, float** outputs, VstInt32 sampleFrames);

	virtual void setProgram (VstInt32 program);
	virtual void setProgramName (char* name);
	virtual bool getProgramNameIndexed (VstInt32 category, VstInt32 index, char* text);
	
	virtual void setParameter (VstInt32 index, float value);
	virtual float getParameter (VstInt32 index);
	virtual void getParameterLabel (VstInt32 index, char* label);
	virtual void getParameterDisplay (VstInt32 index, char* text);
	virtual void getParameterName (VstInt32 index, char* text);

	virtual bool getEffectName (char* name);
	virtual bool getVendorString (char* text);
	virtual bool getProductString (char* text);
	virtual VstInt32 getVendorVersion () { return 2100; }
	
	virtual VstPlugCategory getPlugCategory () { return kPlugCategEffect; }

protected:
	void setCth (float Cth);
	void setAtkT (float AtkT);
	void setRelT (float RelT);
	void setCompF (float CompF);
	void setKnee (float Knee);
	void setOutG (float OutG);
	void setLth (float Lth);
	void setOnOff (int OnOff);
	void initProgram ();

	CompzocchioProgram* programs;
	
	float fCth;
	float fAtkT;
	float fRelT;
	float fCompF;
	float fKnee;
	float fOutG;
	float fLth;
	
	float Cth;
	float AtkT;
	float RelT;
	float CompF;
	float Knee;
	float OutG;
	float Lth;
	float GRed;
	int OnOff;

	float sizeCth;
	float sizeAtkT;
	float sizeRelT;
	float sizeCompF;
	float sizeKnee;
	float sizeOutG;
	float sizeLth;

	float maxatt;
	float maxprec;
	float maxprec_dB;
	float Cth_low;
	float Cth_dB_low;
	float Cth_highin;
	float Cth_dB_highin;
	float Cth_knee;
	float Cth_dB_knee;
	float Cth_highout;
	float Cth_dB_highout;
	float maxknee;
	float Dknee;
	float xi[3];
	float yi[4];
	float Cth_dB;
	float Lth_dB;
	float gain_r_Db[4];
	int comp;
	long k_at; 
	long k_rt;	
	long at;
	long rt;
};

#endif
