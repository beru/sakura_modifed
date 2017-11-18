/*!	@file
	@brief プラグイン基本クラス
*/
#include "StdAfx.h"
#include <vector>		// wstring_split用 2010/4/4 Uchi
#include "Plugin.h"
#include "JackManager.h"

/////////////////////////////////////////////
// Plug メンバ関数
bool Plug::Invoke(EditView& view, WSHIfObj::List& params) {
	return plugin.InvokePlug(view, *this, params);
}

EFunctionCode Plug::GetFunctionCode() const {
	return GetPluginFunctionCode(plugin.id, id);
}

/////////////////////////////////////////////
// Plugin メンバ関数

// コンストラクタ
Plugin::Plugin(const tstring& sBaseDir)
	: sBaseDir(sBaseDir)
{
	nCommandCount = 0;
}

// デストラクタ
Plugin::~Plugin(void)
{
	for (auto it=options.begin(); it!=options.end(); ++it) {
		delete *it;
	}
}

// プラグイン定義ファイルのCommonセクションを読み込む
bool Plugin::ReadPluginDefCommon(
	DataProfile& profile,
	DataProfile* profileMlang
	)
{
	profile.IOProfileData(PII_PLUGIN, PII_PLUGIN_ID, sId);
	profile.IOProfileData(PII_PLUGIN, PII_PLUGIN_NAME, sName);
	profile.IOProfileData(PII_PLUGIN, PII_PLUGIN_DESCRIPTION, sDescription);
	profile.IOProfileData(PII_PLUGIN, PII_PLUGIN_AUTHOR, sAuthor);
	profile.IOProfileData(PII_PLUGIN, PII_PLUGIN_VERSION, sVersion);
	profile.IOProfileData(PII_PLUGIN, PII_PLUGIN_URL, sUrl);
	if (profileMlang) {
		profileMlang->IOProfileData(PII_PLUGIN, PII_PLUGIN_NAME, sName);
		profileMlang->IOProfileData(PII_PLUGIN, PII_PLUGIN_DESCRIPTION, sDescription);
		profileMlang->IOProfileData(PII_PLUGIN, PII_PLUGIN_URL, sUrl);
	}

	DEBUG_TRACE(_T("    Name:%ls\n"), sName.c_str());
	DEBUG_TRACE(_T("    Description:%ls\n"), sDescription.c_str());
	DEBUG_TRACE(_T("    Author:%ls\n"), sAuthor.c_str());
	DEBUG_TRACE(_T("    Version:%ls\n"), sVersion.c_str());
	DEBUG_TRACE(_T("    Url:%ls\n"), sUrl.c_str());

	return true;
}

// プラグイン定義ファイルのPlugセクションを読み込む
// @date 2011.08.20 syat Plugセクションも複数定義可能とする
bool Plugin::ReadPluginDefPlug(
	DataProfile& profile,
	DataProfile* profileMlang
	)
{
	std::vector<JackDef> jacks = JackManager::getInstance().GetJackDef();
	wchar_t szIndex[8];

	for (size_t i=0; i<jacks.size(); ++i) {
		const wstring& sKey = jacks[i].szName;
		for (int nCount=0; nCount<MAX_PLUG_CMD; ++nCount) {
			if (nCount == 0) {
				szIndex[0] = L'\0';
			}else {
				swprintf(szIndex, L"[%d]", nCount);
			}
			wstring sHandler;
			if (profile.IOProfileData(PII_PLUG, (sKey + szIndex).c_str(), sHandler)) {
				// ラベルの取得
				wstring sKeyLabel = sKey + szIndex + L".Label";
				wstring sLabel;
				profile.IOProfileData(PII_PLUG, sKeyLabel.c_str(), sLabel);
				if (profileMlang) {
					profileMlang->IOProfileData(PII_PLUG, sKeyLabel.c_str(), sLabel);
				}
				if (sLabel == L"") {
					sLabel = sHandler;		// Labelが無ければハンドラ名で代用
				}

				Plug *newPlug = CreatePlug(*this, nCount, sKey, sHandler, sLabel);
				plugs.push_back(newPlug);
			}else {
				break;		// 定義がなければ読み込みを終了
			}
		}
	}

	return true;
}

// プラグイン定義ファイルのCommandセクションを読み込む
bool Plugin::ReadPluginDefCommand(
	DataProfile& profile,
	DataProfile* profileMlang
	)
{
	wstring sHandler;
	wchar_t bufKey[64];

	for (int nCount=1; nCount<MAX_PLUG_CMD; ++nCount) {	// 添え字は１から始める
		swprintf(bufKey, L"C[%d]", nCount);
		if (profile.IOProfileData(PII_COMMAND, bufKey, sHandler)) {
			wstring sLabel;
			wstring sIcon;

			// ラベルの取得
			swprintf(bufKey, L"C[%d].Label", nCount);
			profile.IOProfileData(PII_COMMAND, bufKey, sLabel);
			if (profileMlang) {
				profileMlang->IOProfileData(PII_COMMAND, bufKey, sLabel);
			}
			if (sLabel == L"") {
				sLabel = sHandler;		// Labelが無ければハンドラ名で代用
			}
			// アイコンの取得
			swprintf(bufKey, L"C[%d].Icon", nCount);
			profile.IOProfileData(PII_COMMAND, bufKey, sIcon);
			if (profileMlang) {
				profileMlang->IOProfileData(PII_COMMAND, bufKey, sIcon);
			}

			AddCommand(sHandler.c_str(), sLabel.c_str(), sIcon.c_str(), false);
		}else {
			break;		// 定義がなければ読み込みを終了
		}
	}

	return true;
}

// プラグイン定義ファイルのOptionセクションを読み込む	// 2010/3/24 Uchi
bool Plugin::ReadPluginDefOption(
	DataProfile& profile,
	DataProfile* profileMlang
	)
{
	wstring sLabel;
	wstring sSection;
	wstring sSection_wk;
	wstring sKey;
	wstring sType;
	wstring sSelect;
	wstring sDefaultVal;
	wchar_t bufKey[64];

	sSection = L"";
	for (int nCount=1; nCount<MAX_PLUG_OPTION; ++nCount) {	// 添え字は１から始める
		sKey = sLabel = sType = sDefaultVal= L"";
		// Keyの取得
		swprintf(bufKey, L"O[%d].Key", nCount);
		if (profile.IOProfileData(PII_OPTION, bufKey, sKey)) {
			// Sectionの取得
			swprintf(bufKey, L"O[%d].Section", nCount);
			profile.IOProfileData(PII_OPTION, bufKey, sSection_wk);
			if (!sSection_wk.empty()) {		// 指定が無ければ前を引き継ぐ
				sSection = sSection_wk;
			}
			// ラベルの取得
			swprintf(bufKey, L"O[%d].Label", nCount);
			profile.IOProfileData(PII_OPTION, bufKey, sLabel);
			if (profileMlang) {
				profileMlang->IOProfileData(PII_OPTION, bufKey, sLabel);
			}
			// Typeの取得
			swprintf(bufKey, L"O[%d].Type", nCount);
			profile.IOProfileData(PII_OPTION, bufKey, sType);
			// 項目選択候補
			swprintf(bufKey, L"O[%d].Select", nCount);
			profile.IOProfileData(PII_OPTION, bufKey, sSelect);
			if (profileMlang) {
				profileMlang->IOProfileData(PII_OPTION, bufKey, sSelect);
			}
			// デフォルト値
			swprintf(bufKey, L"O[%d].Default", nCount);
			profile.IOProfileData(PII_OPTION, bufKey, sDefaultVal);

			if (sSection.empty() || sKey.empty()) {
				// 設定が無かったら無視
				continue;
			}
			if (sLabel.empty()) {
				// Label指定が無ければ、Keyで代用
				sLabel = sKey;
			}

			options.push_back(
				new PluginOption(*this, sLabel, sSection, sKey, sType, sSelect, sDefaultVal, nCount)
				);
		}
	}

	return true;
}

// プラグインフォルダ基準の相対パスをフルパスに変換
Plugin::tstring Plugin::GetFilePath(const tstring& sFileName) const
{
	return sBaseDir + _T("\\") + to_tchar(sFileName.c_str());
}

Plugin::tstring Plugin::GetFolderName() const
{
	return tstring(GetFileTitlePointer(sBaseDir.c_str()));
}

// コマンドを追加する
int Plugin::AddCommand(
	const wchar_t* handler,
	const wchar_t* label,
	const wchar_t* icon,
	bool doRegister
	)
{
	if (!handler) { handler = L""; }
	if (!label) { label = L""; }

	// コマンドプラグIDは1から振る
	++nCommandCount;
	Plug* newPlug = CreatePlug(*this, nCommandCount, PP_COMMAND_STR, wstring(handler), wstring(label));
	if (icon) {
		newPlug->sIcon = icon;
	}

	plugs.push_back(newPlug);

	if (doRegister) {
		JackManager::getInstance().RegisterPlug(PP_COMMAND_STR, newPlug);
	}
	return newPlug->GetFunctionCode();
}

// 文字列分割	2010/4/4 Uchi
//	独立させたほうがいいのだが
std::vector<std::wstring> wstring_split(
	std::wstring sTrg,
	wchar_t cSep
	)
{
    std::vector<std::wstring> splitVec;
    size_t idx;
    while ((idx = sTrg.find(cSep)) != std::wstring::npos) {
        splitVec.emplace_back(sTrg.substr(0, idx));
        sTrg = sTrg.substr(++idx);
    }
	if (sTrg != L"") {
		splitVec.push_back(sTrg);
	}

    return splitVec;
}

// プラグイン定義ファイルのStringセクションを読み込む
bool Plugin::ReadPluginDefString(
	DataProfile& profile,
	DataProfile* profileMlang
	)
{
	wchar_t bufKey[64];
	aStrings.clear();
	aStrings.emplace_back(); // 0番目ダミー
	for (int nCount=1; nCount<MAX_PLUG_STRING; ++nCount) {	// 添え字は１から始める
		wstring sVal = L"";
		swprintf(bufKey, L"S[%d]", nCount);
		if (profile.IOProfileData(PII_STRING, bufKey, sVal)) {
			if (profileMlang) {
				profileMlang->IOProfileData(PII_STRING, bufKey, sVal);
			}
		}
		aStrings.push_back(sVal);
	}
	return true;
}

