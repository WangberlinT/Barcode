// This file is part of OpenCV project.
// It is subject to the license terms in the LICENSE file found in the top-level directory
// of this distribution and at http://opencv.org/license.html.
// Modified from ZXing. Copyright ZXing authors.
// Licensed under the Apache License, Version 2.0 (the "License").
#ifndef __OPENCV_BARCODE_HYBRID_BINARIZER_HPP__
#define __OPENCV_BARCODE_HYBRID_BINARIZER_HPP__

#include <opencv2/imgproc.hpp>
namespace cv {
namespace barcode {

// This class uses 5x5 blocks to compute local luminance, where each block is 8x8 pixels.
// So this is the smallest dimension in each axis we can accept.
constexpr static int BLOCK_SIZE_POWER = 3;
constexpr static int BLOCK_SIZE = 1 << BLOCK_SIZE_POWER; // ...0100...00
constexpr static int BLOCK_SIZE_MASK = BLOCK_SIZE - 1;   // ...0011...11
constexpr static int MINIMUM_DIMENSION = BLOCK_SIZE * 5;
constexpr static int MIN_DYNAMIC_RANGE = 24;

void hybridBinarization(InputArray _src, OutputArray& _dst);
};
};
#endif //__OPENCV_BARCODE_HYBRID_BINARIZER_HPP__
