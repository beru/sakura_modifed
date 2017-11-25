#pragma once

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                 ���b�Z�[�W�{�b�N�X�F����                    //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

// �e�L�X�g���`�@�\�t��MessageBox
int VMessageBoxF(HWND hwndOwner, UINT uType, LPCTSTR lpCaption, LPCTSTR lpText, va_list& v);
int MessageBoxF (HWND hwndOwner, UINT uType, LPCTSTR lpCaption, LPCTSTR lpText, ...);


//                ���[�U�p���b�Z�[�W�{�b�N�X                   //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

// �f�o�b�O�p���b�Z�[�W�{�b�N�X
#define MYMESSAGEBOX MessageBoxF

// ��ʂ̌x����
#define DefaultBeep()   ::MessageBeep(MB_OK)

// �G���[�F�ԊۂɁu�~�v[OK]
int ErrorMessage   (HWND hwnd, LPCTSTR format, ...);
int TopErrorMessage(HWND hwnd, LPCTSTR format, ...);	//(TOPMOST)
#define ErrorBeep()     ::MessageBeep(MB_ICONSTOP)

// �x���F�O�p�Ɂu�I�v[OK]
int WarningMessage   (HWND hwnd, LPCTSTR format, ...);
int TopWarningMessage(HWND hwnd, LPCTSTR format, ...);
#define WarningBeep()   ::MessageBeep(MB_ICONEXCLAMATION)

// ���F�ۂɁui�v[OK]
int InfoMessage   (HWND hwnd, LPCTSTR format, ...);
int TopInfoMessage(HWND hwnd, LPCTSTR format, ...);
#define InfoBeep()      ::MessageBeep(MB_ICONINFORMATION)

// �m�F�F�����o���́u�H�v [�͂�][������] �߂�l:IDYES,IDNO
int ConfirmMessage   (HWND hwnd, LPCTSTR format, ...);
int TopConfirmMessage(HWND hwnd, LPCTSTR format, ...);
#define ConfirmBeep()   ::MessageBeep(MB_ICONQUESTION)

// �O���F�����o���́u�H�v [�͂�][������][�L�����Z��]  �߂�l:ID_YES,ID_NO,ID_CANCEL
int Select3Message   (HWND hwnd, LPCTSTR format, ...);
int TopSelect3Message(HWND hwnd, LPCTSTR format, ...);

// ���̑����b�Z�[�W�\���p�{�b�N�X[OK]
int OkMessage   (HWND hwnd, LPCTSTR format, ...);
int TopOkMessage(HWND hwnd, LPCTSTR format, ...);

// �^�C�v�w�胁�b�Z�[�W�\���p�{�b�N�X
int CustomMessage   (HWND hwnd, UINT uType, LPCTSTR format, ...);
int TopCustomMessage(HWND hwnd, UINT uType, LPCTSTR format, ...);	//(TOPMOST)

// ��҂ɋ����ė~�����G���[
int PleaseReportToAuthor(HWND hwnd, LPCTSTR format, ...);

