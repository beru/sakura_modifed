#pragma once

#include "SelectLang.h"		// 2011.04.10 nasukoji

class ShareData;

// 2010.04.19 Moca DllSharedData�֘A��DllSharedData.h���Œ���K�v�ȏꏊ�ֈړ�
// ShareData.h�́A������Interface�����񋟂��܂���B�ʂ�DllSharedData.h��include���邱�ƁB
struct DllSharedData;
struct TypeConfig;
class Mutex;

/*!	@brief ���L�f�[�^�̊Ǘ�

	ShareData��Process�̃����o�ł��邽�߁C���҂̎����͓���ł��D
	�{����Process�I�u�W�F�N�g��ʂ��ăA�N�Z�X����ׂ��ł����C
	Process���̃f�[�^�̈�ւ̃|�C���^��static�ϐ��ɕۑ����邱�Ƃ�
	Singleton�̂悤�ɂǂ�����ł��A�N�Z�X�ł���\���ɂȂ��Ă��܂��D

	���L�������ւ̃|�C���^��pShareData�ɕێ����܂��D���̃����o��
	���J����Ă��܂����CShareData�ɂ����Map/Unmap����邽�߂�
	ChareData�̏��łɂ���ă|�C���^pShareData�������ɂȂ邱�Ƃ�
	���ӂ��Ă��������D

	@date 2002.01.03 YAZAKI m_tbMyButton�Ȃǂ�ShareData����CMenuDrawer�ֈړ��������Ƃɂ��C���B
*/
class ShareData : public TSingleton<ShareData> {
	friend class TSingleton<ShareData>;
	ShareData();
	~ShareData();

public:
	/*
	||  Attributes & Operations
	*/
	bool InitShareData();	// ShareData�N���X�̏���������
	void RefreshString();	// ����I����ɋ��L���������̕�������X�V����
	
	// MRU�n
	bool IsPathOpened(const TCHAR* pszPath, HWND* phwndOwner); // �w��t�@�C�����J����Ă��邩���ׂ�
	bool ActiveAlreadyOpenedWindow(const TCHAR* pszPath, HWND* phwndOwner, EncodingType nCharCode); // �w��t�@�C�����J����Ă��邩���ׂA���d�I�[�v�����̕����R�[�h�Փ˂��m�F // 2007.03.16

	// �f�o�b�O  ���͎�Ƀ}�N���E�O���R�}���h���s�p
	void TraceOut(LPCTSTR lpFmt, ...);	// �A�E�g�v�b�g�E�B���h�E�ɏo��(printf�t�H�[�}�b�g)
	void TraceOutString(const wchar_t* pszStr, int len = -1);	// �A�E�g�v�b�g�E�B���h�E�ɏo��(�����H������)
	void SetTraceOutSource(HWND hwnd) { hwndTraceOutSource = hwnd; }	// TraceOut�N�����E�B���h�E�̐ݒ�
	bool OpenDebugWindow(HWND hwnd, bool bAllwaysActive);	// �f�o�b�O�E�B���h�E���J��

	bool IsPrivateSettings(void);

	// �}�N���֘A
	int	 GetMacroFilename(int idx, TCHAR* pszPath, size_t nBufLen); // idx�Ŏw�肵���}�N���t�@�C�����i�t���p�X�j���擾����	// Jun. 14, 2003 genta �����ǉ��D�����ύX
	bool BeReloadWhenExecuteMacro(int idx);	// idx�Ŏw�肵���}�N���́A���s���邽�тɃt�@�C����ǂݍ��ސݒ肩�H

	// �^�C�v�ʐݒ�(�R���g���[���v���Z�X��p)
	void CreateTypeSettings();
	std::vector<TypeConfig*>& GetTypeSettings();

	// ���ۉ��Ή��̂��߂̕������ύX����(�R���g���[���v���Z�X��p)
	void ConvertLangValues(std::vector<std::wstring>& values, bool bSetValues);

	static Mutex& GetMutexShareWork();

protected:
	/*
	||  �����w���p�֐�
	*/

	// Jan. 30, 2005 genta �������֐��̕���
	void InitKeyword(DllSharedData&);
	bool InitKeyAssign(DllSharedData&); // 2007.11.04 genta �N�����~�̂��ߒl��Ԃ�
	void RefreshKeyAssignString(DllSharedData&);
	void InitToolButtons(DllSharedData&);
	void InitTypeConfigs(DllSharedData&, std::vector<TypeConfig*>&);
	void InitPopupMenu(DllSharedData&);

public:
	static void InitFileTree(FileTree*);

private:
	SelectLang		selectLang;			// ���b�Z�[�W���\�[�XDLL�ǂݍ��ݗp�i�v���Z�X��1�j		// 2011.04.10 nasukoji
	HANDLE			hFileMap;
	DllSharedData*	pShareData;
	std::vector<TypeConfig*>* 	pvTypeSettings;	// (�R���g���[���v���Z�X�̂�)
	HWND			hwndTraceOutSource;	// TraceOutA()�N�����E�B���h�E�i���������N�������w�肵�Ȃ��Ă��ނ悤�Ɂj

};

