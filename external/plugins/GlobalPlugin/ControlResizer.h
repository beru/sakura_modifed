#pragma once

// �R���g���[���̐e�E�B���h�E�̃T�C�Y�ύX���ɃR���g���[���̃T�C�Y��ʒu��ύX���鏈�����s��
// �����_��Anchor�̂ݑΉ�

#include <vector>

enum AnchorStyle {
	None	= 0,
	Top		= 1 << 0,
	Bottom	= 1 << 1,
	Left	= 1 << 2,
	Right	= 1 << 3,
};

class ControlResizer
{
public:
	ControlResizer() {}
	~ControlResizer() {}

	void Init(HWND hWnd);
	void Add(int id, int anchorStyle);
	void DefereWindowPos(int width, int height);
	
private:
	struct ControlSetting
	{
		int id;
		int anchorStyle;
		RECT initialRect;
	};
	
	HWND hWnd_;
	int initialWidth_;
	int initialHeight_;
	std::vector<ControlSetting> settings_;
};

