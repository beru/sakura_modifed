/*!	@file
	@brief �����񋤒ʒ�`
*/

#include "StdAfx.h"
#include "global.h"
#include "window/EditWnd.h"
#include "NormalProcess.h"

// EditWnd�̃C���X�^���X�ւ̃|�C���^�������ɕۑ����Ă���
EditWnd* g_pcEditWnd = nullptr;


// �I��̈�`��p�p�����[�^
const COLORREF	SELECTEDAREA_RGB = RGB(255, 255, 255);
const int		SELECTEDAREA_ROP2 = R2_XORPEN;


HINSTANCE G_AppInstance()
{
	return Process::getInstance()->GetProcessInstance();
}

