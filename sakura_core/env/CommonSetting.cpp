#include "StdAfx.h"
#include "CommonSetting.h"
#include <vector>
using namespace std;

// CommonValue�Ǘ�
struct CommonValueInfo {
	enum class Type {
		Unknown,
		AStr,    // char������ (�I�[NULL)
		WStr,    // wchar_t������ (�I�[NULL)
	};

	void* pValue;     // �l�ւ̃|�C���^
	int   nValueSize; // �l�̃T�C�Y�B�o�C�g�P�ʁB
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

		// int�Ɠ����T�C�Y�Ȃ�int�Ƃ��ďo��
		if (nValueSize == sizeof(int)) {
			printf("%d\n", *((int*)pValue));
		// ����ȊO�Ȃ�o�C�i���o��
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

// CommonValue ��virtual�g���̋֎~
template <class T>
class CommonValue {
private:
	typedef CommonValue<T> Me;
public:
	CommonValue()
	{
	}
	void Regist(const char* szEntryKey) {
		// CommonValue���X�g�Ɏ�����ǉ�
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

