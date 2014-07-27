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

CSharp plugin
~~~~~~~~~~~~~

■CSharpPluginについて
編集中のC#スクリプトコードをコンパイルして実行します。
Sakura-Editor UNICODE版のDLLプラグインとして動作します。

■ファイルの配置
プラグインファイルおよびCSharpPluginファイルを以下のように配置してください。

  <Sakura Path>
    plugins/
      CSharpPlugin/
        CSharpPlugin.dll
        CSharpPlugin_1033.dll
        plugin.def
        local/
          plugin_en_US.def


■設定
サクラエディタを起動し、プラグインを登録します。
CSharpPluginの設定画面を開きます。
以下の情報を設定できます。

  Command Path: csc.exeのパスを指定します。
                デフォルト: "C:\Windows\Microsoft.NET\Framework\v4.0.30319\csc.exe"
  Select Mode : 0: 常にファイルの全部を対象にします。
                1: ファイルの一部を選択している場合には、その部分だけを対象にします。
                デフォルト: 0 (Always all text)


■実行
・C#のコンパイル
編集中のC#スクリプトをコンパイルします。

・C#のコンパイルと実行
編集中のC#スクリプトをコンパイルし、さらに実行します。

