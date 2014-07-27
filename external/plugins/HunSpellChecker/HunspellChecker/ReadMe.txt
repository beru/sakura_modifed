/*
	Copyright (C) 2013, Plugins developers

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

Hunspell spell checker plugin
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

■1. インストール

■1.1. Hunspellのダウンロード
Hunspellは以下のURLからダウンロードできます。

  http://hunspell.sourceforge.net/

Hunspellで使用する辞書は以下のURLからダウンロードできます。

  http://misc.aspell.net/wiki/English_Dictionaries

このURLの"American (en_US)"からhttp://wordlist.sourceforge.net/に進みます。


■1.2. Hunspellのビルド
Hunspellのソースファイルを展開後、以下のプロジェクトを開きます。

  hunspell-1.3.2\src\win_api\Hunspell.sln

構成マネージャを開き"Release_dll"に変更します。
ビルドを実行します。
以下のフォルダにlibhunspell.dllが作成されます。

  hunspell-1.3.2\src\win_api\Release_dll\libhunspell\


■1.3. ファイルの配置
プラグインファイルおよびHunspellファイルを以下のように配置してください。

  <Sakura Path>
    plugins/
      HunspellChecker/
        HunspellChecker.dll
        HunspellChecker_1033.dll
        plugin.def
        local/
          plugin_en_US.def
        Hunspell/
          libhunspell.dll
          en_US.aff
          en_US.dic


■2. 設定
サクラエディタを起動し、プラグインを登録します。
HunspellCheckerの設定画面を開きます。
以下の情報を設定できます。

  Hunspellパス: (デフォルトは ".\Hunspell" です)
  辞書        : (デフォルトは "en_US" です)


■3. 実行
スペルチェックには3種類あります。

  1) ファイル先頭からチェックする
     カーソルがどこにあっても、ファイルの先頭からチェックを開始します。
  2) カーソル位置からチェックする
     カーソルのある位置からチェックを開始します。
  3) カーソル位置の単語をチェックする
     カーソルの位置の単語をチェックします。
     他とは異なり、スペルが正しくても候補を表示します。


