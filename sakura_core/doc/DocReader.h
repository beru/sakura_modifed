#pragma once

class DocLineMgr;

// as decorator
class DocReader {
public:
	DocReader(const DocLineMgr& pcDocLineMgr) : pDocLineMgr(&pcDocLineMgr) { }

	wchar_t* GetAllData(size_t* pnDataLen);	// �S�s�f�[�^��Ԃ�
	const wchar_t* GetLineStr(size_t , size_t*);
	const wchar_t* GetLineStrWithoutEOL(size_t , size_t*); // 
	const wchar_t* GetFirstLinrStr(size_t*);	// ���A�N�Z�X���[�h�F�擪�s�𓾂�
	const wchar_t* GetNextLinrStr(size_t*);	// ���A�N�Z�X���[�h�F���̍s�𓾂�

private:
	const DocLineMgr* pDocLineMgr;
};

