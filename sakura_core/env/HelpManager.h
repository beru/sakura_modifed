#pragma once

// �v��s��`
// #include "DllSharedData.h"


// �w���v�Ǘ�
class HelpManager {
public:
	HelpManager() {
		pShareData = &GetDllShareData();
	}
	// �w���v�֘A	//@@@ 2002.2.3 YAZAKI
	bool			ExtWinHelpIsSet(const TypeConfig* pType = nullptr);		// �^�C�v��nType�̂Ƃ��ɁA�O���w���v���ݒ肳��Ă��邩�B
	const TCHAR*	GetExtWinHelp(const TypeConfig* pType = nullptr);		// �^�C�v��nType�̂Ƃ��́A�O���w���v�t�@�C�������擾�B
	bool			ExtHTMLHelpIsSet(const TypeConfig* pType = nullptr);	// �^�C�v��nType�̂Ƃ��ɁA�O��HTML�w���v���ݒ肳��Ă��邩�B
	const TCHAR*	GetExtHTMLHelp(const TypeConfig* pType = nullptr);		// �^�C�v��nType�̂Ƃ��́A�O��HTML�w���v�t�@�C�������擾�B
	bool			HTMLHelpIsSingle(const TypeConfig* pType = nullptr);	// �^�C�v��nType�̂Ƃ��́A�O��HTML�w���v�u�r���[�A�𕡐��N�����Ȃ��v��ON�����擾�B
private:
	DllSharedData* pShareData;
};

