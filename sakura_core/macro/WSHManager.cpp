/*!	@file
	@brief WSH Manager
*/
#include "StdAfx.h"
#include "macro/WSHManager.h"
#include "macro/WSH.h"
#include "macro/EditorIfObj.h"
#include "view/EditView.h"
#include "io/TextStream.h"
#include "util/os.h"
#include "macro/MacroFactory.h"

static void MacroError(
	BSTR Description,
	BSTR Source,
	void* Data
	)
{
	EditView *View = reinterpret_cast<EditView*>(Data);
	MessageBox(View->GetHwnd(), to_tchar(Description), to_tchar(Source), MB_ICONERROR);
}

WSHMacroManager::WSHMacroManager(std::wstring const AEngineName) : engineName(AEngineName)
{
}

WSHMacroManager::~WSHMacroManager()
{
}

/** WSH�}�N���̎��s

	@param EditView [in] ����Ώ�EditView
*/
bool WSHMacroManager::ExecKeyMacro(EditView& editView, int flags) const
{
	auto engine = std::make_unique<WSHClient>(engineName.c_str(), MacroError, &editView);
	bool bRet = false;
	if (engine->isValid) {
		// �C���^�t�F�[�X�I�u�W�F�N�g�̓o�^
		WSHIfObj* objEditor = new EditorIfObj();
		objEditor->ReadyMethods(editView, flags);
		engine->AddInterfaceObject(objEditor);
		for (auto it=params.begin(); it!=params.end(); ++it) {
			(*it)->ReadyMethods(editView, flags);
			engine->AddInterfaceObject(*it);
		}

		bRet = engine->Execute(source.c_str());
	}
	return bRet;
}

/*!
	WSH�}�N���̓ǂݍ��݁i�t�@�C������j

	@param hInstance [in] �C���X�^���X�n���h��(���g�p)
	@param pszPath   [in] �t�@�C���̃p�X
*/
bool WSHMacroManager::LoadKeyMacro(HINSTANCE hInstance, const TCHAR* pszPath)
{
	// �\�[�X�ǂݍ��� -> source
	source = L"";
	
	std::vector<wchar_t> buffW;
	ReadFileAndUnicodify(pszPath, buffW);

	buffW.push_back(0);
	source = &buffW[0];

	return true;
}

/*!
	WSH�}�N���̓ǂݍ��݁i�����񂩂�j

	@param hInstance [in] �C���X�^���X�n���h��(���g�p)
	@param pszCode   [in] �}�N���R�[�h
*/
bool WSHMacroManager::LoadKeyMacroStr(HINSTANCE hInstance, const TCHAR* pszCode)
{
	// �\�[�X�ǂݍ��� -> source
	source = to_wchar(pszCode);
	return true;
}

MacroManagerBase* WSHMacroManager::Creator(EditView& editView, const TCHAR* FileExt)
{
	TCHAR FileExtWithDot[1024], FileType[1024], EngineName[1024]; // 1024�𒴂������͒m��܂���
	
	_tcscpy(FileExtWithDot, _T("."));
	_tcscat(FileExtWithDot, FileExt);

	if (ReadRegistry(HKEY_CLASSES_ROOT, FileExtWithDot, NULL, FileType, 1024)) {
		lstrcat(FileType, _T("\\ScriptEngine"));
		if (ReadRegistry(HKEY_CLASSES_ROOT, FileType, NULL, EngineName, 1024)) {
			wchar_t EngineNameW[1024];
			_tcstowcs(EngineNameW, EngineName, _countof(EngineNameW));
			return new WSHMacroManager(EngineNameW);
		}
	}
	return NULL;
}

void WSHMacroManager::Declare()
{
	// �b��
	MacroFactory::getInstance().RegisterCreator(Creator);
}

// �C���^�t�F�[�X�I�u�W�F�N�g��ǉ�����
void WSHMacroManager::AddParam(WSHIfObj* param)
{
	params.push_back(param);
}

// �C���^�t�F�[�X�I�u�W�F�N�g�B��ǉ�����
void WSHMacroManager::AddParam(WSHIfObj::List& params)
{
	params.insert(params.end(), params.begin(), params.end());
}

// �C���^�t�F�[�X�I�u�W�F�N�g���폜����
void WSHMacroManager::ClearParam()
{
	params.clear();
}

