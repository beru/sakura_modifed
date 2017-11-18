#include "StdAfx.h"
#include "CommonSetting.h"
#include <vector>
using namespace std;

// CommonValue管理
struct CommonValueInfo {
	enum class Type {
		Unknown,
		AStr,    // char文字列 (終端NULL)
		WStr,    // wchar_t文字列 (終端NULL)
	};

	void* pValue;     // 値へのポインタ
	int   nValueSize; // 値のサイズ。バイト単位。
	char  szEntryKey[32];
	Type type;

	CommonValueInfo(
		void* pValue,
		int nValueSize,
		const char* szEntryKey,
		Type type = Type::Unknown
	)
		:
		pValue(pValue),
		nValueSize(nValueSize),
		type(type)
	{
		strcpy_s(this->szEntryKey, _countof(this->szEntryKey), szEntryKey);
	}

	void Save()
	{
		printf("%hs=", szEntryKey);

		// intと同じサイズならintとして出力
		if (nValueSize == sizeof(int)) {
			printf("%d\n", *((int*)pValue));
		// それ以外ならバイナリ出力
		}else {
			for (int i=0; i<nValueSize; ++i) {
				printf("%%%02X", ((BYTE*)pValue)[i]);
			}
		}
	}
};
vector<CommonValueInfo> g_commonvalues;
void CommonValue_AllSave()
{
	int nSize = (int)g_commonvalues.size();
	for (int i=0; i<nSize; ++i) {
		g_commonvalues[i].Save();
	}
}

// CommonValue ※virtual使うの禁止
template <class T>
class CommonValue {
private:
	typedef CommonValue<T> Me;
public:
	CommonValue()
	{
	}
	void Regist(const char* szEntryKey) {
		// CommonValueリストに自分を追加
		g_commonvalues.emplace_back(&value, sizeof(value), szEntryKey);
	}
	Me& operator = (const T& rhs) { value = rhs; return *this; }
	operator T& () { return value; }
	operator const T& () const { return value; }
private:
	T value;
};

typedef char mystring[10];

void sample()
{
	CommonValue<int>      intvalue;
	CommonValue<mystring> strvalue;

	intvalue.Regist("intvalue");
	strvalue.Regist("strvalue");

	intvalue = 3;
	strcpy(strvalue, "hage");

	CommonValue_AllSave();
}

