#pragma once

class DocLineMgr;

// as decorator
class DocReader {
public:
	DocReader(const DocLineMgr& pcDocLineMgr) : pDocLineMgr(&pcDocLineMgr) { }

	wchar_t* GetAllData(size_t* pnDataLen);	// 全行データを返す
	const wchar_t* GetLineStr(size_t , size_t*);
	const wchar_t* GetLineStrWithoutEOL(size_t , size_t*); // 
	const wchar_t* GetFirstLinrStr(size_t*);	// 順アクセスモード：先頭行を得る
	const wchar_t* GetNextLinrStr(size_t*);	// 順アクセスモード：次の行を得る

private:
	const DocLineMgr* pDocLineMgr;
};

