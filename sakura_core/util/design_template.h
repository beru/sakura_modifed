#pragma once

// http://google-styleguide.googlecode.com/svn/trunk/cppguide.xml#Copy_Constructors
// A macro to disallow the copy constructor and operator= functions
// This should be used in the private: declarations for a class
#define DISALLOW_COPY_AND_ASSIGN(TypeName) \
  TypeName(const TypeName&);               \
  void operator=(const TypeName&)

/*!
	Singletonパターン
*/
template <class T>
class TSingleton {
public:
	// 公開インターフェース
	static T& getInstance() {
		static T instance;
		return instance;
	}

protected:
	TSingleton() {}
private:
	DISALLOW_COPY_AND_ASSIGN(TSingleton);
};

/*!
	1個しかインスタンスが存在しないクラスからのインスタンス取得インターフェースをstaticで提供。
	Singletonパターンとは異なり、Instance()呼び出しにより、インスタンスが自動生成されない点に注意。
*/
template <class T>
class TSingleInstance {
public:
	// 公開インターフェース
	static T* getInstance() { return g_instance; } // 作成済みのインスタンスを返す。インスタンスが存在しなければ nullptr。

protected:
	// ※2個以上のインスタンスは想定していません。assertが破綻を検出します。
	TSingleInstance() { assert(g_instance == nullptr); g_instance = static_cast<T*>(this); }
	~TSingleInstance() { assert(g_instance); g_instance = nullptr; }
private:
	static T* g_instance;
};
template <class T>
T* TSingleInstance<T>::g_instance = nullptr;


// 記録もする
#include <vector>
template <class T>
class TInstanceHolder {
public:
	TInstanceHolder() {
		g_table.push_back(static_cast<T*>(this));
	}
	virtual ~TInstanceHolder() {
		for (size_t i=0; i<g_table.size(); ++i) {
			if (g_table[i] == static_cast<T*>(this)) {
				g_table.erase(g_table.begin() + i);
				break;
			}
		}
	}
	static int GetInstanceCount() { return (int)g_table.size(); }
	static T* GetInstance(int nIndex) {
		if (nIndex >= 0 && nIndex < (int)g_table.size()) {
			return g_table[nIndex];
		}else {
			return nullptr;
		}
	}
private:
	static std::vector<T*> g_table;
};

template <class T>
std::vector<T*> TInstanceHolder<T>::g_table;

