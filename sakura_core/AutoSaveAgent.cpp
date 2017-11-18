#include "StdAfx.h"
#include "AutoSaveAgent.h"
#include "doc/EditDoc.h"
#include "env/DllSharedData.h"


//	From Here Aug. 21, 2000 genta
//
//	自動保存を行うかどうかのチェック
//
void AutoSaveAgent::CheckAutoSave()
{
	if (passiveTimer.CheckAction()) {
		EditDoc* pDoc = GetListeningDoc();

		//	上書き保存

		if (!pDoc->docEditor.IsModified()) {	//	変更無しなら何もしない
			return;				//	ここでは，「無変更でも保存」は無視する
		}

		//	2003.10.09 zenryaku 保存失敗エラーの抑制
		if (!pDoc->docFile.GetFilePathClass().IsValidPath()) {	//	まだファイル名が設定されていなければ保存しない
			return;
		}

		bool en = passiveTimer.IsEnabled();
		passiveTimer.Enable(false);	//	2重呼び出しを防ぐため
		pDoc->docFileOperation.FileSave();	//	保存
		passiveTimer.Enable(en);
	}
}

//
//	設定変更を自動保存動作に反映する
//
void AutoSaveAgent::ReloadAutoSaveParam()
{
	auto& csBackup = GetDllShareData().common.backup;
	passiveTimer.SetInterval(csBackup.GetAutoBackupInterval());
	passiveTimer.Enable(csBackup.IsAutoBackupEnabled());
}

//----------------------------------------------------------
//	class PassiveTimer
//
//----------------------------------------------------------
/*!
	時間間隔の設定
	@param m 間隔(min)
	間隔を0以下に設定したときは1秒とみなす。設定可能な最大間隔は35792分。
*/
void PassiveTimer::SetInterval(int m)
{
	if (m <= 0) {
		m = 1;
	}else if (m >= 35792) {	//	35792分以上だと int で表現できなくなる
		m = 35792;
	}
	nInterval = m * MSec2Min;
}

/*!
	タイマーの有効・無効の切り替え
	@param flag true:有効 / false: 無効
	無効→有効に切り替えたときはリセットされる。
*/
void PassiveTimer::Enable(bool flag)
{
	if (bEnabled != flag) {	//	変更があるとき
		bEnabled = flag;
		if (flag) {	//	enabled
			Reset();
		}
	}
}

/*!
	外部で定期に実行されるところから呼び出される関数。
	呼び出されると経過時間をチェックする。

	@retval true 所定時間が経過した。このときは測定基準が自動的にリセットされる。
	@retval false 所定の時間に達していない。
*/
bool PassiveTimer::CheckAction(void)
{
	if (!IsEnabled()) {	//	有効でなければ何もしない
		return false;
	}

	//	時刻比較
	DWORD now = ::GetTickCount();
	int diff;

	diff = now - nLastTick;	//	TickCountが一回りしてもこれでうまくいくはず...

	if (diff < nInterval) {	//	規定時間に達していない
		return false;
	}

	Reset();
	return true;
}

