#pragma once

#include "util/StaticType.h"
#include "Profile.h"

// 文字列バッファの型
template <typename T>
struct StringBuffer {
	T*	pData;
	const size_t nDataCount;

	StringBuffer(T* pData, size_t nDataCount)
		:
		pData(pData),
		nDataCount(nDataCount)
	{ }

	StringBuffer& operator = (const StringBuffer& rhs) {
		auto_strcpy_s(pData, nDataCount, rhs.pData);
		return *this;
	}
};

typedef const StringBuffer<char> StringBufferA;
typedef const StringBuffer<wchar_t> StringBufferW;
typedef StringBufferW StringBufferT;

// 文字列バッファ型インスタンスの生成マクロ
#define MakeStringBufferW(S) StringBufferW(S, _countof(S))
#define MakeStringBufferA(S) StringBufferA(S, _countof(S))
#define MakeStringBufferT(S) StringBufferT(S, _countof(S))
#define MakeStringBufferW0(S) StringBufferW(S, 0)
#define MakeStringBufferT0(S) StringBufferT(S, 0)


// 各種データ変換付きProfile
class DataProfile : public Profile {
private:
	// 専用型
	typedef std::wstring wstring;

protected:
	static const wchar_t* _work_itow(int n) {
		static wchar_t buf[32];
		_itow(n, buf, 10);
		return buf;
	}
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//                       データ変換部                          //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
protected:
	//bool
	void profile_to_value(const wstring& profile, bool* value) {
		if (profile != L"0") *value = true;
		else *value = false;
	}
	void value_to_profile(const bool& value, wstring* profile) {
		*profile = _work_itow(value ? 1 : 0);
	}
	// int
	void profile_to_value(const wstring& profile, int* value) {
		*value = _wtoi(profile.c_str());
	}
	void profile_to_value(const wstring& profile, size_t* value) {
		*value = _wtoi(profile.c_str());
	}
	void value_to_profile(const int& value, wstring* profile) {
		*profile = _work_itow(value);
	}
	void value_to_profile(const size_t& value, wstring* profile) {
		ASSERT_GE(INT32_MAX, value);
		*profile = _work_itow((int)value);
	}
	void profile_to_value(const wstring& profile, long* value) {
		*value = _wtol(profile.c_str());
	}

	// int式入出力実装マクロ
	#define AS_INT(TYPE) \
		void profile_to_value(const wstring& profile, TYPE* value) { *value = (TYPE)_wtoi(profile.c_str()); } \
		void value_to_profile(const TYPE& value, wstring* profile) { *profile = _work_itow(value);    }

	// int式
// Type.hをincludeしないといけないから廃止
//	AS_INT(EOutlineType) 
	AS_INT(WORD)
	AS_INT(UINT)

	// char
	void profile_to_value(const wstring& profile, char* value) {
		if (profile.length() > 0) {
			char buf[2] = {0};
			int ret = wctomb(buf, profile[0]);
			assert_warning(ret == 1);
			(void)ret;
			*value = buf[0];
		}else {
			*value = '\0';
		}
	}
	void value_to_profile(const char& value, wstring* profile) {
		wchar_t buf[2] = {0};
		mbtowc(buf, &value, 1);
		profile->assign(1, buf[0]);
	}
	// wchar_t
	void profile_to_value(const wstring& profile, wchar_t* value) {
		*value = profile[0];
	}
	void value_to_profile(const wchar_t& value, wstring* profile) {
		profile->assign(1, value);
	}
	// StringBufferW
	void profile_to_value(const wstring& profile, StringBufferW* value) {
		wcscpy_s(value->pData, value->nDataCount, profile.c_str());
	}
	void value_to_profile(const StringBufferW& value, wstring* profile) {
		*profile = value.pData;
	}
	// StringBufferA
	void profile_to_value(const wstring& profile, StringBufferA* value) {
		strcpy_s(value->pData, value->nDataCount, to_achar(profile.c_str()));
	}
	void value_to_profile(const StringBufferA& value, wstring* profile) {
		*profile = to_wchar(value. pData);
	}
	// StaticString<wchar_t, N>
	template <int N>
	void profile_to_value(const wstring& profile, StaticString<wchar_t, N>* value) {
		wcscpy_s(value->GetBufferPointer(), value->GetBufferCount(), profile.c_str());
	}
	template <int N>
	void value_to_profile(const StaticString<wchar_t, N>& value, wstring* profile) {
		*profile = value.GetBufferPointer();
	}
	// StaticString<char, N>
	template <int N>
	void profile_to_value(const wstring& profile, StaticString<char, N>* value) {
		strcpy_s(value->GetBufferPointer(), value->GetBufferCount(), to_achar(profile.c_str()));
	}
	template <int N>
	void value_to_profile(const StaticString<char, N>& value, wstring* profile) {
		*profile = to_wchar(value. GetBufferPointer());
	}
	// wstring
	void profile_to_value(const wstring& profile, wstring* value) {
		*value = profile;
	}
	void value_to_profile(const wstring& value, wstring* profile) {
		*profile = value;
	}

	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//                         入出力部                            //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
public:
	// 注意：StringBuffer系はバッファが足りないとabortします
	template <class T> //T=={bool, int, WORD, wchar_t, char, wstring, StringBufferA, StringBufferW, StaticString}
	bool IOProfileData(const wchar_t* pszSectionName, const wchar_t* pszEntryKey, T& tEntryValue) {
		// 読み込み
		if (bRead) {
			// 文字列読み込み
			wstring buf;
			bool ret = GetProfileDataImp(pszSectionName, pszEntryKey, buf);
			if (ret) {
				// Tに変換
				profile_to_value(buf, &tEntryValue);
			}
			return ret;
		// 書き込み
		}else {
			// 文字列に変換
			wstring buf;
			value_to_profile(tEntryValue, &buf);
			// 文字列書き込み
			return SetProfileDataImp(pszSectionName, pszEntryKey, buf);
		}
	}

	// intを介して任意型の入出力を行う
	template <class T>
	bool IOProfileData_WrapInt(const wchar_t* pszSectionName, const wchar_t* pszEntryKey, T& nEntryValue) {
		int n = (int)nEntryValue;
		bool ret = this->IOProfileData(pszSectionName, pszEntryKey, n);
		nEntryValue = (T)n;
		return ret;
	}
};

