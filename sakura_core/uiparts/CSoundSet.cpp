#include "StdAfx.h"
#include "CSoundSet.h"

void SoundSet::NeedlessToSaveBeep()
{
	if (m_nMuteCount >= 1)
		return;
	ErrorBeep();
}

