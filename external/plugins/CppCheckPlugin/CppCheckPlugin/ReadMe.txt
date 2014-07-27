/*
	Copyright (C) 2013-2014, Plugins developers

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

CppCheckPlugin
~~~~~~~~~~~~~~

■CppCheckPluginについて
CppCheckPluginはCppcheckコマンドを使用してソースコードを解析し指摘箇所にジャンプ
します。
Sakura-Editor UNICODE版のDLLプラグインとして動作します。


■ファイルの配置
サクラエディタのプラグインフォルダに格納します。

	sakura/
		plugin/
			CppCheckPlugin/
				plugin.def
				CppCheckPlugin.dll
				CppCheckPlugin_1033.dll
				local/
					plugin_en_US.def
				cppcheck_result.xml	※Cppcheck実行結果ファイルが作成されます

サクラエディタを起動し、共通設定のプラグインタブの「新規プラグインを追加」ボタン
を押してプラグインを登録します。

・cppcheckコマンドの入手
cppcheckコマンドは以下のURLなどから入手してください。

	http://cppcheck.sourceforge.net/


■設定
cppcheckコマンドを設定します。

  Cppcheck Path: cppcheck.exeのパスを指定します。
                 (Default: cppcheck.exe)
  Platform     : プラットフォーム(unix32, unix64, win32A, win32W, win64)を指定します。
                 (Default: win32W)


■使用方法
・cppcheckの実行
現在開いているファイルのcppcheckを実行します。
過去に実行済みの場合は実行結果を表示しています。


