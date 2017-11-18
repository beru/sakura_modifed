#include "StdAfx.h"
#include "SortedTagJumpList.h"

/*!
	@date 2005.04.23 genta �Ǘ����̍ő�l���w�肷������ǉ�
*/
SortedTagJumpList::SortedTagJumpList(int max)
	:
	pTagjump(nullptr),
	nCount(0),
	bOverflow(false),
	capacity(max)
{
	// id == 0 �� �󕶎���ɂ���
	baseDirArr.push_back(_T(""));
}

SortedTagJumpList::~SortedTagJumpList()
{
	Empty();
}

/*
	�w�肳�ꂽ�A�C�e���̃��������������B

	@param[in] item �폜����A�C�e��
*/
void SortedTagJumpList::Free(TagJumpInfo* item)
{
	free(item->keyword);
	free(item->filename);
	free(item->note);
	free(item);
	return;
}

/*
	���X�g�����ׂĉ������B
*/
void SortedTagJumpList::Empty(void)
{
	auto p = pTagjump;
	while (p) {
		auto next = p->next;
		Free(p);
		p = next;
	}
	pTagjump = nullptr;
	nCount = 0;
	bOverflow = false;
	baseDirArr.clear();
	baseDirArr.push_back(_T(""));
}

/*
	��t�H���_��o�^���A��t�H���_ID���擾
	@date 2010.07.23 Moca �V�K�ǉ�
*/
int SortedTagJumpList::AddBaseDir(const TCHAR* baseDir)
{
	baseDirArr.push_back(baseDir);
	return baseDirArr.size() - 1;
}

/*
	�A�C�e�����\�[�g���ꂽ��ԂŃ��X�g�ɒǉ�����B
	�A�C�e�����ő吔�𒴂���ꍇ�́A������A�C�e�����폜����B
	������̓R�s�[���쐬����̂ŁA�Ăяo�����͕�����̃A�h���X���ێ�����K�v�͂Ȃ��B
	
	@param[in] keyword	�L�[���[�h
	@param[in] filename	�t�@�C����
	@param[in] no		�s�ԍ�
	@param[in] type		���
	@param[in] note		���l
	@param[in] depth	(�����̂ڂ�)�K�w
	@param[in] baseDirId	��t�H���_ID�B0�ŋ󕶎���w�� (AddBaseDir�̖߂�l)
	@retval true  �ǉ�����
	@retval false �ǉ����s
	@date 2010.07.23 Moca baseDirId �ǉ�
*/
bool SortedTagJumpList::AddParamA(
	const char* keyword,
	const char* filename,
	int no,
	char type,
	const char* note,
	int depth,
	int baseDirId
	)
{
	TagJumpInfo*	p;
	TagJumpInfo*	prev;
	TagJumpInfo*	item;
	// 3�߂�SJIS�p�ی�
	char typeStr[] = {type, '\0', '\0'};

	// �A�C�e�����쐬����B
	item = (TagJumpInfo*)malloc(sizeof(TagJumpInfo));
	if (!item) {
		return false;
	}
	item->keyword  = _tcsdup(to_tchar(keyword));
	item->filename = _tcsdup(to_tchar(filename));
	item->no       = no;
	item->type     = to_tchar(typeStr)[0];
	item->note     = _tcsdup(to_tchar(note));
	item->depth    = depth;
	item->next     = nullptr;
	item->baseDirId = baseDirId;

	// �����񒷃K�[�h
	if (_tcslen(item->keyword	) >= MAX_TAG_STRING_LENGTH) item->keyword[ MAX_TAG_STRING_LENGTH-1] = 0;
	if (_tcslen(item->filename	) >= MAX_TAG_STRING_LENGTH) item->filename[MAX_TAG_STRING_LENGTH-1] = 0;
	if (_tcslen(item->note		) >= MAX_TAG_STRING_LENGTH) item->note[    MAX_TAG_STRING_LENGTH-1] = 0;

	// �A�C�e�������X�g�̓K���Ȉʒu�ɒǉ�����B
	prev = nullptr;
	for (p=pTagjump; p; p=p->next) {
		if (_tcscmp(p->keyword, item->keyword) > 0) {
			break;
		}
		prev = p;
	}
	item->next = p;
	if (prev) prev->next = item;
	else      pTagjump = item;
	++nCount;

	// �ő吔�𒴂�����Ō�̃A�C�e�����폜����B
	if (nCount > capacity) {
		prev = nullptr;
		for (p=pTagjump; p->next; p=p->next) {
			prev = p;
		}
		if (prev) prev->next = nullptr;
		else      pTagjump = nullptr;
		Free(p);
		nCount--;
		bOverflow = true;
	}
	return true;
}

/*
	�w��̏����擾����B

	@param[out] keyword		�L�[���[�h
	@param[out] filename	�t�@�C����
	@param[out] no			�s�ԍ�
	@param[out] type		���
	@param[out] note		���l
	@param[out] depth		(�����̂ڂ�)�K�w
	@param[out] baseDir		�t�@�C�����̊�t�H���_
	@return ��������

	@note �s�v�ȏ��̏ꍇ�͈����� NULL ���w�肷��B
*/
bool SortedTagJumpList::GetParam(
	int index,
	TCHAR* keyword,
	TCHAR* filename,
	int* no,
	TCHAR* type,
	TCHAR* note,
	int* depth,
	TCHAR* baseDir
	)
{
	if (keyword ) keyword[0] = 0;
	if (filename) filename[0] = 0;
	if (no      ) *no    = 0;
	if (type    ) *type  = 0;
	if (note    ) note[0] = 0;
	if (depth   ) *depth = 0;
	if (baseDir ) baseDir[0] = 0;

	SortedTagJumpList::TagJumpInfo* p;
	p = GetPtr(index);
	if (p) {
		if (keyword ) _tcscpy(keyword, p->keyword);
		if (filename) _tcscpy(filename, p->filename);
		if (no      ) *no    = p->no;
		if (type    ) *type  = p->type;
		if (note    ) _tcscpy(note, p->note);
		if (depth   ) *depth = p->depth;
		if (baseDir) {
			if (0 <= p->baseDirId && (size_t)p->baseDirId < baseDirArr.size()) {
				auto_strcpy(baseDir, baseDirArr[p->baseDirId].c_str());
			}
		}
		return true;
	}
	return false;
}

/*
	�w��̏����\���̃|�C���^�Ŏ擾����B
	�擾�������͎Q�ƂȂ̂ŉ�����Ă͂Ȃ�Ȃ��B

	@param[in] index �v�f�ԍ�
	@return �^�O�W�����v���
*/
SortedTagJumpList::TagJumpInfo* SortedTagJumpList::GetPtr(int index)
{
	int	i = 0;
	for (auto p=pTagjump; p; p=p->next) {
		if (index == i) {
			return p;
		}
		++i;
	}
	return nullptr;
}

