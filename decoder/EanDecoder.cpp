#include "EanDecoder.h"
#include <iostream>
/*	Ŀǰֻ��������ͼƬʶ�����ͼƬ�Ƿ�����Ҫ��תһ��ʶ�������
	TODO��
	1. ��λ����������4���ز��Һڰ����Ŀ�Ȳ����Ȼ����ʶ�������Ҫ�Ż�������ܴﵽʶ��2���ص�λ������Ͳ�����
	2. ����У��
*/
// ���ֱ��뷽ʽ https://baike.baidu.com/item/EAN-13
static std::map<uchar, std::string> A = {
	{0x0D,"0" }, { 0x19,"1" }, {0x13, "2"}, 
	{0x3D, "3"}, {0x23,"4"}, {0x31, "5"},
	{0x2F, "6"},{0x3B, "7"}, {0x37, "8"}, {0x0B,"9"}
	};

static std::map<uchar, std::string> B = {
	{0x27,"0" }, {0x33,"1" }, {0x1B, "2"},
	{0x21, "3"}, {0x1D,"4"}, {0x39, "5"},
	{0x05, "6"}, {0x11, "7"}, {0x09, "8"}, {0x17,"9"}
};

static std::map<uchar, std::string> R = {
	{0x72,"0" }, {0x66,"1" }, {0x6C, "2"},
	{0x42, "3"}, {0x5C,"4"}, {0x4E, "5"},
	{0x50, "6"}, {0x44, "7"}, {0x48, "8"}, {0x74,"9"}
};

static std::map<std::string, std::string> prefixMap = {
	{"AAAAAA","0"}, {"AABABB","1"}, {"AABBAB","2"},
	{"AABBBA","3"}, {"ABAABB","4"}, {"ABBAAB","5"},
	{"ABBBAA","6"}, {"ABABAB","7"}, {"ABABBA","8"},
	{"ABBABA","9"}
};

struct EncodePair {
	EncodePair(std::string content, std::string type) {
		this->content = content;
		this->type = type;
	}
	std::string content;
	std::string type;
	bool valid = true;
};


EanDecoder::EanDecoder(std::string name, uchar unitWidth) {
	this->name = name;
	this->unitWidth = unitWidth;
	if (name == EAN13) {
		bitsNum = EAN13LENGTH;
		//7 module ����һ��digit
		codeLength = 7;
	}
}
EanDecoder::~EanDecoder() {}
/*
	data �����ʼλ�ù̶���2ֵ���������, ��������ַ���
	start ��һ��������ֵ��±�
	return ��������������
*/
std::string EanDecoder::decode(std::vector<uchar> data, int start) const {
	//decode EAN-13
	//����ܳ���, ����ֱ�ӷ���
	if (data.size() - start + 1 < unitWidth * EAN13LENGTH) {
		return "size wrong";
	}
	std::vector<uchar>::const_iterator barcodeItr = data.begin() + start;
	std::vector<uchar> headDelimiter;
	std::vector<uchar> leftPart;
	std::vector<uchar> middleDelimiter;
	std::vector<uchar> rightPart;
	std::vector<uchar> tailDelimiter;

	int barCnt = 0;
	uchar preBar = *barcodeItr;
	int tempWidth = 0;
	for (; barcodeItr != data.end(); barcodeItr++) {
		if (preBar == *barcodeItr) {
			tempWidth++;
		}
		else {
			int tempLen = std::round((double)(tempWidth) / unitWidth);
			if (tempLen > 4) {
				tempLen = 4;
			}
			barCnt += tempLen;
			for (int i = 0; i < tempLen; i++) {
				if (barCnt <= 3) {
					//head delimiter
					headDelimiter.push_back(preBar);
				}
				else if (barCnt > 3 && barCnt <= 45) {
					//left part
					leftPart.push_back(preBar);
				}
				else if (barCnt > 45 && barCnt <= 50) {
					//middle delimiter
					middleDelimiter.push_back(preBar);
				}
				else if (barCnt > 50 && barCnt <= 92) {
					//right part
					rightPart.push_back(preBar);
				}
				else if (barCnt > 92 && barCnt <= 95) {
					//tail delimiter
					tailDelimiter.push_back(preBar);
				}
			}
			preBar = *barcodeItr;
			tempWidth = 1;
			if (barCnt == 3 && !delimiterIsValid(headDelimiter)) {
				return "head delimiter wrong";
			}
		}
	}
	std::string result = parseCode(leftPart) + parseCode(rightPart);

	return result;
}

std::string EanDecoder::getName() const {
	return this->name;
}

bool EanDecoder::isValid() const {
	return false;
}

// ����򻯿�Ⱥ��bits
std::string EanDecoder::parseCode(std::vector<uchar> part) const {
	std::vector<uchar> array(codeLength);
	std::string content = "";
	for (int i = 0; i < part.size(); i++) {
		int digit = i / codeLength;
		array[digit] = array[digit] << 1;
		if (part[i] == BLACK) {
			array[digit] += 1;
		}
	}
	std::string types = "";
	//TODO ���result �Ƿ�����A,B,R�е�����һ��,�������������ɨ��
	for (int i = 0; i < codeLength; i++) {
		EncodePair temp = getContent(array[i]); 
		if (temp.valid == true) {
			content.append(temp.content);
			types.append(temp.type);
		}
	}
	std::map<std::string, std::string>::iterator iter = prefixMap.find(types);
	
	if (iter != prefixMap.end()) {
		content.insert(0, iter->second);
	}
	return content;
}
	
//���ر������ݺͱ��뼯
EncodePair EanDecoder::getContent(uchar code) const {
	std::map<uchar, std::string>::iterator iter;
	iter = A.find(code);
	if (iter != A.end()) {
		return EncodePair(iter->second, "A");
	}
	iter = B.find(code);
	if (iter != B.end()) {
		return EncodePair(iter->second, "B");
	}
	iter = R.find(code);
	if (iter != R.end()) {
		return EncodePair(iter->second, "R");
	}
	EncodePair wrong = EncodePair("WRONG", "WRONG");
	wrong.valid = false;
	return wrong;
}

bool EanDecoder::delimiterIsValid(std::vector<uchar> data) const {
	//���ͷ�ֽ��
	uchar delimiter[3] = { BLACK, WHITE, BLACK };
	for (int i = 0; i < 3; i++) {
		if (data[i] != delimiter[i]) {
			return false;
		}
	}
	return true;
}
	
	