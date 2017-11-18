/*!	@file
	@brief プロセス生成クラスヘッダファイル
*/

#pragma once

#include "global.h"

class Process;

/*-----------------------------------------------------------------------
クラスの宣言
-----------------------------------------------------------------------*/
/*!
	@brief プロセス生成クラス

	与えられたコマンドライン引数から生成すべきプロセスの種別を判定し，
	対応するオブジェクトを返すFactoryクラス．

	通常のエディタプロセスの起動が指定された場合には，必要に応じてコントロールプロセス
	起動の起動をエディタの起動に先立って行う．
*/
class ProcessFactory {
public:
	Process* Create(HINSTANCE hInstance, LPCTSTR lpCmdLine);
protected:
private:
	bool IsValidVersion();
	bool ProfileSelect(HINSTANCE, LPCTSTR);
	bool IsStartingControlProcess();
	bool IsExistControlProcess();
	bool StartControlProcess();
	bool WaitForInitializedControlProcess();
	bool TestWriteQuit();
};

