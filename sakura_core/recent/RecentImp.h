// �eRecent�����N���X�̃x�[�X�N���X
// �G�f�B�^�n�t�@�C������include����Ƃ��� Recent.h ��include
#pragma once

#include "recent/Recent.h"


template <class DATA_TYPE, class RECEIVE_TYPE = const DATA_TYPE*>
class RecentImp : public Recent {
private:
	typedef RecentImp<DATA_TYPE, RECEIVE_TYPE>	Me;
	typedef DATA_TYPE							DataType;
	typedef RECEIVE_TYPE						ReceiveType;

public:
	RecentImp() { Terminate(); }
	virtual ~RecentImp() { Terminate(); }

protected:
	// ����
	bool Create(
		DataType*	pszItemArray,	// �A�C�e���z��ւ̃|�C���^
		size_t*		pnItemCount,	// �A�C�e�����ւ̃|�C���^
		bool*		pbItemFavorite,	// ���C�ɓ���ւ̃|�C���^(NULL����)
		size_t		nArrayCount,	// �ő�Ǘ��\�ȃA�C�e����
		size_t*		pnViewCount		// �\����(NULL����)
	);
public:
	void Terminate();
	bool IsAvailable() const;
	void _Recovery();

	// �X�V
	bool ChangeViewCount(size_t nViewCount);	// �\�����̕ύX
	bool UpdateView();

	// �v���p�e�B�擾�n
	size_t GetArrayCount() const { return nArrayCount; }	// �ő�v�f��
	size_t GetItemCount() const { return (IsAvailable() ? *pnUserItemCount : 0); }	// �o�^�A�C�e����
	size_t GetViewCount() const { return (IsAvailable() ? (pnUserViewCount ? *pnUserViewCount : nArrayCount) : 0); }	// �\����

	// ���C�ɓ��萧��n
	bool SetFavorite(size_t nIndex, bool bFavorite = true);	// ���C�ɓ���ɐݒ�
	bool ResetFavorite(size_t nIndex) { return SetFavorite(nIndex, false); }	// ���C�ɓ��������
	void ResetAllFavorite();			// ���C�ɓ�������ׂĉ���
	bool IsFavorite(size_t nIndex) const;	// ���C�ɓ��肩���ׂ�

	// �A�C�e������
	bool AppendItem(ReceiveType pItemData);	// �A�C�e����擪�ɒǉ�
	bool AppendItemText(LPCTSTR pszText);
	bool EditItemText(size_t nIndex, LPCTSTR pszText);
	bool DeleteItem(size_t nIndex);				// �A�C�e�����N���A
	bool DeleteItem(ReceiveType pItemData) {
		return DeleteItem(FindItem(pItemData));
	}
	bool DeleteItemsNoFavorite();			// ���C�ɓ���ȊO�̃A�C�e�����N���A
	void DeleteAllItem();					// �A�C�e�������ׂăN���A

	// �A�C�e���擾
	const DataType* GetItem(size_t nIndex) const;
	DataType* GetItem(size_t nIndex) { return const_cast<DataType*>(static_cast<const Me*>(this)->GetItem(nIndex)); }
	int FindItem(ReceiveType pItemData) const;
	bool MoveItem(size_t nSrcIndex, size_t nDstIndex);	// �A�C�e�����ړ�

	// �I�[�o�[���C�h�p�C���^�[�t�F�[�X
	virtual int  CompareItem(const DataType* p1, ReceiveType p2) const = 0;
	virtual void CopyItem(DataType* dst, ReceiveType src) const = 0;
	virtual bool DataToReceiveType(ReceiveType* dst, const DataType* src) const = 0;
	virtual bool TextToDataType(DataType* dst, LPCTSTR pszText) const = 0;

	// �����⏕
private:
	const DataType* GetItemPointer(size_t nIndex) const;
	DataType* GetItemPointer(size_t nIndex) { return const_cast<DataType*>(static_cast<const Me*>(this)->GetItemPointer(nIndex)); }
	void   ZeroItem(size_t nIndex);	// �A�C�e�����[���N���A����
	int    GetOldestItem(size_t nIndex, bool bFavorite);	// �ŌẪA�C�e����T��
	bool   CopyItem(size_t nSrcIndex, size_t nDstIndex);

protected:
	// �����t���O
	bool		bCreate;				// Create�ς݂�

	// �O���Q��
	DataType*	puUserItemData;			// �A�C�e���z��ւ̃|�C���^
	size_t*		pnUserItemCount;		// �A�C�e�����ւ̃|�C���^
	bool*		pbUserItemFavorite;		// ���C�ɓ���ւ̃|�C���^ (nullptr����)
	size_t		nArrayCount;			// �ő�Ǘ��\�ȃA�C�e����
	size_t*		pnUserViewCount;		// �\���� (nullptr����)
};

#include "RecentFile.h"
#include "RecentFolder.h"
#include "RecentExceptMru.h"
#include "RecentSearch.h"
#include "RecentReplace.h"
#include "RecentGrepFile.h"
#include "RecentGrepFolder.h"
#include "RecentCmd.h"
#include "RecentCurDir.h"
#include "RecentEditNode.h"
#include "RecentTagjumpKeyword.h"

