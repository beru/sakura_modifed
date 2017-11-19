#pragma once

#include "_main/global.h"
#include "_main/AppMode.h"
#include "DocEditor.h"
#include "DocFile.h"
#include "DocFileOperation.h"
#include "DocType.h"
#include "DocOutline.h"
#include "DocLocker.h"
#include "layout/LayoutMgr.h"
#include "logic/DocLineMgr.h"
#include "BackupAgent.h"
#include "AutoSaveAgent.h"
#include "AutoReloadAgent.h"
#include "func/FuncLookup.h"
#include "Eol.h"
#include "macro/CookieManager.h"
#include "util/design_template.h"

class SMacroMgr;
class EditWnd;
struct EditInfo;
class FuncInfoArr;
class EditApp;

/*!
	�����֘A���̊Ǘ�
*/
class EditDoc
	:
	public DocSubject,
	public TInstanceHolder<EditDoc>
{
public:
	// �R���X�g���N�^�E�f�X�g���N�^
	EditDoc(EditApp& app);
	~EditDoc();

	// ������
	BOOL Create(EditWnd* pEditWnd);
	void InitDoc();		// �����f�[�^�̃N���A
	void InitAllView();	// �S�r���[�̏������F�t�@�C���I�[�v��/�N���[�Y�����ɁA�r���[������������
	void Clear();

	// �ݒ�
	void SetFilePathAndIcon(const TCHAR* szFile);

	// ����
	EncodingType	GetDocumentEncoding() const;							// �h�L�������g�̕����R�[�h���擾
	bool			GetDocumentBomExist() const;							// �h�L�������g��BOM�t�����擾
	void			SetDocumentEncoding(EncodingType eCharCode, bool bBom);	// �h�L�������g�̕����R�[�h��ݒ�
	bool IsModificationForbidden(EFunctionCode nCommand) const;			// �w��R�}���h�ɂ�鏑���������֎~����Ă��邩�ǂ���
	bool IsEditable() const { return !AppMode::getInstance().IsViewMode() && !(!docLocker.IsDocWritable() && GetDllShareData().common.file.bUneditableIfUnwritable); }	// �ҏW�\���ǂ���
	void GetSaveInfo(SaveInfo* pSaveInfo) const;			// �Z�[�u�����擾

	// ���
	void GetEditInfo(EditInfo*) const;		// �ҏW�t�@�C�������擾
	bool IsAcceptLoad() const;				// ���̃E�B���h�E��(�V�����E�B���h�E���J������)�V�����t�@�C�����J���邩

	// �C�x���g
	bool HandleCommand(EFunctionCode);
	void OnChangeType();
	void OnChangeSetting(bool bDoLayout = true);					// �r���[�ɐݒ�ύX�𔽉f������
	BOOL OnFileClose(bool);			// �t�@�C�������Ƃ���MRU�o�^ & �ۑ��m�F �� �ۑ����s

	void RunAutoMacro(int idx, LPCTSTR pszSaveFilePath = NULL);

	void SetBackgroundImage();

	void SetCurDirNotitle();

	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//                       �����o�ϐ��Q                          //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	// �Q��
	EditWnd*			pEditWnd;

	// �f�[�^�\��
	DocLineMgr			docLineMgr;
	LayoutMgr			layoutMgr;

	// �e��@�\
	DocFile				docFile;
	DocFileOperation	docFileOperation;
	DocEditor			docEditor;
	DocType				docType;
	CookieManager		cookie;

	// �w���p
	BackupAgent			backupAgent;
	AutoSaveAgent		autoSaveAgent;		// �����ۑ��Ǘ�
	AutoReloadAgent		autoReloadAgent;
	DocOutline			docOutline;
	DocLocker			docLocker;

	// ���I���
	int					nCommandExecNum;			// �R�}���h���s��

	// �����
	FuncLookup			funcLookup;				// �@�\���C�@�\�ԍ��Ȃǂ�resolve

	// �������ϐ�
	TextWrappingMethod	nTextWrapMethodCur;		// �܂�Ԃ����@
	bool			bTextWrapMethodCurTemp;	// �܂�Ԃ����@�ꎞ�ݒ�K�p��
	LOGFONT			lfCur;					// �ꎞ�ݒ�t�H���g
	int				nPointSizeCur;			// �ꎞ�ݒ�t�H���g�T�C�Y
	bool			blfCurTemp;				// �t�H���g�ݒ�K�p��
	int				nPointSizeOrg;			// ���̃t�H���g�T�C�Y
	bool			bTabSpaceCurTemp;			// �^�u���ꎞ�ݒ�K�p��

	HBITMAP			hBackImg;
	int				nBackImgWidth;
	int				nBackImgHeight;
};

