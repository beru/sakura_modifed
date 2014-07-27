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

Speech plugin
~~~~~~~~~~~~~

■1. インストール
Windows8で動作します。
Windows8ではSpeech to textの日本語音声合成(Microsoft Haruka Desktop - Japanese)
が最初から用意されています。

■1.1. Speech to Textのダウンロード (Windows7の場合)
Windows7日本語版では日本語音声合成ができません。
Microsoft Speech Platform - Runtime (Version 11)をインストールするとHarukaが
インストールされるらしいです。

  http://www.microsoft.com/en-us/download/details.aspx?id=27225

あなたのコンピュータで使用できるかどうかは、コントロールパネルで確認してくださ
い。
  コントロールパネル
    コンピュータの簡単操作
      音声認識
        音声合成
          音声の種類

■1.2. ファイルの配置
プラグインファイルおよびSpeechPluginファイルを以下のように配置してください。

  <Sakura Path>
    plugins/
      SpeechPlugin/
        SpeechPlugin.dll
        SpeechPlugin_1033.dll
        plugin.def
        local/
          plugin_en_US.def


■2. 設定
サクラエディタを起動し、プラグインを登録します。
SpeechPluginの設定画面を開きます。
以下の情報を設定できます。

  Voice type: 音声の種類を指定します
              (デフォルトは "Microsoft Haruka Desktop - Japanese" です)
  Volume    : 0 ... 100 で音量を指定します (デフォルトは100です)
  Rate      : -10 ... +10 で音声の速を指定します (デフォルトは0です)


■3. 実行
・読み上げ画面を開く
読み上げ画面を表示します。
音声の種類、音声の速度、音量を設定することができます。
ここで指定した音声の種類、音声の速度、音量は保存され、読み上げで使用されます。
カーソル行を読み上げます。

・カーソル位置の単語を読み上げ
カーソル単語を読み上げます。

・カーソル行を読み上げ
カーソル行を読み上げます。

・すべてのテキストを読み上げ
すべてのテキストを読み上げます。
テキストサイズが大きすぎると無応答になるかもしれません。

・選択されたテキストを読み上げ
選択されているテキストを読み上げます。

・読み上げを中止
読み上げを中止します。

