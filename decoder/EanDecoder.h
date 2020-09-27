#ifndef EANDECODER_H
#define EANDECODER_H
#include "AbsDecoder.h"
#include "opencv2/core/mat.hpp"
#include <map>

extern struct EncodePair;

class EanDecoder : public AbsDecoder {
public:
	EanDecoder(std::string name, uchar unitWidth);
	~EanDecoder();
	//�����ʼλ�ù̶���2ֵ���������, ��������ַ���
	std::string decode(std::vector<uchar> data, int start) const;
	std::string getName() const;
private:
	std::string name;//EAN����������EAN-13 / EAN-8
	uchar unitWidth;
	uchar bitsNum;
	uchar codeLength;
	static const uchar EAN13LENGTH = 95;
	//TODO EAN8 Length ...

	bool isValid() const;
	// ����򻯿�Ⱥ��bits
	std::string parseCode(std::vector<uchar> part) const;

	//���ر������ݺͱ��뼯
	EncodePair getContent(uchar code) const;

	bool delimiterIsValid(std::vector<uchar> data) const;

};
#endif // !EANDECODER_H

