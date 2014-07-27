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
GlobalPlugin
~~~~~~~~~~~~

■GlobalPluginについて
GlobalPluginはGTAGSコマンドを使用してソースコードからタグファイルを作成しキーワー
ドを検索して目的のソースコードにジャンプします。
Sakura-Editor UNICODE版のDLLプラグインとして動作します。


■ファイルの配置
サクラエディタのプラグインフォルダに格納します。

	sakura/
		plugin/
			GlobalPlugin/
				plugin.def
				GlobalPlugin.dll
				GlobalPlugin_1033.dll
				GlobalPlugin.ini	※オプション設定を行うと作成されます
				local/
					plugin_en_US.def
				gtags.exe			※gtags.exeは別途入手してください
				global.exe			※global.exeは別途入手してください
				Result/				※この配下にタグファイルが作成されます

サクラエディタを起動し、共通設定のプラグインタブの「新規プラグインを追加」ボタン
を押してプラグインを登録します。

・GNU GLOBALの入手
globalは以下のURLなどから入手してください。

	http://www.gnu.org/software/global/


■設定
・タグの作成
「GlobalPluginオプションの設定」メニューから画面を開き、タグ化するパスを追加しま
す。
このとき、「作成」ボタンを押してタグを作成します。
タグ化したものの検索の対象に加えたくない場合はリストのチェックをはずします。


■使用方法
・一覧を使用したタグジャンプ
「タグファイルジャンプ一覧」メニューから画面を開き、検索するキーワードを入力しま
す。
リストからキーワードを選択しOKボタンを押します。

・画面のキーワードからジャンプ
検索したいキーワードにカーソルを合わせ、「タグファイルジャンプ」メニューを選択し
ます。
検索キーワードが1件の場合は直接ジャンプします。
検索キーワードが複数ある場合はタグファイルジャンプ一覧画面が表示されます。


