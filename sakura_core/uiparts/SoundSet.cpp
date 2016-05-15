#include "StdAfx.h"
#include "SoundSet.h"

void SoundSet::NeedlessToSaveBeep()
{
	if (nMuteCount >= 1)
		return;
	ErrorBeep();
}

