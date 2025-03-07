/*!	@file
	@brief ZIP file操作

*/
#include "StdAfx.h"
#include <ShellAPI.h>
#include "ZipFile.h"

#ifdef __MINGW32__
//uuid(D8F015C0-C278-11CE-A49E-444553540000);
const GUID IID_IShellDispatch =
{
  0xD8F015C0, 0xc278, 0x11ce, { 0xa4, 0x9e, 0x44, 0x45, 0x53, 0x54 }
};
// 13709620-C279-11CE-A49E-444553540000
const GUID CLSID_Shell =
{
  0x13709620, 0xc279, 0x11ce, { 0xa4, 0x9e, 0x44, 0x45, 0x53, 0x54 }
};
#endif


// コンストラクタ
ZipFile::ZipFile() {
	HRESULT hr = CoCreateInstance(
		CLSID_Shell,
		NULL,
		CLSCTX_INPROC_SERVER,
		IID_IShellDispatch,
		reinterpret_cast<void **>(&psd)
	);
	if (FAILED(hr)) {
		psd = NULL;
	}
	pZipFile = NULL;
}


// デストラクタ
ZipFile::~ZipFile() {
	if (pZipFile) {
		pZipFile->Release();
		pZipFile = NULL;
	}
	psd = NULL;
}


// Zip File名 設定
bool ZipFile::SetZip(const std::tstring& sZipPath)
{
	if (pZipFile) {
		pZipFile->Release();
		pZipFile = NULL;
	}

	// ZIP Folder設定
	VARIANT var;
	VariantInit(&var);
	var.vt = VT_BSTR;
	var.bstrVal = SysAllocString(to_wchar(sZipPath.c_str()));
	HRESULT hr = psd->NameSpace(var, &pZipFile);
	if (hr != S_OK) {
		pZipFile = NULL;
		return false;
	}

	sZipName = sZipPath;

	return true;
}


// ZIP File 内 フォルダ名取得と定義ファイル検査(Plugin用)
bool ZipFile::ChkPluginDef(
	const std::tstring& sDefFile,
	std::tstring& sFolderName
	)
{
	VARIANT			var;
	FolderItems*	pZipFileItems;
	long			lCount;
	bool			bFoundDef = false;

	sFolderName = _T("");

	// ZIP File List
	HRESULT hr = pZipFile->Items(&pZipFileItems);
	if (hr != S_OK) {
		pZipFile->Release();
		return false;
	}

	// 検査
	hr = pZipFileItems->get_Count(&lCount);
	VariantInit(&var);
	var.vt = VT_I4;
	for (var.lVal=0; var.lVal<lCount; var.lVal++) {
		BSTR			bps;
		VARIANT_BOOL	vFolder;
		FolderItem*		pFileItem;

		hr = pZipFileItems->Item(var, &pFileItem);
		if (hr != S_OK) { continue; }
		hr = pFileItem->get_Name(&bps);
		if (hr != S_OK) { continue; }
		hr = pFileItem->get_IsFolder(&vFolder);
		if (hr != S_OK) { continue; }
		if (vFolder) {
			long			lCount2;
			VARIANT			varj;
			FolderItems*	pFileItems2;
			Folder*			pFile;

			sFolderName = to_tchar(bps);	// Install Follder Name
			hr = pFileItem->get_GetFolder((IDispatch **)&pFile);
			if (hr != S_OK) { continue; }
			hr = pFile->Items(&pFileItems2);
			if (hr != S_OK) { continue; }
			hr = pFileItems2->get_Count(&lCount2);
			if (hr != S_OK) { continue; }
			varj.vt = VT_I4;
			for (varj.lVal=0; varj.lVal<lCount2; varj.lVal++) {
				hr = pFileItems2->Item(varj, &pFileItem);
				if (hr != S_OK) { continue; }
				hr = pFileItem->get_IsFolder(&vFolder);
				if (hr != S_OK) { continue; }
				hr = pFileItem->get_Path(&bps);
				if (hr != S_OK) { continue; }

				// 定義ファイルか
				if (!vFolder && auto_strlen(bps) >= sDefFile.length()
					&& (auto_stricmp(to_tchar(bps), to_tchar((sFolderName + _T("/") + sDefFile).c_str())) == 0
					|| auto_stricmp(to_tchar(bps), to_tchar((sFolderName + _T("\\") + sDefFile).c_str())) == 0
					|| auto_stricmp(to_tchar(bps), to_tchar((sZipName + _T("\\") + sFolderName + _T("\\") + sDefFile).c_str())) == 0)) {
					bFoundDef = true;
					break;
				}
			}
			VariantClear(&varj);
			if (bFoundDef) {
				break;
			}
		}
	}
	VariantClear(&var);
	pZipFileItems->Release();

	return bFoundDef;
}


// ZIP File 解凍
bool ZipFile::Unzip(const std::tstring& sOutPath)
{
	Folder* pOutFolder;
	FolderItems* pZipFileItems;

	// ZIP File List
	HRESULT hr = pZipFile->Items(&pZipFileItems);
	if (hr != S_OK) {
		pZipFile->Release();
		return false;
	}

	// 出力Folder設定
	VARIANT var;
	VariantInit(&var);
	var.vt = VT_BSTR;
	var.bstrVal = SysAllocString(to_wchar(sOutPath.c_str()));
	hr = psd->NameSpace(var, &pOutFolder);
	VariantClear(&var);
	if (hr != S_OK) {
		pZipFileItems->Release();
		pZipFile->Release();
		return false;
	}

	// 展開の設定
	VariantInit(&var);
	var.vt = VT_DISPATCH;
	var.pdispVal = pZipFileItems;
	VARIANT varOpt;
	VariantInit(&varOpt);
	varOpt.vt = VT_I4;
	varOpt.lVal = FOF_SILENT | FOF_NOCONFIRMATION;

	// 展開
	hr = pOutFolder->CopyHere(var, varOpt);

	pOutFolder->Release();
	pZipFileItems->Release();

	return (hr == S_OK);
}

