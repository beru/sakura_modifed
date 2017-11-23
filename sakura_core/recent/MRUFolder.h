#pragma once

// MRU���X�g�ƌĂ΂�郊�X�g���Ǘ�����B�t�H���_�ŁB

#include <Windows.h> /// BOOL,HMENU // 2002/2/10 aroka
#include "recent/RecentFolder.h"

class MenuDrawer;

class MruFolder {
public:
	// �R���X�g���N�^
	MruFolder();
	~MruFolder();

	// ���j���[���擾����
	HMENU CreateMenu(MenuDrawer& menuDrawer) const;	// ���[��BpMenuDrawer���K�v�Ȃ��Ȃ�Ƃ����Ȃ��B
	HMENU CreateMenu(HMENU hMenu, MenuDrawer& menuDrawer) const;
	BOOL DestroyMenu(HMENU hMenu) const;
	
	// �t�H���_���̈ꗗ��������
	std::vector<LPCTSTR> GetPathList() const;

	// �A�N�Z�X�֐�
	size_t Length() const;	// �A�C�e���̐��B
	size_t MenuLength(void) const { return t_min(Length(), recentFolder.GetViewCount()); }	// ���j���[�ɕ\�������A�C�e���̐�
	void ClearAll();					// �A�C�e�����폜�`�B
	void Add(const TCHAR* pszFolder);	// pszFolder��ǉ�����B
	const TCHAR* GetPath(int num) const;

protected:
	// ���L�������A�N�Z�X�p�B
	struct DllSharedData* pShareData;			// ���L���������Q�Ƃ����B

private:
	RecentFolder recentFolder;	// ����	//@@@ 2003.04.08 MIK
};

