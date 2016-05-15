/*
	Copyright (C) 2008, kobake

	This software is provided 'as-is', without any express or implied
	warranty. In no event will the authors be held liable for any damages
	arising from the use of this software.

	Permission is granted to anyone to use this software for any purpose,
	including commercial applications, and to alter it and redistribute it
	freely, subject to the following restrictions:

		1. The origin of this software must not be misrepresented;
		   you must not claim that you wrote the original software.
		   If you use this software in a product, an acknowledgment
		   in the product documentation would be appreciated but is
		   not required.

		2. Altered source versions must be plainly marked as such,
		   and must not be misrepresented as being the original software.

		3. This notice may not be removed or altered from any source
		   distribution.
*/
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

