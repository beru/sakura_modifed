#pragma once

// 処理所要時間の計測クラス

#include <windows.h>
// RunningTimerで経過時間の測定を行う場合にはコメントを外してください
//#define TIME_MEASURE

/*-----------------------------------------------------------------------
クラスの宣言
-----------------------------------------------------------------------*/
/*!
	@brief 処理所要時間の計測クラス

	定義の切り替えのみでタイマーのON/OFFを行えるようにするため，
	このクラスを直接使わず，後ろにあるMY_RUNNINGTIMERとMY_TRACETIMEを
	使うこと．
*/
class RunningTimer {
public:
	/*
	||  Constructors
	*/
	RunningTimer(const char* Text = NULL);
	~RunningTimer();

	/*
	|| 関数
	*/
	void Reset();
	DWORD Read();
	
	void WriteTrace(const char* msg = "") const;

protected:
	DWORD	nStartTime;
	char	szText[100];	// タイマー名
	int		nDepth;		// このオブジェクトのネストの深さ

#if 1 //def _DEBUG
	static int nNestCount;
#endif
};

// Oct. 16, 2002 genta
//#ifdef _DEBUG～#endifで逐一囲まなくても簡単にタイマーのON/OFFを行うためのマクロ
#if defined(_DEBUG) && defined(TIME_MEASURE)
  #define MY_TRACETIME(c, m) (c).WriteTrace(m)
  #define MY_RUNNINGTIMER(c, m) RunningTimer c(m)
#else
  #define MY_TRACETIME(c, m)
  #define MY_RUNNINGTIMER(c, m)
#endif

