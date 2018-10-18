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

#ifndef __UTIL__
#define __UTIL__

#include <opencv2/core/core.hpp>

namespace util{

	//! 画像からはみ出る矩形の整形
	cv::Rect TruncateRect(const cv::Rect& obj_rect, const cv::Size& img_size);

	//! 画像からはみ出る矩形を中心を維持しながら整形
	cv::Rect TruncateRectKeepCenter(const cv::Rect& obj_rect, const cv::Size& max_size);

	//! アノテーションファイルの読み込み
	/*!
	opencv_createsamles.exeと同形式のアノテーションファイル読み書き
	ReadCsvFile()関数必須
	\param[in] gt_file アノテーションファイル名
	\param[out] imgpathlist 画像ファイルへのパス
	\param[out] rectlist 各画像につけられたアノテーションのリスト
	\return 読み込みの成否
	*/
	bool LoadAnnotationFile(const std::string& gt_file, std::vector<std::string>& imgpathlist, std::vector<std::vector<cv::Rect>>& rectlist);

	//! アノテーションファイルへ追記
	/*!
	opencv_createsamles.exeと同形式のアノテーションファイル読み書き
	\param[in] anno_file アノテーションファイル名
	\param[in] img_file 画像ファイルへのパス
	\param[int] obj_rects 各画像につけられたアノテーションのリスト
	\return 保存の成否
	*/
	bool AddAnnotationLine(const std::string& anno_file, const std::string& img_file, const std::vector<cv::Rect>& obj_rects, const std::string& sep);

	// ディレクトリから画像ファイル名一覧を取得
	bool ReadImageFilesInDirectory(const std::string& img_dir, std::vector<std::string>& image_lists);

	bool hasImageExtention(const std::string& filename);

	bool ReadCSVFile(const std::string& input_file, std::vector<std::vector<std::string>>& output_strings,
		const std::vector<std::string>& separater_vec = std::vector<std::string>());;

	std::vector<std::string> TokenizeString(const std::string& input_string, const std::vector<std::string>& separater_vec);
}

#endif