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


#include "precomp.hpp"

namespace cv {
namespace barcode {
std::ostream &operator<<(std::ostream &out, BarcodeType format)
{
    switch (format)
    {
        case BarcodeType::EAN_8:
            out << "EAN_8";
            break;
        case BarcodeType::EAN_13:
            out << "EAN_13";
            break;
        case BarcodeType::UPC_E:
            out << "UPC_E";
            break;
        case BarcodeType::UPC_A:
            out << "UPC_A";
            break;
        case BarcodeType::UPC_EAN_EXTENSION:
            out << "UPC_EAN_EXTENSION";
            break;
        default:
            out << "NONE";
    }
    return out;
}

static bool checkBarInputImage(InputArray img, Mat &gray)
{
    CV_Assert(!img.empty());
    CV_CheckDepthEQ(img.depth(), CV_8U, "");
    if (img.cols() <= 40 || img.rows() <= 40)
    {
        return false; // image data is not enough for providing reliable results
    }
    int incn = img.channels();
    CV_Check(incn, incn == 1 || incn == 3 || incn == 4, "");
    if (incn == 3 || incn == 4)
    {
        cvtColor(img, gray, COLOR_BGR2GRAY);
    }
    else
    {
        gray = img.getMat();
    }
    return true;
}

static void updatePointsResult(OutputArray points_, const vector<Point2f> &points)
{
    if (points_.needed())
    {
        int N = int(points.size() / 4);
        if (N > 0)
        {
            Mat m_p(N, 4, CV_32FC2, (void *) &points[0]);
            int points_type = points_.fixedType() ? points_.type() : CV_32FC2;
            m_p.reshape(2, points_.rows()).convertTo(points_, points_type);  // Mat layout: N x 4 x 2cn
        }
        else
        {
            points_.release();
        }
    }
}

struct BarcodeDetector::Impl
{
public:
    Impl() = default;

    ~Impl() = default;
};

BarcodeDetector::BarcodeDetector() : p(new Impl)
{
}

BarcodeDetector::~BarcodeDetector() = default;

bool BarcodeDetector::detect(InputArray img, OutputArray points) const
{
    Mat inarr;
    if (!checkBarInputImage(img, inarr))
    {
        points.release();
        return false;
    }

    Detect bardet;
    bardet.init(inarr);
    bardet.localization();
    if (!bardet.computeTransformationPoints())
    { return false; }
    vector<vector<Point2f>> pnts2f = bardet.getTransformationPoints();
    vector<Point2f> trans_points;
    for (auto &i : pnts2f)
    {
        for (const auto &j : i)
        {
            trans_points.push_back(j);
        }
    }

    updatePointsResult(points, trans_points);
    return true;
}

bool BarcodeDetector::decode(InputArray img, InputArray points, vector<std::string> &decoded_info,
                             vector<BarcodeType> &decoded_type) const
{
    Mat inarr;
    if (!checkBarInputImage(img, inarr))
    {
        return false;
    }
    CV_Assert(points.size().width > 0);
    CV_Assert((points.size().width % 4) == 0);
    vector<Point2f> src_points;
    points.copyTo(src_points);
    BarDecode bardec;
    bardec.init(img.getMat(), src_points);
    bool ok = bardec.decodeMultiplyProcess();
    const vector<Result> &_decoded_info = bardec.getDecodeInformation();
    decoded_info.clear();
    decoded_type.clear();
    for (const auto &info : _decoded_info)
    {
        decoded_info.emplace_back(info.result);
        decoded_type.emplace_back(info.format);
    }
    return ok;
}

bool BarcodeDetector::detectAndDecode(InputArray img, vector<std::string> &decoded_info,
                                      vector<BarcodeType> &decoded_format, OutputArray points_) const
{
    Mat inarr;
    if (!checkBarInputImage(img, inarr))
    {
        points_.release();
        return false;
    }
    vector<Point2f> points;
    bool ok = this->detect(img, points);
    if (!ok)
    {
        points_.release();
        return false;
    }
    updatePointsResult(points_, points);
    decoded_info.clear();
    decoded_format.clear();
    ok = this->decode(inarr, points, decoded_info, decoded_format);
    return ok;
}

bool BarcodeDetector::decodeDirectly(InputArray img, std::string &decoded_info, BarcodeType &decoded_format) const
{
    Mat inarr;
    if (!checkBarInputImage(img, inarr))
    {
        return false;
    }
    Result _decoded_info;
    std::unique_ptr<AbsDecoder> decoder{new Ean13Decoder()};
    _decoded_info = decoder->decodeImg(inarr);
    decoded_info = _decoded_info.result;
    decoded_format = _decoded_info.format;
    if (decoded_format == BarcodeType::NONE || decoded_info.empty())
    {
        return false;
    }
    return true;
}
}// namespace barcode
} // namespace cv
