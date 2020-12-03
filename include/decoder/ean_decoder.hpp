/*
Copyright 2020 ${ALL COMMITTERS}

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
*/

#ifndef __OPENCV_BARCODE_EAN_DECODER_HPP__
#define __OPENCV_BARCODE_EAN_DECODER_HPP__

#include "absdecoder.hpp"
#include "opencv2/core/mat.hpp"

namespace cv {
//extern struct EncodePair;
using std::string;
using std::vector;
// TODO, EAN-8
enum class EAN
{
    TYPE13 = 0, TYPE8
};
class ean_decoder : public absdecoder
{
public:
    explicit ean_decoder(EAN name);

    ~ean_decoder() override = default;

    //输入初始位置固定的2值化后的数据, 输出解码字符串
    string decode(vector<uchar> data, int start) const override;

    //Detect encode type
    string decodeDirectly(InputArray img) const override;

    string getName() const override;

    string decodeOuter(vector<uchar> data);

    vector<string> rectToResults(Mat &mat, const std::vector<RotatedRect> &rects) const;

private:
    string name; //EAN具体解码类别：EAN-13 / EAN-8
    static constexpr uchar EAN13LENGTH = 95;
    static constexpr uchar EAN13DIGITNUMBER = 13;
    //TODO EAN8 Length ...
    uchar bitsNum;
    uchar digitNumber;

    bool isValid(string result) const override;

    static int
    decodeDigit(const vector<uchar> &row, vector<int> &counters, int rowOffset, vector<vector<int>> patterns);

    static std::pair<int, int>
    findGuardPatterns(const vector<uchar> &row, int rowOffset, uchar whiteFirst, const vector<int> &pattern,
                      vector<int> counters);

    string rectToResult(const Mat &gray, Mat &mat, const RotatedRect &rect, int PART, int directly) const;

    string lineDecodeToString(const Mat &bar_img, const Point2i &begin, const Point2i &end) const;

    void linesFromRect(const Size2i &shape, int angle, int PART, vector<std::pair<Point2f, Point2f>> &results) const;
};

Mat grayNomalization(Mat mat, double M0, double VAR0);

vector<Mat> getBarcodeImgs(Mat gray_img, const vector<RotatedRect> &rects);
} // namespace cv
#endif // !__OPENCV_BARCODE_EAN_DECODER_HPP__
