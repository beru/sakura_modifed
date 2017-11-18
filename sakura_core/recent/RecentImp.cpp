#include "StdAfx.h"
#include "RecentImp.h"

#include "env/AppNodeManager.h" // EditNode
#include "EditInfo.h" // EditInfo

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                           ����                              //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

/*
	������������

	@note
	nCmpType = strcmp, stricmp �̂Ƃ��� nCmpSize = 0 ���w�肷��ƁAAppendItem 
	�ł̃f�[�^��������ł���ƔF������ strcpy ������B
	���̏ꍇ�� memcpy �� nItemSize �����R�s�[����B
	
	pnViewCount = nullptr �ɂ���ƁA�[���I�� nViewCount == nArrayCount �ɂȂ�B
*/
template <class T, class S>
bool RecentImp<T, S>::Create(
	DataType*	pszItemArray,	// �A�C�e���z��ւ̃|�C���^
	size_t*		pnItemCount,	// �A�C�e�����ւ̃|�C���^
	bool*		pbItemFavorite,	// ���C�ɓ���ւ̃|�C���^(NULL����)
	size_t		nArrayCount,	// �ő�Ǘ��\�ȃA�C�e����
	size_t*		pnViewCount		// �\����(NULL����)
	)
{
	Terminate();

	// �p�����[�^�`�F�b�N
	if (!pszItemArray) return false;
	if (!pnItemCount) return false;
	if (nArrayCount == 0) return false;
	if (pnViewCount && (nArrayCount < *pnViewCount)) return false;

	// �e�p�����[�^�i�[
	this->puUserItemData		= pszItemArray;
	this->pnUserItemCount		= pnItemCount;
	this->pbUserItemFavorite	= pbItemFavorite;
	this->nArrayCount			= nArrayCount;
	this->pnUserViewCount		= pnViewCount;
	this->bCreate = true;

	// �ʂɑ��삳��Ă����Ƃ��̂��߂̑Ή�
	UpdateView();

	return true;
}

/*
	�I������
*/
template <class T, class S>
void RecentImp<T, S>::Terminate()
{
	bCreate = false;

	puUserItemData     = nullptr;
	pnUserItemCount    = nullptr;
	pnUserViewCount    = nullptr;
	pbUserItemFavorite = nullptr;

	nArrayCount  = 0;
}


/*
	�������ς݂����ׂ�B
*/
template <class T, class S>
bool RecentImp<T, S>::IsAvailable() const
{
	if (!bCreate) {
		return false;
	}

	// �f�[�^�j�󎞂̃��J�o��������Ă݂��肷��
	const_cast<RecentImp*>(this)->_Recovery(); 

	return true;
}

// ���J�o��
template <class T, class S>
void RecentImp<T, S>::_Recovery()
{
	if (*pnUserItemCount < 0) *pnUserItemCount = 0;
	if (*pnUserItemCount > nArrayCount) *pnUserItemCount = nArrayCount;

	if (pnUserViewCount) {
		if (*pnUserViewCount < 0) *pnUserViewCount = 0;
		if (*pnUserViewCount > nArrayCount) *pnUserViewCount = nArrayCount;
	}
}


// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                        ���C�ɓ���                           //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

/*
	���C�ɓ����Ԃ�ݒ肷��B

	true	�ݒ�
	false	����
*/
template <class T, class S>
bool RecentImp<T, S>::SetFavorite(size_t nIndex, bool bFavorite)
{
	if (! IsAvailable()) return false;
	if (nIndex >= *pnUserItemCount) return false;
	if (!pbUserItemFavorite) return false;

	pbUserItemFavorite[nIndex] = bFavorite;

	return true;
}

/*
	���ׂĂ̂��C�ɓ����Ԃ���������B
*/
template <class T, class S>
void RecentImp<T, S>::ResetAllFavorite()
{
	if (! IsAvailable()) {
		return;
	}

	for (size_t i=0; i<*pnUserItemCount; ++i) {
		SetFavorite(i, false);
	}
}

/*
	���C�ɓ����Ԃ��ǂ������ׂ�B

	true	���C�ɓ���
	false	�ʏ�
*/
template <class T, class S>
bool RecentImp<T, S>::IsFavorite(size_t nIndex) const
{
	if (! IsAvailable()) return false;
	if (nIndex >= *pnUserItemCount) return false;
	if (!pbUserItemFavorite) return false;

	return pbUserItemFavorite[nIndex];
}

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                       �A�C�e������                          //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

/*
	�A�C�e����擪�ɒǉ�����B

	@note	���łɓo�^�ς݂̏ꍇ�͐擪�Ɉړ�����B
	@note	�����ς��̂Ƃ��͍ŌẪA�C�e�����폜����B
	@note	���C�ɓ���͍폜����Ȃ��B
*/
template <class T, class S>
bool RecentImp<T, S>::AppendItem(ReceiveType pItemData)
{
	if (!IsAvailable()) return false;
	if (!pItemData) return false;

	// �o�^�ς݂����ׂ�B
	int	nIndex = FindItem(pItemData);
	if (nIndex >= 0) {
		CopyItem(GetItemPointer(nIndex), pItemData);

		// �擪�Ɏ����Ă���B
		MoveItem(nIndex, 0);
		goto reconfigure;
	}

	// �����ς��̂Ƃ��͍ŌÂ̒ʏ�A�C�e�����폜����B
	if (nArrayCount <= *pnUserItemCount) {
		nIndex = GetOldestItem(*pnUserItemCount - 1, false);
		if (nIndex == -1) {
			return false;
		}

		DeleteItem(nIndex);
	}

	for (size_t i=*pnUserItemCount; i>0; --i) {
		CopyItem(i - 1, i);
	}

	CopyItem(GetItemPointer(0), pItemData);

	//(void)SetFavorite(0, true);
	// �����������Ȃ��Ƃ��߁B
	if (pbUserItemFavorite) {
		pbUserItemFavorite[0] = false;
	}

	*pnUserItemCount += 1;


reconfigure:
	// ���C�ɓ����\�����Ɉړ�����B
	if (pnUserViewCount) {
		ChangeViewCount(*pnUserViewCount);
	}
	return true;
}


template <class T, class S>
bool RecentImp<T, S>::AppendItemText(LPCTSTR pText)
{
	DataType data;
	ReceiveType receiveData;
	if (!TextToDataType(&data, pText)) {
		return false;
	}
	if (!DataToReceiveType(&receiveData, &data)) {
		return false;
	}
	int findIndex = FindItem(receiveData);
	if (findIndex != -1) {
		return false;
	}
	return AppendItem(receiveData);
}

template <class T, class S>
bool RecentImp<T, S>::EditItemText(size_t nIndex, LPCTSTR pText)
{
	DataType data;
	ReceiveType receiveData;
	memcpy_raw(&data, GetItemPointer(nIndex), sizeof(data));
	if (!TextToDataType(&data, pText)) {
		return false;
	}
	if (!DataToReceiveType(&receiveData, &data)) {
		return false;
	}
	int findIndex = FindItem(receiveData);
	if (findIndex != -1 && nIndex != findIndex) {
		// �d���s�B�����������ꍇ�͑啶���������̕ύX��������Ȃ��̂�OK
		return false;
	}
	CopyItem(GetItemPointer(nIndex), receiveData);
	return true;
}


/*
	�A�C�e�����[���N���A����B
*/
template <class T, class S>
void RecentImp<T, S>::ZeroItem(size_t nIndex)
{
	if (! IsAvailable()) {
		return;
	}
	if (nIndex >= nArrayCount) {
		return;
	}

	memset_raw(GetItemPointer(nIndex), 0, sizeof(DataType));

	if (pbUserItemFavorite) {
		pbUserItemFavorite[nIndex] = false;
	}

	return;
}

/*
	�A�C�e�����폜����B
*/
template <class T, class S>
bool RecentImp<T, S>::DeleteItem(size_t nIndex)
{
	if (!IsAvailable()) {
		return false;
	}
	if (nIndex >= *pnUserItemCount) {
		return false;
	}

	ZeroItem(nIndex);

	// �ȍ~�̃A�C�e����O�ɋl�߂�B
	size_t i;
	for (i=nIndex; i<*pnUserItemCount-1; ++i) {
		CopyItem(i + 1, i);
	}
	ZeroItem(i);

	*pnUserItemCount -= 1;

	return true;
}

/*
	���C�ɓ���ȊO�̃A�C�e�����폜����B
*/
template <class T, class S>
bool RecentImp<T, S>::DeleteItemsNoFavorite()
{
	if (! IsAvailable()) {
		return false;
	}

	bool bDeleted = false;
	for (int i=(int)(*pnUserItemCount)-1; 0<=i; --i) {
		if (!IsFavorite(i)) {
			if (DeleteItem(i)) {
				bDeleted = true;
			}
		}
	}

	return bDeleted;
}

/*
	���ׂẴA�C�e�����폜����B

	@note	�[���N���A���\�Ƃ��邽�߁A���ׂĂ��ΏۂɂȂ�B
*/
template <class T, class S>
void RecentImp<T, S>::DeleteAllItem()
{
	if (!IsAvailable()) {
		return;
	}
	
	for (size_t i=0; i<nArrayCount; ++i) {
		ZeroItem(i);
	}
	
	*pnUserItemCount = 0;
	
	return;
}

/*
	�A�C�e�����ړ�����B
*/
template <class T, class S>
bool RecentImp<T, S>::MoveItem(size_t nSrcIndex, size_t nDstIndex)
{
	bool	bFavorite;

	if (! IsAvailable()) return false;
	if (nSrcIndex >= *pnUserItemCount) return false;
	if (nDstIndex >= *pnUserItemCount) return false;

	if (nSrcIndex == nDstIndex) return true;

	DataType pri;

	// �ړ��������ޔ�
	memcpy_raw(&pri, GetItemPointer(nSrcIndex), sizeof(pri));
	bFavorite = IsFavorite(nSrcIndex);

	if (nSrcIndex < nDstIndex) {
		for (size_t i=nSrcIndex; i<nDstIndex; ++i) {
			CopyItem(i + 1, i);
		}
	}else {
		for (size_t i=nSrcIndex; i>nDstIndex; --i) {
			CopyItem(i - 1, i);
		}
	}

	// �V�����ʒu�Ɋi�[
	memcpy_raw(GetItemPointer(nDstIndex), &pri, sizeof(pri));
	SetFavorite(nDstIndex, bFavorite);
	return true;
}

template <class T, class S>
bool RecentImp<T, S>::CopyItem(size_t nSrcIndex, size_t nDstIndex)
{
	if (!IsAvailable()) return false;
	if (nSrcIndex >= nArrayCount) return false;
	if (nDstIndex >= nArrayCount) return false;

	if (nSrcIndex == nDstIndex) return true;

	memcpy_raw(GetItemPointer(nDstIndex), GetItemPointer(nSrcIndex), sizeof(DataType));

	//(void)SetFavorite(nDstIndex, IsFavorite(nSrcIndex));
	// �����������Ȃ��Ƃ��߁B
	if (pbUserItemFavorite) pbUserItemFavorite[nDstIndex] = pbUserItemFavorite[nSrcIndex];

	return true;
}


// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                       �A�C�e���擾                          //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

template <class T, class S>
const T* RecentImp<T, S>::GetItem(size_t nIndex) const
{
	if (!IsAvailable() || nIndex >= *pnUserItemCount) return nullptr;
	return &puUserItemData[nIndex];
}

template <class T, class S>
const T* RecentImp<T, S>::GetItemPointer(size_t nIndex) const
{
	if (!IsAvailable() || nIndex >= nArrayCount) return nullptr;
	return &puUserItemData[nIndex];
}

/*
	�A�C�e������������B
*/
template <class T, class S>
int RecentImp<T, S>::FindItem(ReceiveType pItemData) const
{
	if (!IsAvailable()) return -1;
	if (!pItemData) return -1;

	for (size_t i=0; i<*pnUserItemCount; ++i) {
		if (CompareItem(GetItemPointer(i), pItemData) == 0) {
			return (int)i;
		}
	}

	return -1;
}

/*
	�A�C�e�����X�g��������Ƃ��Â��o���C�ɓ���E�ʏ�p�̃A�C�e����T���B

	bFavorite=true	���C�ɓ���̒�����T��
	bFavorite=false	�ʏ�̒�����T��
*/
template <class T, class S>
int RecentImp<T, S>::GetOldestItem(size_t nIndex, bool bFavorite)
{
	if (!IsAvailable()) return -1;
	if (nIndex >= *pnUserItemCount) nIndex = *pnUserItemCount - 1;

	for (int i=(int)nIndex; i>=0; --i) {
		if (IsFavorite(i) == bFavorite) return i;
	}

	return -1;
}


// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                          ���̑�                             //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

/*
	�Ǘ�����Ă���A�C�e���̂����̕\������ύX����B

	@note	���C�ɓ���͉\�Ȍ���\�����Ɉړ�������B
*/
template <class T, class S>
bool RecentImp<T, S>::ChangeViewCount(size_t nViewCount)
{
	// �͈͊O�Ȃ�G���[
	if (!IsAvailable()) return false;
	if (nViewCount > nArrayCount) return false;

	// �\�������X�V����B
	if (pnUserViewCount) {
		*pnUserViewCount = nViewCount;
	}

	// �͈͓��ɂ��ׂĎ��܂��Ă���̂ŉ������Ȃ��Ă悢�B
	if (nViewCount >= *pnUserItemCount) return true;

	// �ł��Â����C�ɓ����T���B
	int	i = GetOldestItem(*pnUserItemCount - 1, true);
	if (i == -1) return true;	// �Ȃ��̂ŉ������Ȃ��ŏI��

	// �\���O�A�C�e����\�����Ɉړ�����B
	for (; i>=(int)nViewCount; --i) {
		if (IsFavorite(i)) {
			// �J�����g�ʒu�����ɒʏ�A�C�e����T��
			int	nIndex = GetOldestItem(i - 1, false);
			if (nIndex == -1) break;	// ����1���Ȃ�

			// ���������A�C�e�����J�����g�ʒu�Ɉړ�����
			MoveItem(nIndex, i);
		}
	}

	return true;
}

/*
	���X�g���X�V����B
*/
template <class T, class S>
bool RecentImp<T, S>::UpdateView()
{
	// �͈͊O�Ȃ�G���[
	if (!IsAvailable()) return false;

	size_t nViewCount = pnUserViewCount ? *pnUserViewCount : nArrayCount;
	return ChangeViewCount(nViewCount);
}


// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                      �C���X�^���X��                         //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
template class RecentImp<CmdString, LPCTSTR>;
template class RecentImp<EditNode>;
template class RecentImp<EditInfo>;
template class RecentImp<PathString, LPCTSTR>;
#ifndef __MINGW32__
template class RecentImp<MetaPath, LPCTSTR>;
template class RecentImp<GrepFileString, LPCTSTR>;
template class RecentImp<GrepFolderString, LPCTSTR>;
template class RecentImp<SearchString, LPCWSTR>;
template class RecentImp<TagJumpKeywordString, LPCWSTR>;
template class RecentImp<CurDirString, LPCTSTR>;
#endif
#if !defined(__MINGW32__) || (defined(__MINGW32__) && !defined(UNICODE))
template class RecentImp<ReplaceString, LPCWSTR>;
#endif

