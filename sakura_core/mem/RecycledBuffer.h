// 一時的なメモリブロックをローテーションして使いまわすためのモノ
// Getで取得したメモリブロックは、「ある程度の期間」上書きされないことが保障される。
// その「期間」とは、Getを呼んでから再度CHAIN_COUNT回、Getを呼び出すまでの間である。
// 取得したメモリブロックはRecycledBufferの管理下にあるため、解放してはいけない。
#pragma once

class RecycledBuffer {
// コンフィグ
private:
	static const int BLOCK_SIZE  = 1024;	// ブロックサイズ。バイト単位。
	static const int CHAIN_COUNT = 64;		// 再利用可能なブロック数。

// コンストラクタ・デストラクタ
public:
	RecycledBuffer() {
		current=0;
	}

// インターフェース
public:
	// 一時的に確保されたメモリブロックを取得。このメモリブロックを解放してはいけない。
	template <class T>
	T* GetBuffer(
		size_t* nCount // [out] 領域の要素数を受け取る。T単位。
		)
	{
		if (nCount) {
			*nCount = BLOCK_SIZE / sizeof(T);
		}
		current = (current + 1) % CHAIN_COUNT;
		return reinterpret_cast<T*>(buf[current]);
	}

	// 領域の要素数を取得。T単位
	template <class T>
	size_t GetMaxCount() const {
		return BLOCK_SIZE / sizeof(T);
	}


// メンバ変数
private:
	BYTE buf[CHAIN_COUNT][BLOCK_SIZE];
	int  current;
};

class RecycledBufferDynamic {
// コンフィグ
private:
	static const int CHAIN_COUNT = 64;   // 再利用可能なブロック数。

// コンストラクタ・デストラクタ
public:
	RecycledBufferDynamic() {
		current = 0;
		for (size_t i=0; i<_countof(buf); ++i) {
			buf[i] = NULL;
		}
	}
	~RecycledBufferDynamic() {
		for (size_t i=0; i<_countof(buf); ++i) {
			if (buf[i]) delete[] buf[i];
		}
	}

// インターフェース
public:
	// 一時的に確保されたメモリブロックを取得。このメモリブロックを解放してはいけない。
	template <class T>
	T* GetBuffer(
		size_t nCount // [in] 確保する要素数。T単位。
	)
	{
		current = (current + 1) % CHAIN_COUNT;

		// メモリ確保
		if (buf[current]) {
			delete[] buf[current];
		}
		buf[current] = new BYTE[nCount * sizeof(T)];

		return reinterpret_cast<T*>(buf[current]);
	}

// メンバ変数
private:
	BYTE* buf[CHAIN_COUNT];
	size_t current;
};

