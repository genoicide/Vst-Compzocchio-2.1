//------------------------------------------------------------------------------------------------------
// VST Plug-Ins SDK
//
// Category     : VST 2.x SDK Compressore
// Filename     : Compzocchio.cpp
// Created by   : Mattia Mazzocchio
// Description  : Compressore mono
//
//------------------------------------------------------------------------------------------------------

#include <stdio.h>
#include <string.h>
#include <math.h>

#ifndef __Compzocchio__
#include "Compzocchio.h"
#endif

//----------------------------------------------------------------------------- 
CompzocchioProgram::CompzocchioProgram () {}

//-----------------------------------------------------------------------------
Compzocchio::Compzocchio (audioMasterCallback audioMaster)
	: AudioEffectX (audioMaster, kNumPrograms, kNumParams)
{
	// init
	sizeCth = 1; //Dimensione Treshold compressore
	sizeAtkT = 49; //Dimensione Attack time
	sizeRelT = 499; //Dimensione Release time
	sizeCompF = 9; //Dimensione Compressor ratio
	sizeKnee = 12; //Dimensione Knee
	sizeOutG = 24; //Dimensione Make-up Gain
	sizeLth = 1; //Dimensione Treshold Limiter

	Cth = 0; //Threshold compressore [0..1]
	Cth_dB = 0; //Threshold compressore [dB]
	AtkT = 0; //Attack time [campioni]
	RelT = 0; //Release time [campioni]
	CompF = 0; //Fattore di compressione
	Knee = 0; //Knee [dB]
	OutG = 0; //Make-up gain [dB]
	Lth = 0; //Threshold limiter [0..1]
	Lth_dB = 0; //Threshold limiter [dB]
	GRed = 1; //Gain Reduction meter [0..1]
	OnOff = 1; //OnOff Switch flag [0-1]

	maxatt = 0; //Segnalatore di ampiezza segnale massimo nel ciclo in corso [0..1]
	maxprec = 0; //Segnalatore di ampiezza segnale massimo nel ciclo precedente [0..1]
	maxprec_dB = 0; //Segnalatore di ampiezza segnale massimo nel ciclo precedente [dB]
	gain_r_Db[0] = 0; //Gain reduction (partenza)
	gain_r_Db[1] = 0; //Gain reduction (destinazione)
	gain_r_Db[2] = 0; //Gain reduction (attuale)
	gain_r_Db[3] = 0; //Gain reduction (limiter reduction)
	xi[0] = 0; //Cth_low interpolazione (partenza) [dB]
	xi[1] = 0; //Cth interpolazione (intermedio) [dB]
	xi[2] = 0; //Cth_highin interpolazione (finale) [dB]
	yi[0] = 0; //Cth_knee interpolazione (partenza) [dB]
	yi[1] = 0; //Cth_highout interpolazione (intermedio) [dB]
	yi[2] = 0; //Cth_low interpolazione (finale) [dB]
	yi[3] = 0; //Interpolazione [dB]
	comp = 0; //Fase compressore: 0 disinserito, 1 in inserzione/compressione piena, 2 in disinserzione
	k_at = 1; //Contatore tempo attacco in campioni
	k_rt = 1; //Contatore tempo rilascio in campioni
	at = AtkT * (sampleRate / 1000); //AtkT in campioni 
	rt = RelT * (sampleRate / 1000); //RelT in campioni
	
	programs = new CompzocchioProgram[numPrograms];

	fCth = 0;
	fAtkT = 0;
	fRelT = 0;
	fCompF = 0;
	fKnee = 0;
	fOutG = 0;
	fLth = 0;

	if (programs)
		setProgram (0);

	initProgram ();

	setNumInputs (1);	// mono input
	setNumOutputs (1);	// mono output

	setUniqueID ('CZoc');	// this should be unique, use the Steinberg web page for plugin Id registration
}

//------------------------------------------------------------------------
Compzocchio::~Compzocchio ()
{
	if (programs)
		delete[] programs;
}

//------------------------------------------------------------------------
void Compzocchio::setProgram (VstInt32 program)
{
	CompzocchioProgram* ap = &programs[program];
	curProgram = program;
	switch (program)
	{
		case 0 :	programs[curProgram].fCth = 1;
					programs[curProgram].fAtkT = 0;
					programs[curProgram].fRelT = 0;
					programs[curProgram].fCompF = 0;
					programs[curProgram].fKnee = 0;
					programs[curProgram].fOutG = 0.5;
					programs[curProgram].fLth = 1;
					break;
		case 1 :	programs[curProgram].fCth = 0.09;
					programs[curProgram].fAtkT = 0.17;
					programs[curProgram].fRelT = 1;
					programs[curProgram].fCompF = 0.5;
					programs[curProgram].fKnee = 0.55;
					programs[curProgram].fOutG = 0.8;
					programs[curProgram].fLth = 1;
					break;
		case 2 :	programs[curProgram].fCth = 0.16;
					programs[curProgram].fAtkT = 0.13;
					programs[curProgram].fRelT = 0.8;
					programs[curProgram].fCompF = 0.55;
					programs[curProgram].fKnee = 0.4;
					programs[curProgram].fOutG = 0.65;
					programs[curProgram].fLth = 1;
					break;
		case 3 :	programs[curProgram].fCth = 0.14;
					programs[curProgram].fAtkT = 0.14;
					programs[curProgram].fRelT = 0.9;
					programs[curProgram].fCompF = 0.22;
					programs[curProgram].fKnee = 0.4;
					programs[curProgram].fOutG = 0.5;
					programs[curProgram].fLth = 1;
					break;
	}	
	setParameter (kCth, ap->fCth);
	setParameter (kAtkT, ap->fAtkT);
	setParameter (kRelT, ap->fRelT);
	setParameter (kCompF, ap->fCompF);
	setParameter (kKnee, ap->fKnee);
	setParameter (kOutG, ap->fOutG);
	setParameter (kLth, ap->fLth);
}

//------------------------------------------------------------------------
void Compzocchio::setCth (float fcth)
{
	fCth = fcth;
	programs[curProgram].fCth = fcth;
	Cth = fCth * sizeCth;
	Cth_dB = 20 * log10(Cth); //Treshold in dB
	Cth_dB_low = Cth_dB - Knee;
	Cth_low = pow(10,( (float)Cth_dB_low / 20));
	Cth_dB_highin = Cth_dB + Knee;
	Cth_highin = pow(10,( (float)Cth_dB_highin / 20));
	Cth_dB_highout = ((Cth_dB_highin - Cth_dB) / CompF) + Cth_dB;
	Cth_highout = pow(10,( (float)Cth_dB_highout / 20));
	maxknee = ((Cth_dB_highout - Cth_dB_low) / (Cth_dB_highin - Cth_dB_low)) * (Cth_dB-Cth_dB_low)+Cth_dB_low;	
	Dknee = 0.5 * (Cth_dB - maxknee);
	Cth_dB_knee = Cth_dB - Dknee;
	Cth_knee = pow(10,( (float)Cth_dB_knee / 20));
	xi[0] = Cth_dB_low;
	xi[1] = Cth_dB;
	xi[2] = Cth_dB_highin;
	yi[0] = Cth_dB_low;
	yi[1] = Cth_dB_knee;
	yi[2] = Cth_dB_highout;
}

//------------------------------------------------------------------------
void Compzocchio::setAtkT (float fatkT)
{
	fAtkT = fatkT;
	programs[curProgram].fAtkT = fatkT;
	AtkT = fAtkT * sizeAtkT + 1;
	at = AtkT * (sampleRate / 1000); //AtkT in campioni sampleRate
}

//------------------------------------------------------------------------
void Compzocchio::setRelT (float frelT)
{
	fRelT = frelT;
	programs[curProgram].fRelT = frelT;
	RelT = fRelT * sizeRelT + 1;
	rt = RelT * (sampleRate / 1000); //RelT in campioni
}

//------------------------------------------------------------------------
void Compzocchio::setCompF (float fcompF)
{
	fCompF = fcompF;
	programs[curProgram].fCompF = fcompF;
	CompF = fCompF * sizeCompF + 1;
}

//------------------------------------------------------------------------
void Compzocchio::setKnee (float fknee)
{
	fKnee = fknee;
	programs[curProgram].fKnee = fknee;
	Knee = fKnee * sizeKnee;
	Cth_dB_low = Cth_dB - Knee;
	Cth_low = pow(10,( (float)Cth_dB_low / 20));
	Cth_dB_highin = Cth_dB + Knee;
	Cth_highin = pow(10,( (float)Cth_dB_highin / 20));
	Cth_dB_highout = ((Cth_dB_highin - Cth_dB) / CompF) + Cth_dB;
	Cth_highout = pow(10,( (float)Cth_dB_highout / 20));
	maxknee = ((Cth_dB_highout - Cth_dB_low) / (Cth_dB_highin - Cth_dB_low)) * (Cth_dB-Cth_dB_low)+Cth_dB_low;	
	Dknee = 0.5 * (Cth_dB - maxknee);
	Cth_dB_knee = Cth_dB - Dknee;
	Cth_knee = pow(10,( (float)Cth_dB_knee / 20));
	xi[0] = Cth_dB_low;
	xi[1] = Cth_dB;
	xi[2] = Cth_dB_highin;
	yi[0] = Cth_dB_low;
	yi[1] = Cth_dB_knee;
	yi[2] = Cth_dB_highout;
}

//------------------------------------------------------------------------
void Compzocchio::setOutG (float foutG)
{
	fOutG = foutG;
	programs[curProgram].fOutG = foutG;
	OutG = pow(10,((float)(fOutG * sizeOutG - 12)/20));
}

//------------------------------------------------------------------------
void Compzocchio::setLth (float flth)
{
	fLth = flth;
	programs[curProgram].fLth = flth;
	Lth = fLth * sizeLth;
	Lth_dB = 20 * log10(Lth); //Treshold in dB
}

//------------------------------------------------------------------------
void Compzocchio::setOnOff (int fonoff)
{
	OnOff = fonoff; //Flag On-Off
}

//------------------------------------------------------------------------
void Compzocchio::initProgram ()
{
	setProgram (0);
	setProgramName ("Default");
	setProgram (1);
	setProgramName ("Kick");
	setProgram (2);
	setProgramName ("Bass");
	setProgram (3);
	setProgramName ("Voice");
	setProgram (4);
	setProgramName ("Free");
	programs[4].fCth = 1;
	programs[4].fAtkT = 0;
	programs[4].fRelT = 0;
	programs[4].fCompF = 0;
	programs[4].fKnee = 0;
	programs[4].fOutG = 0.5;
	programs[4].fLth = 1;
	setProgram (0);
}
//------------------------------------------------------------------------
void Compzocchio::setProgramName (char *name)
{
	strcpy (programs[curProgram].name, name);
}

//-----------------------------------------------------------------------------------------
bool Compzocchio::getProgramNameIndexed (VstInt32 category, VstInt32 index, char* text)
{
	if (index < kNumPrograms)
	{
		strcpy (text, programs[index].name);
		return true;
	}
	return false;
}

//------------------------------------------------------------------------
void Compzocchio::setParameter (VstInt32 index, float value)
{
	CompzocchioProgram* ap = &programs[curProgram];

	switch (index)
	{
		case kCth :		setCth (value);		break;
		case kAtkT :	setAtkT (value);	break;
		case kRelT :	setRelT (value);	break;
		case kCompF :	setCompF (value);	break;
		case kKnee :	setKnee (value);	break;
		case kOutG :	setOutG (value);	break;
		case kLth :		setLth (value);		break;
		case kOnOff :	setOnOff (value);	break;
	}
}

//------------------------------------------------------------------------
float Compzocchio::getParameter (VstInt32 index)
{
	float v = 0;

	switch (index)
	{
		case kCth :		v = fCth;	break;
		case kAtkT :	v = fAtkT; 	break;
		case kRelT :	v = fRelT; 	break;
		case kCompF :	v = fCompF; break;
		case kKnee :	v = fKnee;	break;
		case kOutG :	v = fOutG; 	break;
		case kLth :		v = fLth; 	break;
		case kGRed :	v = GRed; 	break;
		case kOnOff :	v = OnOff; 	break;
	}
	return v;
}

//------------------------------------------------------------------------
void Compzocchio::getParameterName (VstInt32 index, char *label)
{
	switch (index)
	{
		case kCth :		strcpy (label, "Treshold");				break;
		case kAtkT :	strcpy (label, "Attack time");			break;
		case kRelT :	strcpy (label, "Release time");			break;
		case kCompF :	strcpy (label, "Compression factor");	break;
		case kKnee :	strcpy (label, "Knee");					break;
		case kOutG :	strcpy (label, "Make-up gain");			break;
		case kLth :		strcpy (label, "Limiter");				break;
	}
}

//------------------------------------------------------------------------
void Compzocchio::getParameterDisplay (VstInt32 index, char *text)
{
	switch (index)
	{
		case kCth :		dB2string (Cth, text, kVstMaxParamStrLen);		break;
		case kAtkT :	int2string (AtkT, text, kVstMaxParamStrLen);	break;
		case kRelT :	int2string (RelT, text, kVstMaxParamStrLen);	break;
		case kCompF :	float2string (CompF, text, kVstMaxParamStrLen);	break;
		case kKnee :	float2string (Knee, text, kVstMaxParamStrLen);	break;
		case kOutG :	dB2string (OutG, text, kVstMaxParamStrLen);		break;
		case kLth :		dB2string (Lth, text, kVstMaxParamStrLen);		break;
	}
}

//------------------------------------------------------------------------
void Compzocchio::getParameterLabel (VstInt32 index, char *label)
{
	switch (index)
	{
		case kCth :		strcpy (label, "dB");		break;
		case kAtkT :	strcpy (label, "ms");		break;
		case kRelT :	strcpy (label, "ms");		break;
		case kCompF :	strcpy (label, "Ratio");	break;
		case kKnee :	strcpy (label, "dB");		break;
		case kOutG :	strcpy (label, "dB");		break;
		case kLth :		strcpy (label, "dB");		break;
	}
}

//------------------------------------------------------------------------
bool Compzocchio::getEffectName (char* name)
{
	strcpy (name, "Compzocchio");
	return true;
}

//------------------------------------------------------------------------
bool Compzocchio::getProductString (char* text)
{
	strcpy (text, "Compzocchio");
	return true;
}

//------------------------------------------------------------------------
bool Compzocchio::getVendorString (char* text)
{
	strcpy (text, "Mattia Mazzocchio");
	return true;
}

//---------------------------------------------------------------------------
void Compzocchio::processReplacing (float** inputs, float** outputs, VstInt32 sampleFrames)
{
	float* in1  =  inputs[0];
    float* out1 = outputs[0];

	while (--sampleFrames >= 0)
	{
		float x = *in1++;
		float y = x;
		if (OnOff>0)
		{
			if (maxatt < abs(x))
				maxatt = abs(x);
			if (abs(20*log10(maxprec) - 20*log10(maxatt)) > 0.1) //riazzera il tempo di attacco se |maxatt-maxprec|>0.1dB
				k_at = 1; 
			if (maxprec > Cth_low)
				comp = 1;
			if ((maxprec < Cth_low) && (comp!=0))
				comp = 2;
			if (comp == 1)
			{
				if ((maxprec >= Cth_low) && (maxprec < Cth))
				{
					yi[3] = yi[0]*((maxprec_dB-xi[1])/(xi[0]-xi[1]))*((maxprec_dB-xi[2])/(xi[0]-xi[2]))+yi[1]*((maxprec_dB-xi[0])/(xi[1]-xi[0]))*((maxprec_dB-xi[2])/(xi[1]-xi[2]))+yi[2]*((maxprec_dB-xi[0])/(xi[2]-xi[0]))*((maxprec_dB-xi[1])/(xi[2]-xi[1]));
					if (yi[3] > maxprec_dB) // Controllo coerenza interpolazione
						yi[3] = maxprec_dB;
					gain_r_Db[1] = yi[3] - maxprec_dB;
				}
				if ((maxprec >= Cth) && (maxprec < Cth_highin))
				{
					yi[3] = yi[0]*((maxprec_dB-xi[1])/(xi[0]-xi[1]))*((maxprec_dB-xi[2])/(xi[0]-xi[2]))+yi[1]*((maxprec_dB-xi[0])/(xi[1]-xi[0]))*((maxprec_dB-xi[2])/(xi[1]-xi[2]))+yi[2]*((maxprec_dB-xi[0])/(xi[2]-xi[0]))*((maxprec_dB-xi[1])/(xi[2]-xi[1]));
					if (yi[3] > (((maxprec_dB - Cth_dB) / CompF) + Cth_dB)) // Controllo coerenza interpolazione
						yi[3] = ((maxprec_dB - Cth_dB) / CompF) + Cth_dB;
					gain_r_Db[1] = yi[3] - maxprec_dB;
				}
				if (maxprec >= Cth_highin)
					gain_r_Db[1]=(((maxprec_dB - Cth_dB) / CompF) + Cth_dB) - maxprec_dB;
				if (k_at == 1)
					gain_r_Db[0]=gain_r_Db[2];
				gain_r_Db[2]=(((gain_r_Db[1] - gain_r_Db[0]) / at) * k_at) + gain_r_Db[0];
				k_at++;
				if (k_at >= at)
				{
					comp=2;
					k_at=1;
					k_rt=1;
				}
			}
			if (comp == 2)
			{
				if (k_rt == 1)
					gain_r_Db[0]=gain_r_Db[2];
				gain_r_Db[2]=(((0 - gain_r_Db[0]) / rt) * k_rt) + gain_r_Db[0];
				k_rt++;
				if (k_rt >= rt)
					comp=0;
			}
			y = maxprec * pow(10,( (float)gain_r_Db[2] / 20)) * OutG; // max y in compressione con make-up gain
			if (y > Lth) //Check Limiter
			{
				gain_r_Db[3] = (((maxprec_dB - Lth_dB) / 30) + Lth_dB) - maxprec_dB;
				y = x * pow(10,( (float)gain_r_Db[3] / 20));
			}
			else
			{
				y = x * pow(10,( (float)gain_r_Db[2] / 20)) * OutG;
				gain_r_Db[3] = 0;
			}
		}
		(*out1++) = y;
		GRed = (float)y / x; //Gain reduction
	}
	maxprec = maxatt;
	maxprec_dB = 20*log10(maxprec);
	maxatt = 0;
}