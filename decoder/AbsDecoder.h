#ifndef ABSDECODER_H
#define ABSDECODER_H
#include <string>
#include "opencv2/core/mat.hpp"
#include <vector>
/*
	AbsDecoder �Ǹ���ʶ��ʽ�ĳ����࣬EAN-13/8 Code128 �ȵȶ�ʵ����
	��Bardecoder �ж�̬ѡ��ʹ�����ֽ���Decoder
*/

void test();

class AbsDecoder {
public:
	//input 1 row 2-value Mat, return decode string
	virtual std::string decode(std::vector<uchar> bar, int start) const=0;
	virtual std::string getName() const=0;
private:
	virtual bool isValid() const=0;
};

const std::string EAN13 = "EAN-13";
const std::string EAN8 = "EAN-8";
const uchar BLACK = 0;
const uchar WHITE = 255;

#endif
