#pragma once

#include "util/design_template.h"

// �^�O�W�����v���X�g

#define MAX_TAG_STRING_LENGTH _MAX_PATH	// �Ǘ����镶����̍ő咷

/*!	@brief �_�C���N�g�^�O�W�����v�p�������ʂ��\�[�g���ĕێ�����D*/
class SortedTagJumpList {
public:
	SortedTagJumpList(int max);
	~SortedTagJumpList();

	int AddBaseDir(const TCHAR* baseDir);
	bool AddParamA(const char* keyword, const char* filename, int no, char type, const char* note, int depth, const int baseDirId);
	bool GetParam(int index, TCHAR* keyword, TCHAR* filename, int* no, TCHAR* type, TCHAR* note, int* depth, TCHAR* baseDir);
	int GetCount(void) { return nCount; }
	void Empty(void);
	bool IsOverflow(void) { return bOverflow; }

	typedef struct tagjump_info_t {
		struct tagjump_info_t*	next;	// ���̃��X�g
		TCHAR*	keyword;	// �L�[���[�h
		TCHAR*	filename;	// �t�@�C����
		int		no;			// �s�ԍ�
		TCHAR	type;		// ���
		TCHAR*	note;		// ���l
		int		depth;		// (�����̂ڂ�)�K�w
		int		baseDirId;	// �t�@�C�����̃x�[�X�f�B���N�g��
	} TagJumpInfo;

	TagJumpInfo* GetPtr(int index);

	/*!	@brief �Ǘ����̍ő�l���擾���� */
	int GetCapacity(void) const { return capacity; }

private:
	TagJumpInfo* pTagjump;	// �^�O�W�����v���
	std::vector<std::tstring> baseDirArr;	// �x�[�X�f�B���N�g�����
	int		nCount;		// ��
	bool	bOverflow;	// �I�[�o�[�t���[
	
	const int	capacity;	// �Ǘ�������̍ő吔

	void Free(TagJumpInfo* item);

private:
	DISALLOW_COPY_AND_ASSIGN(SortedTagJumpList);
};

