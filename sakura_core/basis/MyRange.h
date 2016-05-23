#pragma once

class Range {
public:
	// �R���X�g���N�^
	Range() {
	}
	Range(const Range& rhs) {
		operator = (rhs);
	}
	Range(const Point& ptFrom, const Point& ptTo)
		:
		ptFrom(ptFrom),
		ptTo(ptTo)
	{
	}

	// ���
	Range& operator = (const Range& rhs) {
		ptFrom = rhs.ptFrom;
		ptTo = rhs.ptTo;
		return *this;
	}

	// ��r
	bool operator == (const Range& rhs) const {
		return ptFrom == rhs.ptFrom && ptTo == rhs.ptTo;
	}
	bool operator != (const Range& rhs) const {
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
	Point GetFrom() const {
		return ptFrom;
	}
	Point GetTo() const {
		return ptTo;
	}

	// ����
	Point* GetFromPointer() {
		return &ptFrom;
	}
	Point* GetToPointer() {
		return &ptTo;
	}

	// �ݒ�
	void Clear(int n) {
		ptFrom.Set(n, n);
		ptTo.Set(n, n);
	}
	void Set(const Point& pt) {
		ptFrom = pt;
		ptTo = pt;
	}
	void SetFrom(const Point& pt) {
		ptFrom = pt;
	}
	void SetTo(const Point& pt) {
		ptTo = pt;
	}

	void SetFromX(int nX) {
		ptFrom.x = nX;
	}
	/*
	void SetFromY(int nY) {
		ptFrom.y = nY;
	}
	*/
	void SetFromY(int nY) {
		ptFrom.y = nY;
	}
	
	void SetToX(int nX) {
		ptTo.x = nX;
	}
	/*
	void SetToY(int nY) {
		ptTo.y = nY;
	}
	*/
	void SetToY(int nY) {
		ptTo.y = nY;
	}

	// ����ݒ�
	void SetLine(int nY)					{ ptFrom.y = nY;     ptTo.y = nY;   }
	void SetXs(int nXFrom, int nXTo)	{ ptFrom.x = nXFrom; ptTo.x = nXTo; }
private:
	Point ptFrom;
	Point ptTo;
};



