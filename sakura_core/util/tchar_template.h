/*
	_T�}�N���݊��̃e���v���[�g�B
	�r���h��Ɋւ�炸�A�w�肵���^�̕����萔��񋟂���B

	_T2(char,'A')
	_T2(wchar_t,'x')
	_T2(TCHAR,'/')
	�̂悤�Ɏg���܂��B

	�e���v���[�g�Ă񂱂���Ȃ̂ŁA�R���p�C�����d���Ȃ�Ǝv���܂��B
	�C���N���[�h�͍ŏ����ɁI

	2007.10.23 kobake �쐬
*/

template <class CHAR_TYPE, int CHAR_VALUE>
CHAR_TYPE _TextTemplate();

// ������`�}�N��
#define DEFINE_T2(CHAR_VALUE) \
template <> char _TextTemplate<char,CHAR_VALUE>() { return ATEXT(CHAR_VALUE); } \
template <> wchar_t _TextTemplate<wchar_t,CHAR_VALUE>() { return LTEXT(CHAR_VALUE); }

// �g�p�}�N��
#define _T2(CHAR_TYPE, CHAR_VALUE) _TextTemplate<CHAR_TYPE,CHAR_VALUE>()

