#pragma once

#include "doc/DocListener.h"

class DocLocker : public DocListenerEx {
public:
	DocLocker();

	// �N���A
	void Clear(void) { bIsDocWritable = true; }

	// ���[�h�O��
	void OnAfterLoad(const LoadInfo& loadInfo);
	
	// �Z�[�u�O��
	void OnBeforeSave(const SaveInfo& saveInfo);
	void OnAfterSave(const SaveInfo& saveInfo);

	// ���
	bool IsDocWritable() const { return bIsDocWritable; }

	// �`�F�b�N
	void CheckWritable(bool bMsg);

private:
	bool bIsDocWritable;
};

