#include "StdAfx.h"
#include "AutoSaveAgent.h"
#include "doc/EditDoc.h"
#include "env/DllSharedData.h"


//	From Here Aug. 21, 2000 genta
//
//	�����ۑ����s�����ǂ����̃`�F�b�N
//
void AutoSaveAgent::CheckAutoSave()
{
	if (passiveTimer.CheckAction()) {
		EditDoc* pDoc = GetListeningDoc();

		//	�㏑���ۑ�

		if (!pDoc->docEditor.IsModified()) {	//	�ύX�����Ȃ牽�����Ȃ�
			return;				//	�����ł́C�u���ύX�ł��ۑ��v�͖�������
		}

		//	2003.10.09 zenryaku �ۑ����s�G���[�̗}��
		if (!pDoc->docFile.GetFilePathClass().IsValidPath()) {	//	�܂��t�@�C�������ݒ肳��Ă��Ȃ���Εۑ����Ȃ�
			return;
		}

		bool en = passiveTimer.IsEnabled();
		passiveTimer.Enable(false);	//	2�d�Ăяo����h������
		pDoc->docFileOperation.FileSave();	//	�ۑ�
		passiveTimer.Enable(en);
	}
}

//
//	�ݒ�ύX�������ۑ�����ɔ��f����
//
void AutoSaveAgent::ReloadAutoSaveParam()
{
	auto& csBackup = GetDllShareData().common.backup;
	passiveTimer.SetInterval(csBackup.GetAutoBackupInterval());
	passiveTimer.Enable(csBackup.IsAutoBackupEnabled());
}

//----------------------------------------------------------
//	class PassiveTimer
//
//----------------------------------------------------------
/*!
	���ԊԊu�̐ݒ�
	@param m �Ԋu(min)
	�Ԋu��0�ȉ��ɐݒ肵���Ƃ���1�b�Ƃ݂Ȃ��B�ݒ�\�ȍő�Ԋu��35792���B
*/
void PassiveTimer::SetInterval(int m)
{
	if (m <= 0) {
		m = 1;
	}else if (m >= 35792) {	//	35792���ȏゾ�� int �ŕ\���ł��Ȃ��Ȃ�
		m = 35792;
	}
	nInterval = m * MSec2Min;
}

/*!
	�^�C�}�[�̗L���E�����̐؂�ւ�
	@param flag true:�L�� / false: ����
	�������L���ɐ؂�ւ����Ƃ��̓��Z�b�g�����B
*/
void PassiveTimer::Enable(bool flag)
{
	if (bEnabled != flag) {	//	�ύX������Ƃ�
		bEnabled = flag;
		if (flag) {	//	enabled
			Reset();
		}
	}
}

/*!
	�O���Œ���Ɏ��s�����Ƃ��납��Ăяo�����֐��B
	�Ăяo�����ƌo�ߎ��Ԃ��`�F�b�N����B

	@retval true ���莞�Ԃ��o�߂����B���̂Ƃ��͑����������I�Ƀ��Z�b�g�����B
	@retval false ����̎��ԂɒB���Ă��Ȃ��B
*/
bool PassiveTimer::CheckAction(void)
{
	if (!IsEnabled()) {	//	�L���łȂ���Ή������Ȃ�
		return false;
	}

	//	������r
	DWORD now = ::GetTickCount();
	int diff;

	diff = now - nLastTick;	//	TickCount�����肵�Ă�����ł��܂������͂�...

	if (diff < nInterval) {	//	�K�莞�ԂɒB���Ă��Ȃ�
		return false;
	}

	Reset();
	return true;
}

