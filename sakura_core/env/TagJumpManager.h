#pragma once

// �v��s��`
// #define DllSharedData.h


// 2004/06/21 novice �^�O�W�����v�@�\�ǉ�
// �^�O�W�����v���
struct TagJump {
	HWND hwndReferer;	// �Q�ƌ��E�B���h�E
	Point point;		// ���C��, �J����
};


// ���L���������\����
// 2004/06/21 �^�O�W�����v�@�\�ǉ�
// 2005.04.03 MIK �L�[���[�h�w��^�O�W�����v
struct Share_TagJump {
	// �^
	typedef StaticVector<
		StaticString<wchar_t, _MAX_PATH>,
		MAX_TAGJUMP_KEYWORD
	> ATagJumpKeywords;

	// �f�[�^
	int					tagJumpNum;					// �^�O�W�����v���̗L���f�[�^��
	int					tagJumpTop;					// �X�^�b�N�̈�ԏ�̈ʒu
	TagJump				tagJumps[MAX_TAGJUMPNUM];	// �^�O�W�����v���
	ATagJumpKeywords	aTagJumpKeywords;
	bool				bTagJumpICase;				// �啶���������𓯈ꎋ
	bool				bTagJumpAnyWhere;			// ������̓r���Ƀ}�b�`
};


class TagJumpManager {
public:
	TagJumpManager() {
		pShareData = &GetDllShareData();
	}
	// �^�O�W�����v�֘A	// 2004/06/21 novice �^�O�W�����v�@�\�ǉ�
	void PushTagJump(const TagJump*);		// �^�O�W�����v���̕ۑ�
	bool PopTagJump(TagJump*);				// �^�O�W�����v���̎Q��
private:
	DllSharedData* pShareData;
};

