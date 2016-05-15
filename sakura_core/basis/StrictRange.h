/*
	Copyright (C) 2008, kobake

	This software is provided 'as-is', without any express or implied
	warranty. In no event will the authors be held liable for any damages
	arising from the use of this software.

	Permission is granted to anyone to use this software for any purpose,
	including commercial applications, and to alter it and redistribute it
	freely, subject to the following restrictions:

		1. The origin of this software must not be misrepresented;
		   you must not claim that you wrote the original software.
		   If you use this software in a product, an acknowledgment
		   in the product documentation would be appreciated but is
		   not required.

		2. Altered source versions must be plainly marked as such,
		   and must not be misrepresented as being the original software.

		3. This notice may not be removed or altered from any source
		   distribution.
*/
#pragma once

template <class PointType>
class RangeBase {
public:
	typedef typename PointType::IntType IntType;
public:
	// �R���X�g���N�^
	RangeBase() {
	}
	RangeBase(const RangeBase& rhs) {
		operator = (rhs);
	}
	RangeBase(const PointType& _ptFrom, const PointType& _ptTo) {
		ptFrom = _ptFrom;
		ptTo = _ptTo;
	}

	// ���
	RangeBase& operator = (const RangeBase& rhs) {
		ptFrom = rhs.ptFrom;
		ptTo = rhs.ptTo;
		return *this;
	}

	// ��r
	bool operator == (const RangeBase& rhs) const {
		return ptFrom == rhs.ptFrom && ptTo == rhs.ptTo;
	}
	bool operator != (const RangeBase& rhs) const {
		return !(operator == (rhs));
	}

	// ����
	// 1���������I�����ĂȂ���ԂȂ�true
	bool IsOne() const {
		return ptFrom == ptTo;
	}
	bool IsLineOne() const {
		return ptFrom.y == ptTo.y;
	}
	bool IsValid() const { // �L���Ȕ͈͂Ȃ�true
		return ptFrom.BothNatural() && ptTo.BothNatural();
	}

	// �擾
	PointType GetFrom() const {
		return ptFrom;
	}
	PointType GetTo() const {
		return ptTo;
	}

	// ����
	PointType* GetFromPointer() {
		return &ptFrom;
	}
	PointType* GetToPointer() {
		return &ptTo;
	}

	// �ݒ�
	void Clear(int n) {
		ptFrom.Set(IntType(n), IntType(n));
		ptTo.Set(IntType(n), IntType(n));
	}
	void Set(const PointType& pt) {
		ptFrom = pt;
		ptTo = pt;
	}
	void SetFrom(const PointType& pt) {
		ptFrom = pt;
	}
	void SetTo(const PointType& pt) {
		ptTo = pt;
	}

	void SetFromX(IntType nX) {
		ptFrom.x = nX;
	}
	/*
	void SetFromY(int nY) {
		ptFrom.y = nY;
	}
	*/
	void SetFromY(IntType nY) {
		ptFrom.y = nY;
	}
	
	void SetToX(IntType nX) {
		ptTo.x = nX;
	}
	/*
	void SetToY(int nY) {
		ptTo.y = nY;
	}
	*/
	void SetToY(IntType nY) {
		ptTo.y = nY;
	}

	// ����ݒ�
	void SetLine(IntType nY)					{ ptFrom.y = nY;     ptTo.y = nY;   }
	void SetXs(IntType nXFrom, IntType nXTo)	{ ptFrom.x = nXFrom; ptTo.x = nXTo; }
private:
	PointType ptFrom;
	PointType ptTo;
};

