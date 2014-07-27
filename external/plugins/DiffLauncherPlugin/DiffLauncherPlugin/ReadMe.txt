/*
	Copyright (C) 2014, Plugins developers

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

Diff Launcher plugin
~~~~~~~~~~~~~~~~~~~~

■1. インストール
※このプラグインは同一共有番号のsakura.exeでしか動作しません。
新しい共有番号で動作させたい場合は、このプラグインをリビルドしてください。

■1.1. 差分コマンドの準備
WinMergeのような差分アプリケーションを用意してください。

■1.2. ファイルの配置
プラグインファイルおよびDiffLauncherPluginファイルを以下のように配置してください。

  <Sakura Path>
    plugins/
      DiffLauncherPlugin/
        DiffLauncherPlugin.dll
        DiffLauncherPlugin_1033.dll
        plugin.def
        local/
          plugin_en_US.def


■2. 設定
サクラエディタを起動し、プラグインを登録します。
DiffLauncherPluginの設定画面を開きます。
以下の情報を設定できます。

  Command Path: 差分アプリケーションのパスを指定します。
                デフォルト: "C:\Program Files (x86)\WinMerge\WinMergeU.exe"


■3. 実行
・ファイルの差分を確認する
編集中であっても書き込みは行わないため、編集前の状態で比較されます。

