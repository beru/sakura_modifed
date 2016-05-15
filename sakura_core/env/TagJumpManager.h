/*
	2008.05.18 kobake CShareData ���番��
*/
/*
	Copyright (C) 2008, kobake

	This software is provided 'as-is', without any express or implied
	warranty. In no event will the authors be held liable for any damages
	arising from the use of this software.

	Permission is granted to anyone to use this software for any purpose,
	including commercial applications, and to alter it and redistribute it
	freely, subject to the following restrictions:

		1. The origin of this software must not be misrepresented;
		   you must not claim that you wrote the original software.
		   If you use this software in a product, an acknowledgment
		   in the product documentation would be appreciated but is
		   not required.

		2. Altered source versions must be plainly marked as such,
		   and must not be misrepresented as being the original software.

		3. This notice may not be removed or altered from any source
		   distribution.
*/
#pragma once

// �v��s��`
// #define DllSharedData.h


// 2004/06/21 novice �^�O�W�����v�@�\�ǉ�
// �^�O�W�����v���
struct TagJump {
	HWND		hwndReferer;				// �Q�ƌ��E�B���h�E
	LogicPoint	point;						// ���C��, �J����
};


// ���L���������\����
// 2004/06/21 �^�O�W�����v�@�\�ǉ�
// 2005.04.03 MIK �L�[���[�h�w��^�O�W�����v
struct Share_TagJump {
	// �^
	typedef StaticVector<
		StaticString<WCHAR, _MAX_PATH>,
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

