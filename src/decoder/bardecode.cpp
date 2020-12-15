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
#include "decoder/bardecode.hpp"

namespace cv {

//TODO 读取
void cutImage(InputArray _src, OutputArray &_dst, const std::vector<Point2f> &rects)
{
    std::vector<Point2f> vertices = rects;
    int height = cv::norm(vertices[0] - vertices[1]);
    int width = cv::norm(vertices[1] - vertices[2]);
    if (height > width)
    {
        std::swap(height, width);
        Point2f v0 = vertices[0];
        vertices.erase(vertices.begin());
        vertices.push_back(v0);
    }
    std::vector<Point2f> dst_vertices{
            Point2f(0, height - 1), Point2f(0, 0), Point2f(width - 1, 0), Point2f(width - 1, height - 1)};
    _dst.create(Size(width, height), CV_8UC1);
    Mat M = getPerspectiveTransform(vertices, dst_vertices);
    Mat dst = _dst.getMat();
    warpPerspective(_src.getMat(), dst, M, _dst.size(), cv::INTER_LINEAR, BORDER_CONSTANT, Scalar(255));
}
}
