#include "StdAfx.h"
#include "SoundSet.h"

void SoundSet::NeedlessToSaveBeep()
{
	if (m_nMuteCount >= 1)
		return;
	ErrorBeep();
}

