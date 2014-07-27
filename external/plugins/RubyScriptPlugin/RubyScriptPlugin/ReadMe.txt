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

Ruby script plugin
~~~~~~~~~~~~~~~~~~

■1. インストール

■1.1. Rubyのダウンロード
Rubyは以下のURLからダウンロードできます。

  https://www.ruby-lang.org/ja/

このURLの"ダウンロード"から

  "One-Click Ruby Installer for Windows" http://rubyinstaller.org/

に進みます。ダウンロードのページから "RubyInstaller 2.0.0-p*** released"
をダウンロードしインストールしてください。

※RubyScriptPluginではRuby-2.0.0を使用します。Ruby-1.8以下はAPIが異なるため
使用できません。


■1.3. ファイルの配置
プラグインファイルおよびRubyScriptPluginファイルを以下のように配置してください。

  <Sakura Path>
    plugins/
      RubyScriptPlugin/
        RubyScriptPlugin.dll
        RubyScriptPlugin_1033.dll
        plugin.def
        local/
          plugin_en_US.def


■2. 設定
サクラエディタを起動し、プラグインを登録します。
RubyScriptPluginの設定画面を開きます。
以下の情報を設定できます。

  Ruby Module Path: (デフォルトは "msvcrt-ruby200.dll" です)


■3. 実行
Rubyスクリプトを作成します。ファイルの拡張子は"mrb"にしてください。
マクロ設定画面でRubyスクリプトを登録します。
マクロを実行します。


