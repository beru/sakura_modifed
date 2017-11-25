#pragma once

// http://google-styleguide.googlecode.com/svn/trunk/cppguide.xml#Copy_Constructors
// A macro to disallow the copy constructor and operator= functions
// This should be used in the private: declarations for a class
#define DISALLOW_COPY_AND_ASSIGN(TypeName) \
  TypeName(const TypeName&);               \
  void operator=(const TypeName&)

/*!
	Singleton�p�^�[��
*/
template <class T>
class TSingleton {
public:
	// ���J�C���^�[�t�F�[�X
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
	1�����C���X�^���X�����݂��Ȃ��N���X����̃C���X�^���X�擾�C���^�[�t�F�[�X��static�Œ񋟁B
	Singleton�p�^�[���Ƃ͈قȂ�AInstance()�Ăяo���ɂ��A�C���X�^���X��������������Ȃ��_�ɒ��ӁB
*/
template <class T>
class TSingleInstance {
public:
	// ���J�C���^�[�t�F�[�X
	static T* getInstance() { return g_instance; } // �쐬�ς݂̃C���X�^���X��Ԃ��B�C���X�^���X�����݂��Ȃ���� nullptr�B

protected:
	// ��2�ȏ�̃C���X�^���X�͑z�肵�Ă��܂���Bassert���j�]�����o���܂��B
	TSingleInstance() { assert(g_instance == nullptr); g_instance = static_cast<T*>(this); }
	~TSingleInstance() { assert(g_instance); g_instance = nullptr; }
private:
	static T* g_instance;
};
template <class T>
T* TSingleInstance<T>::g_instance = nullptr;


// �L�^������
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

