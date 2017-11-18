#pragma once

#include <Windows.h> /// BOOL,HMENU // 2002/2/10 aroka
#include <vector>
#include "recent/RecentFile.h"

struct EditInfo; // 2004.04.11 genta �p�����[�^����struct���폜���邽�߁Ddoxygen�΍�
class MenuDrawer;

//	@date 2002.2.17 YAZAKI ShareData�̃C���X�^���X�́AProcess�ɂЂƂ���̂݁B
class MruFile {
public:
	//	�R���X�g���N�^
	MruFile();
	~MruFile();

	//	���j���[���擾����
	HMENU CreateMenu(MenuDrawer& menuDrawer) const;	//	���[��BmenuDrawer���K�v�Ȃ��Ȃ�Ƃ����Ȃ��B
	HMENU CreateMenu(HMENU hMenu, MenuDrawer& menuDrawer) const;	//	2010/5/21 Uchi
	BOOL DestroyMenu(HMENU hMenu) const;
	
	//	�t�@�C�����̈ꗗ��������
	std::vector<LPCTSTR> GetPathList() const;

	//	�A�N�Z�X�֐�
	size_t Length(void) const;	//	�A�C�e���̐��B
	size_t MenuLength(void) const { return t_min(Length(), recentFile.GetViewCount()); }	//	���j���[�ɕ\�������A�C�e���̐�
	void ClearAll(void);	//	�A�C�e�����폜�`�B
	bool GetEditInfo(size_t num, EditInfo* pfi) const;				//	�ԍ��Ŏw�肵��EditInfo�i�����܂邲�Ɓj
	bool GetEditInfo(const TCHAR* pszPath, EditInfo* pfi) const;	//	�t�@�C�����Ŏw�肵��EditInfo�i�����܂邲�Ɓj
	void Add(EditInfo* pEditInfo);		//	*pEditInfo��ǉ�����B

protected:
	// ���L�������A�N�Z�X�p�B
	struct DllSharedData* pShareData;		//	���L���������Q�Ƃ����B
	
private:
	RecentFile	recentFile;	// ����	//@@@ 2003.04.08 MIK
};

