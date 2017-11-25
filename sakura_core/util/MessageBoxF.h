#pragma once

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                 メッセージボックス：実装                    //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

// テキスト整形機能付きMessageBox
int VMessageBoxF(HWND hwndOwner, UINT uType, LPCTSTR lpCaption, LPCTSTR lpText, va_list& v);
int MessageBoxF (HWND hwndOwner, UINT uType, LPCTSTR lpCaption, LPCTSTR lpText, ...);


//                ユーザ用メッセージボックス                   //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

// デバッグ用メッセージボックス
#define MYMESSAGEBOX MessageBoxF

// 一般の警告音
#define DefaultBeep()   ::MessageBeep(MB_OK)

// エラー：赤丸に「×」[OK]
int ErrorMessage   (HWND hwnd, LPCTSTR format, ...);
int TopErrorMessage(HWND hwnd, LPCTSTR format, ...);	//(TOPMOST)
#define ErrorBeep()     ::MessageBeep(MB_ICONSTOP)

// 警告：三角に「！」[OK]
int WarningMessage   (HWND hwnd, LPCTSTR format, ...);
int TopWarningMessage(HWND hwnd, LPCTSTR format, ...);
#define WarningBeep()   ::MessageBeep(MB_ICONEXCLAMATION)

// 情報：青丸に「i」[OK]
int InfoMessage   (HWND hwnd, LPCTSTR format, ...);
int TopInfoMessage(HWND hwnd, LPCTSTR format, ...);
#define InfoBeep()      ::MessageBeep(MB_ICONINFORMATION)

// 確認：吹き出しの「？」 [はい][いいえ] 戻り値:IDYES,IDNO
int ConfirmMessage   (HWND hwnd, LPCTSTR format, ...);
int TopConfirmMessage(HWND hwnd, LPCTSTR format, ...);
#define ConfirmBeep()   ::MessageBeep(MB_ICONQUESTION)

// 三択：吹き出しの「？」 [はい][いいえ][キャンセル]  戻り値:ID_YES,ID_NO,ID_CANCEL
int Select3Message   (HWND hwnd, LPCTSTR format, ...);
int TopSelect3Message(HWND hwnd, LPCTSTR format, ...);

// その他メッセージ表示用ボックス[OK]
int OkMessage   (HWND hwnd, LPCTSTR format, ...);
int TopOkMessage(HWND hwnd, LPCTSTR format, ...);

// タイプ指定メッセージ表示用ボックス
int CustomMessage   (HWND hwnd, UINT uType, LPCTSTR format, ...);
int TopCustomMessage(HWND hwnd, UINT uType, LPCTSTR format, ...);	//(TOPMOST)

// 作者に教えて欲しいエラー
int PleaseReportToAuthor(HWND hwnd, LPCTSTR format, ...);

