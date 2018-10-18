/*M///////////////////////////////////////////////////////////////////////////////////////
//
//  IMPORTANT: READ BEFORE DOWNLOADING, COPYING, INSTALLING OR USING.
//
//  By downloading, copying, installing or using the software you agree to this license.
//  If you do not agree to this license, do not download, install,
//  copy or use the software.
//
//
//                           License Agreement
//
// Copyright (C) 2014 Takuya MINAGAWA.
// Third party copyrights are property of their respective owners.
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights to
// use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies
// of the Software, and to permit persons to whom the Software is furnished to do
// so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED,
// INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A
// PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
// HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
// OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
// SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
//
//M*/

#ifndef __DATA_AUGMENTATION__
#define __DATA_AUGMENTATION__

#include <opencv2/core/core.hpp>

cv::Mat ImageTransform(const cv::Mat& img, const cv::Rect& area, 
	double yaw_range, double pitch_range, double roll_range,
	double blur_sigma, double noise_sigma, double x_slide, double y_slide,
	double aspect_range, double hflip_ratio, double vflip_ratio, 
	cv::RNG& rng = cv::RNG());


void DataAugmentation(const std::vector<std::string>& img_files, const std::vector<std::vector<cv::Rect>>& areas,
	const std::string& output_folder, const std::string& output_file,
	int num_generate, double yaw_range, double pitch_range, double roll_range,
	double blur_sigma, double noise_sigma, double x_slide, double y_slide, double aspect_range,
	double hflip_ratio, double vflip_ratio);


#endif