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
//
// Created by 97659 on 2020/11/3.
//

#include "precomp.hpp"
#include "barcode.hpp"

namespace cv {
namespace barcode {
static bool checkBarInputImage(InputArray img, Mat &gray)
{
    CV_Assert(!img.empty());
    CV_CheckDepthEQ(img.depth(), CV_8U, "");
    if (img.cols() <= 20 || img.rows() <= 20)
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
    vector<vector<Point2f> > pnts2f = bardet.getTransformationPoints();
    vector<Point2f> trans_points;
    for (auto &i : pnts2f)
        for (const auto &j : i)
            trans_points.push_back(j);

    updatePointsResult(points, trans_points);
    return true;
}

bool BarcodeDetector::decode(InputArray img, InputArray points, CV_OUT std::vector<std::string> &decoded_info) const
{
    Mat inarr;
    if (!checkBarInputImage(img, inarr))
    {
        return false;
    }
    CV_Assert(points.size().width > 0);
    CV_Assert((points.size().width % 4) == 0);
    vector<vector<Point2f> > src_points;
    Mat bar_points = points.getMat();
    bar_points = bar_points.reshape(2, 1);
    for (int i = 0; i < bar_points.size().width; i += 4)
    {
        vector<Point2f> tempMat = bar_points.colRange(i, i + 4);
        if (contourArea(tempMat) > 0.0)
        {
            src_points.push_back(tempMat);
        }
    }
    CV_Assert(!src_points.empty());
    ean13_decoder decoder;
    vector<std::string> _decoded_info = decoder.rectToResults(inarr, src_points);
    decoded_info.clear();
    decoded_info.assign(_decoded_info.begin(), _decoded_info.end());
    return !decoded_info.empty();
}

bool BarcodeDetector::detectAndDecode(InputArray img, CV_OUT std::vector<std::string> &decoded_info,
                                      OutputArray points_) const
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
    ok = this->decode(inarr, points, decoded_info);
    return ok;
}

bool BarcodeDetector::decodeDirectly(InputArray img, CV_OUT string &decoded_info) const
{
    Mat inarr;
    if (!checkBarInputImage(img, inarr))
    {
        return false;
    }
    ean13_decoder ean13;
    decoded_info = ean13.decodeDirectly(inarr);
    if (!decoded_info.empty())
    {
        return false;
    }
    return true;
}
}// namespace barcode
} // namespace cv
