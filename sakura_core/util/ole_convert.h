/*!	@file
	@brief OLE�^�iVARIANT, BSTR�Ȃǁj�̕ϊ��֐�

*/
#pragma once

#include <string>
#include "_os/OleTypes.h"

bool variant_to_wstr( VARIANT v, std::wstring& wstr );	// VARIANT�ϐ���BSTR�Ƃ݂Ȃ��Awstring�ɕϊ�����
bool variant_to_int( VARIANT v, int& n );	// VARIANT�ϐ��𐮐��Ƃ݂Ȃ��Aint�ɕϊ�����

