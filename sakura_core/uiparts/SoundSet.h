#pragma once

class SoundSet {
public:
	SoundSet() : nMuteCount(0) { }
	void NeedlessToSaveBeep(); // �㏑���s�v�r�[�v��
	void MuteOn() { ++nMuteCount; }
	void MuteOff() { --nMuteCount; }
private:
	int	nMuteCount;
};

