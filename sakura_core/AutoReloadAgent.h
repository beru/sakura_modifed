#pragma once

#include "doc/DocListener.h"

// �t�@�C�����X�V���ꂽ�ꍇ�ɍēǍ����s�����ǂ����̃t���O
enum class WatchUpdateType {
	Query,		// �ēǍ����s�����ǂ����_�C�A���O�{�b�N�X�Ŗ₢���킹��
	Notify,		// �X�V���ꂽ���Ƃ��X�e�[�^�X�o�[�Œʒm
	None,		// �X�V�Ď����s��Ȃ�
	AutoLoad,	// �X�V���ꖢ�ҏW�̏ꍇ�ɍă��[�h
};

class AutoReloadAgent : public DocListenerEx {
public:
	AutoReloadAgent();
	void OnBeforeSave(const SaveInfo& saveInfo);
	void OnAfterSave(const SaveInfo& saveInfo);
	void OnAfterLoad(const LoadInfo& loadInfo);
	
	// �Ď��̈ꎞ��~
	void PauseWatching() { ++nPauseCount; }
	void ResumeWatching() { --nPauseCount; assert(nPauseCount >= 0); }
	bool IsPausing() const { return nPauseCount >= 1; }
	
public://#####��
	bool _ToDoChecking() const;
	bool _IsFileUpdatedByOther(FILETIME* pNewFileTime) const;
	void CheckFileTimeStamp();	// �t�@�C���̃^�C���X�^���v�̃`�F�b�N����
	
public:
	WatchUpdateType	watchUpdateType;	// �X�V�Ď����@
	
private:
	int nPauseCount;	// ���ꂪ1�ȏ�̏ꍇ�͊Ď������Ȃ�
	int nDelayCount;	// ���ҏW�ōă��[�h���̒x���J�E���^
};

