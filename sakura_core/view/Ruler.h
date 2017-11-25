#pragma once

class TextArea;
class EditView;
class EditDoc;
class TextMetrics;
class Graphics;

class Ruler {
public:
	Ruler(const EditView& editView, const EditDoc& editDoc);
	virtual ~Ruler();
	
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//                     �C���^�[�t�F�[�X                        //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	
	// ���[���[�`�� (�w�i�ƃL�����b�g)
	void DispRuler(HDC);
	
	// ���[���[�̔w�i�̂ݕ`��
	void DrawRulerBg(Graphics& gr);
	
	void SetRedrawFlag() { bRedrawRuler = true; }
	bool GetRedrawFlag() { return bRedrawRuler; }
	
private:
	// ���[���[�̃L�����b�g�̂ݕ`�� 2002.02.25 Add By KK
	void DrawRulerCaret(Graphics& gr);
	
	void _DrawRulerCaret(Graphics& gr, int nCaretDrawX, int nCaretWidth);
	
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//                       �����o�ϐ��Q                          //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
private:
	// �Q��
	const EditView&	editView;
	const EditDoc&	editDoc;
	
	// ���
	bool	bRedrawRuler;		// ���[���[�S�̂�`�������� = true
	int		nOldRulerDrawX;	// �O��`�悵�����[���[�̃L�����b�g�ʒu
	int		nOldRulerWidth;	// �O��`�悵�����[���[�̃L�����b�g��
};

