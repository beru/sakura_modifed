#pragma once

class CodeBase;

class CodeFactory {
public:
	// eCodeType�ɓK������ CodeBase�C���X�^���X �𐶐�
	static CodeBase* CreateCodeBase(
		EncodingType	eCodeType,		// �����R�[�h
		int			nFlag			// bit 0: MIME Encode���ꂽ�w�b�_��decode���邩�ǂ���
	);
};

