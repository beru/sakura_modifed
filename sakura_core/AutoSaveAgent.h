#pragma once

#include <Windows.h>
#include "_main/global.h"
#include "doc/DocListener.h"

// 分→ミリ秒に変換するための係数
const int MSec2Min = 1000 * 60;

/*! @class PassiveTimer CAutoSave.h
	基準時刻からの経過時間が設定間隔を過ぎたかどうかを判定する。
	頻繁に呼び出されるタイマーが既に別の場所にあるとき、それよりも間隔が広くて
	間隔の厳密さが要求されない用途に利用可能。
	ファイルの自動保存で使っている。
*/
class PassiveTimer {
public:
	/*!
		初期値は間隔1msecでタイマーは無効。
	*/
	PassiveTimer() : nInterval(1), bEnabled(false) { Reset(); }

	// 時間間隔
	void SetInterval(int m);	// 時間間隔の設定
	int GetInterval(void) const { return nInterval / MSec2Min; }	// 時間間隔の取得
	void Reset(void) { nLastTick = ::GetTickCount(); }			// 基準時刻のリセット

	// 有効／無効
	void Enable(bool flag);							// 有効／無効の設定
	bool IsEnabled(void) const { return bEnabled; }	// 有効／無効の読み出し

	// 規定時間に達したかどうかの判定
	bool CheckAction(void);

private:
	DWORD	nLastTick;	// 最後にチェックしたときの時刻 (GetTickCount()で取得したもの)
	int		nInterval;	// Action間隔 (分)
	bool	bEnabled;	// 有効かどうか
};


class AutoSaveAgent : public DocListenerEx {
public:
	void CheckAutoSave();
	void ReloadAutoSaveParam();	// 設定をSharedAreaから読み出す

private:
	PassiveTimer passiveTimer;
};

