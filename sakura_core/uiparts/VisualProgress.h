#pragma once

#include "doc/DocListener.h"
#include "util/design_template.h"
class WaitCursor;

class VisualProgress :
	public DocListenerEx,
	public ProgressListener
{
public:
	// �R���X�g���N�^�E�f�X�g���N�^
	VisualProgress();
	virtual ~VisualProgress();

	// ���[�h�O��
	void OnBeforeLoad(LoadInfo* pLoadInfo);
	void OnAfterLoad(const LoadInfo& loadInfo);

	// �Z�[�u�O��
	void OnBeforeSave(const SaveInfo& saveInfo);
	void OnFinalSave(SaveResultType eSaveResult);

	// �v���O���X��M
	void OnProgress(size_t nPer);

protected:
	// �����⏕
	void _Begin();
	void _Doing(int nPer);
	void _End();
private:
	WaitCursor* pWaitCursor;
	int	nOldValue;

private:
	DISALLOW_COPY_AND_ASSIGN(VisualProgress);
};

