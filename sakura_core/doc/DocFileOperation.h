#pragma once

#include "doc/DocListener.h" // LoadInfo
#include "Eol.h"

class EditDoc;

class DocFileOperation {
public:
	DocFileOperation(EditDoc& doc) : doc(doc) { }

	// ロック
	bool _ToDoLock() const;
	void DoFileLock(bool bMsg = true);
	void DoFileUnlock();
	
	// ロードUI
	bool OpenFileDialog(
		HWND				hwndParent,
		const TCHAR*		pszOpenFolder,	// [in]  NULL以外を指定すると初期フォルダを指定できる
		LoadInfo*			pLoadInfo,		// [in/out] ロード情報
		std::vector<std::tstring>&	files
	);

	// ロードフロー
	bool DoLoadFlow(LoadInfo* pLoadInfo);
	bool FileLoad(
		LoadInfo*	pLoadInfo			// [in/out]
	);
	bool FileLoadWithoutAutoMacro(
		LoadInfo*	pLoadInfo			// [in/out]
	);
	void ReloadCurrentFile(				// 同一ファイルの再オープン
		EncodingType	nCharCode		// [in] 文字コード種別
	);

	
	// セーブUI
	bool SaveFileDialog(SaveInfo* pSaveInfo);	//「ファイル名を付けて保存」ダイアログ
	bool SaveFileDialog(LPTSTR szPath);			//「ファイル名を付けて保存」ダイアログ

	// セーブフロー
	bool DoSaveFlow(SaveInfo* pSaveInfo);
	bool FileSaveAs(
		const wchar_t*	filename = NULL,
		EncodingType	eCodeType = CODE_NONE,
		EolType			eEolType = EolType::None,
		bool			bDialog = true
	);	// ダイアログでファイル名を入力させ、保存
	bool FileSave();			// 上書き保存。ファイル名が指定されていなかったらダイアログで入力を促す

	// クローズ
	bool FileClose();			// 閉じて(無題)

	// その他
	void FileCloseOpen(				// 閉じて開く
		const LoadInfo& loadInfo = LoadInfo(_T(""), CODE_AUTODETECT, false)
	);

private:
	EditDoc& doc;
};

