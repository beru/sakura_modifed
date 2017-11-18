#include "StdAfx.h"
#include "env/DllSharedData.h"

#include "TagJumpManager.h"


/*!
	@brief タグジャンプ情報の保存

	タグジャンプするときに、タグジャンプ先の情報を保存する。

	@param[in] pTagJump 保存するタグジャンプ情報
	@retval true	保存成功
	@retval false	保存失敗

	@date 2004/06/21 新規作成
	@date 2004/06/22 Moca 一杯になったら一番古い情報を削除しそこに新しい情報を入れる
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
	@brief タグジャンプ情報の参照

	タグジャンプバックするときに、タグジャンプ元の情報を参照する。

	@param[out] pTagJump 参照するタグジャンプ情報
	@retval true	参照成功
	@retval false	参照失敗

	@date 2004/06/21 新規作成
	@date 2004/06/22 Moca SetTagJump変更による修正
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

