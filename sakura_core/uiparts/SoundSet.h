#pragma once

class SoundSet {
public:
	SoundSet() : nMuteCount(0) { }
	void NeedlessToSaveBeep(); // 上書き不要ビープ音
	void MuteOn() { ++nMuteCount; }
	void MuteOff() { --nMuteCount; }
private:
	int	nMuteCount;
};

