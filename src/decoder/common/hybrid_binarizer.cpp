// This file is part of OpenCV project.
// It is subject to the license terms in the LICENSE file found in the top-level directory
// of this distribution and at http://opencv.org/license.html.
// Modified from ZXing. Copyright ZXing authors.
// Licensed under the Apache License, Version 2.0 (the "License").
#include "hybrid_binarizer.hpp"
#include <vector>

namespace cv{
namespace barcode{

int cap(int value, int min, int max)
{
    if (value < min) {
        return min;
    } else {
        return value > max ? max : value;
    }
}

void thresholdBlock(std::vector<uchar> luminances, int xoffset, int yoffset, int threshold, int stride, Mat& dst) {
    int y = 0;

    for(int offset = yoffset * stride + xoffset; y < 8; offset += stride) {
        for(int x = 0; x < 8; ++x) {
            if ((luminances[offset + x] & 255) <= threshold) {
                dst.at<uchar>(yoffset + y, xoffset + x) = 0;
            }else {
                dst.at<uchar>(yoffset + y, xoffset + x) = 255;
            }
        }
        ++y;
    }

}

void calculateThresholdForBlock(std::vector<uchar> luminances, int sub_width, int sub_height, int width, int height,Mat black_points, Mat& dst)
{
    int maxYOffset = height - BLOCK_SIZE;
    int maxXOffset = width - BLOCK_SIZE;
    for (int y = 0; y < sub_height; y++) {
        int yoffset = y << BLOCK_SIZE_POWER;
        if (yoffset > maxYOffset) {
            yoffset = maxYOffset;
        }
        int top = cap(y, 2, sub_height - 3);
        for (int x = 0; x < sub_width; x++) {
            int xoffset = x << BLOCK_SIZE_POWER;
            if (xoffset > maxXOffset) {
                xoffset = maxXOffset;
            }
            int left = cap(x, 2, sub_width - 3);
            int sum = 0;
            for (int z = -2; z <= 2; z++) {
                const uchar* black_row = black_points.ptr<uchar>(top + z);
                sum += black_row[left - 2] + black_row[left - 1] + black_row[left] + black_row[left + 1] + black_row[left + 2];
            }
            int average = sum / 25;
            thresholdBlock(luminances, xoffset, yoffset, average, width, dst);
        }
    }

}

Mat calculateBlackPoints(std::vector<uchar> luminances, int sub_width, int sub_height, int width, int height)
{
    int maxYOffset = height - BLOCK_SIZE;
    int maxXOffset = width - BLOCK_SIZE;
    Mat black_points(Size(sub_width, sub_height), CV_8UC1);
    for (int y = 0; y < sub_height ;y++) {
        int yoffset = y << BLOCK_SIZE_POWER;
        if (yoffset > maxYOffset) {
            yoffset = maxYOffset;
        }
        for (int x = 0; x < sub_width; x++) {
            int xoffset = x << BLOCK_SIZE_POWER;
            if (xoffset > maxXOffset) {
                xoffset = maxXOffset;
            }
            int sum = 0;
            int min = 0xFF;
            int max = 0;
            for (int yy = 0, offset = yoffset * width + xoffset; yy < BLOCK_SIZE; yy++, offset += width) {
                for (int xx = 0; xx < BLOCK_SIZE; xx++) {
                    int pixel = luminances[offset + xx] & 0xFF;
                    sum += pixel;
                    // still looking for good contrast
                    if (pixel < min) {
                        min = pixel;
                    }
                    if (pixel > max) {
                        max = pixel;
                    }
                }
                // short-circuit min/max tests once dynamic range is met
                if (max - min > MIN_DYNAMIC_RANGE) {
                    // finish the rest of the rows quickly
                    for (yy++, offset += width; yy < BLOCK_SIZE; yy++, offset += width) {
                        for (int xx = 0; xx < BLOCK_SIZE; xx++) {
                            sum += luminances[offset + xx] & 0xFF;
                        }
                    }
                }
            }

            // The default estimate is the average of the values in the block.
            int average = sum >> (BLOCK_SIZE_POWER * 2);
            if (max - min <= MIN_DYNAMIC_RANGE) {
                // If variation within the block is low, assume this is a block with only light or only
                // dark pixels. In that case we do not want to use the average, as it would divide this
                // low contrast area into black and white pixels, essentially creating data out of noise.
                //
                // The default assumption is that the block is light/background. Since no estimate for
                // the level of dark pixels exists locally, use half the min for the block.
                average = min / 2;

                if (y > 0 && x > 0) {
                    // Correct the "white background" assumption for blocks that have neighbors by comparing
                    // the pixels in this block to the previously calculated black points. This is based on
                    // the fact that dark barcode symbology is always surrounded by some amount of light
                    // background for which reasonable black point estimates were made. The bp estimated at
                    // the boundaries is used for the interior.

                    // The (min < bp) is arbitrary but works better than other heuristics that were tried.
                    int averageNeighborBlackPoint =
                            (black_points.at<uchar>(y - 1, x) + (2 * black_points.at<uchar>(y, x - 1)) + black_points.at<uchar>(y - 1, x - 1)) / 4;
                    if (min < averageNeighborBlackPoint) {
                        average = averageNeighborBlackPoint;
                    }
                }
            }
            black_points.at<uchar>(y,x) = (uchar)average;
        }
    }
    return black_points;

}


void hybridBinarization(Mat src, Mat & dst)
{
    int width = src.cols;
    int height = src.rows;

    if(width >= MINIMUM_DIMENSION && height >= MINIMUM_DIMENSION)
    {
        std::vector<uchar> luminances(src.begin<uchar>(), src.end<uchar>());

        int sub_width = width >> BLOCK_SIZE_POWER;
        if((width & BLOCK_SIZE_MASK) != 0)
            sub_width ++;

        int sub_height = height >> BLOCK_SIZE_POWER;
        if((height & BLOCK_SIZE_MASK) != 0)
            sub_height ++;

        Mat black_points = calculateBlackPoints(luminances, sub_width, sub_height, width, height);

        dst.create(src.size(), src.type());
        calculateThresholdForBlock(luminances, sub_width, sub_height, width, height,black_points, dst);
    }
    else
    {
        threshold(src, dst, 155, 255, THRESH_OTSU + THRESH_BINARY);
    }

}
};
};