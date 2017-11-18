/*!	@file
	@brief ZIP file操作

*/
#pragma once

#include <ShlDisp.h>

class ZipFile {
private:
	IShellDispatch*	psd;
	Folder*			pZipFile;
	std::tstring	sZipName;

public:
	ZipFile();		// コンストラクタ
	~ZipFile();	// デストラクタ

public:
	bool	IsOk() { return (psd != NULL); }			// Zip Folderが使用できるか?
	bool	SetZip(const std::tstring& sZipPath);		// Zip File名 設定
	bool	ChkPluginDef(const std::tstring& sDefFile, std::tstring& sFolderName);	// ZIP File 内 フォルダ名取得と定義ファイル検査(Plugin用)
	bool	Unzip(const std::tstring& sOutPath);			// Zip File 解凍
};

