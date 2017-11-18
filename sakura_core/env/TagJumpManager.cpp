#include "StdAfx.h"
#include "env/DllSharedData.h"

#include "TagJumpManager.h"


/*!
	@brief �^�O�W�����v���̕ۑ�

	�^�O�W�����v����Ƃ��ɁA�^�O�W�����v��̏���ۑ�����B

	@param[in] pTagJump �ۑ�����^�O�W�����v���
	@retval true	�ۑ�����
	@retval false	�ۑ����s
*/
void TagJumpManager::PushTagJump(const TagJump* pTagJump)
{
	int i = pShareData->tagJump.tagJumpTop + 1;
	if (MAX_TAGJUMPNUM <= i) {
		i = 0;
	}
	if (pShareData->tagJump.tagJumpNum < MAX_TAGJUMPNUM) {
		pShareData->tagJump.tagJumpNum++;
	}
	pShareData->tagJump.tagJumps[i] = *pTagJump;
	pShareData->tagJump.tagJumpTop = i;
}


/*!
	@brief �^�O�W�����v���̎Q��

	�^�O�W�����v�o�b�N����Ƃ��ɁA�^�O�W�����v���̏����Q�Ƃ���B

	@param[out] pTagJump �Q�Ƃ���^�O�W�����v���
	@retval true	�Q�Ɛ���
	@retval false	�Q�Ǝ��s
*/
bool TagJumpManager::PopTagJump(TagJump *pTagJump)
{
	if (0 < pShareData->tagJump.tagJumpNum) {
		*pTagJump = pShareData->tagJump.tagJumps[pShareData->tagJump.tagJumpTop--];
		if (pShareData->tagJump.tagJumpTop < 0) {
			pShareData->tagJump.tagJumpTop = MAX_TAGJUMPNUM - 1;
		}
		pShareData->tagJump.tagJumpNum--;
		return true;
	}
	return false;
}

