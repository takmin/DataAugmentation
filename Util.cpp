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

#include "Util.h"
#include <fstream>
#include <boost/filesystem/operations.hpp>
#include <boost/filesystem/path.hpp>

namespace util{
	
	//! はみ出る領域をカット
	cv::Rect TruncateRect(const cv::Rect& obj_rect, const cv::Size& img_size)
	{
		cv::Rect resize_rect = obj_rect;
		if (obj_rect.x < 0){
			resize_rect.x = 0;
			resize_rect.width += obj_rect.x;
		}
		if (obj_rect.y < 0){
			resize_rect.y = 0;
			resize_rect.height += obj_rect.y;
		}
		if (resize_rect.x + resize_rect.width > img_size.width){
			resize_rect.width = img_size.width - resize_rect.x;
		}
		if (resize_rect.y + resize_rect.height > img_size.height){
			resize_rect.height = img_size.height - resize_rect.y;
		}

		return resize_rect;
	}


	//! 中心を動かさずに、はみ出る領域をカット
	cv::Rect TruncateRectKeepCenter(const cv::Rect& obj_rect, const cv::Size& max_size)
	{
		cv::Rect exp_rect = obj_rect;
		if (exp_rect.x < 0){
			exp_rect.width += 2 * exp_rect.x;
			exp_rect.x = 0;
		}
		if (exp_rect.y < 0){
			exp_rect.height += 2 * exp_rect.y;
			exp_rect.y = 0;
		}
		if (exp_rect.x + exp_rect.width > max_size.width){
			exp_rect.x += (exp_rect.x + exp_rect.width - max_size.width) / 2;
			exp_rect.width = max_size.width - exp_rect.x;
		}
		if (exp_rect.y + exp_rect.height > max_size.height){
			exp_rect.y += (exp_rect.y + exp_rect.height - max_size.height) / 2;
			exp_rect.height = max_size.height - exp_rect.y;
		}
		return exp_rect;
	}


	//! アノテーションファイルの読み込み
	/*!
	opencv_createsamles.exeと同形式のアノテーションファイル読み書き
	ReadCsvFile()関数必須
	\param[in] gt_file アノテーションファイル名
	\param[out] imgpathlist 画像ファイルへのパス
	\param[out] rectlist 各画像につけられたアノテーションのリスト
	\return 読み込みの成否
	*/
	bool LoadAnnotationFile(const std::string& gt_file, std::vector<std::string>& imgpathlist, std::vector<std::vector<cv::Rect>>& rectlist)
	{
		std::vector<std::vector<std::string>> tokenized_strings;
		std::vector<std::string> sep;
		sep.push_back(" ");
		if (!ReadCSVFile(gt_file, tokenized_strings, sep))
			return false;

		std::vector<std::vector<std::string>>::iterator it, it_end = tokenized_strings.end();
		for (it = tokenized_strings.begin(); it != it_end; it++){
			int num_str = it->size();
			if (num_str < 2)
				continue;

			std::string filename = (*it)[0];
			if (filename.empty() || filename.find("#") != std::string::npos){
				continue;
			}

			imgpathlist.push_back(filename);
			int obj_num = atoi((*it)[1].c_str());
			std::vector<cv::Rect> rects;
			for (int i = 0; i<obj_num && 4 * i + 6 <= num_str; i++){
				int j = 4 * i + 2;
				cv::Rect obj_rect;
				obj_rect.x = atoi((*it)[j].c_str());
				obj_rect.y = atoi((*it)[j + 1].c_str());
				obj_rect.width = atoi((*it)[j + 2].c_str());
				obj_rect.height = atoi((*it)[j + 3].c_str());
				rects.push_back(obj_rect);
			}
			rectlist.push_back(rects);
		}

		return true;
	}


	//! アノテーションファイルへ追記
	/*!
	opencv_createsamles.exeと同形式のアノテーションファイル読み書き
	\param[in] anno_file アノテーションファイル名
	\param[in] img_file 画像ファイルへのパス
	\param[int] obj_rects 各画像につけられたアノテーションのリスト
	\return 保存の成否
	*/
	bool AddAnnotationLine(const std::string& anno_file, const std::string& img_file, const std::vector<cv::Rect>& obj_rects, const std::string& sep)
	{
		// 出力ファイルを開く
		std::ofstream ofs(anno_file, std::ios::app);
		if (!ofs.is_open()){
			return false;
		}

		ofs << img_file << sep << obj_rects.size();
		for (int i = 0; i < obj_rects.size(); i++){
			cv::Rect rect = obj_rects[i];
			ofs << sep << rect.x << sep << rect.y << sep << rect.width << sep << rect.height;
		}
		ofs << std::endl;
		return true;
	}


	// ディレクトリから画像ファイル名一覧を取得
	bool ReadImageFilesInDirectory(const std::string& img_dir, std::vector<std::string>& image_lists)
	{
		using namespace boost::filesystem;

		path img_dir_path(img_dir);
		if (!is_directory(img_dir_path)){
			return false;
		}

		directory_iterator end;
		for (directory_iterator p(img_dir_path); p != end; ++p){
			std::string file_name = p->path().generic_string();
			if (hasImageExtention(file_name)){
				image_lists.push_back(file_name);
			}
		}
		return true;
	}

	bool hasImageExtention(const std::string& filename){
		std::string ext = boost::filesystem::path(filename).extension().string();

		return (ext == ".jpg" || ext == ".JPG" || ext == ".jpeg" || ext == ".JPEG" ||
			ext == ".bmp" || ext == ".BMP" || ext == ".png" || ext == ".PNG" ||
			ext == ".dib" || ext == ".DIB" || ext == ".pbm" || ext == ".PBM" ||
			ext == ".pgm" || ext == ".PGM" || ext == ".ppm" || ext == ".PPM" ||
			ext == ".sr" || ext == ".SR" || ext == ".ras" || ext == ".RAS");
	}


	bool ReadCSVFile(const std::string& input_file, std::vector<std::vector<std::string>>& output_strings,
		const std::vector<std::string>& separater_vec)
	{
		std::vector<std::string> sep_vec;
		if (separater_vec.empty()){
			sep_vec.push_back(",");
		}
		else{
			sep_vec = separater_vec;
		}
		std::ifstream ifs(input_file);
		if (!ifs.is_open())
			return false;

		output_strings.clear();

		std::string buf;
		while (ifs && std::getline(ifs, buf)){
			std::vector<std::string> str_list = TokenizeString(buf, sep_vec);
			output_strings.push_back(str_list);
		}
		return true;
	}


	std::vector<std::string> TokenizeString(const std::string& input_string, const std::vector<std::string>& separater_vec)
	{
		std::vector<std::string>::const_iterator separater_itr;
		std::vector<std::string::size_type>	index_vec;
		std::string::size_type	index;
		for (separater_itr = separater_vec.begin(); separater_itr != separater_vec.end(); separater_itr++){
			index = 0;
			while (true){
				index = input_string.find(*separater_itr, index);
				if (index == std::string::npos){
					break;
				}
				else{
					index_vec.push_back(index);
					index++;
				}
			}
		}
		sort(index_vec.begin(), index_vec.end());

		std::vector<std::string> ret_substr_vec;
		std::vector<std::string::size_type>::iterator idx_itr;
		std::string::size_type start_idx = 0;
		int str_size;
		for (idx_itr = index_vec.begin(); idx_itr != index_vec.end(); idx_itr++){
			str_size = *idx_itr - start_idx;
			ret_substr_vec.push_back(input_string.substr(start_idx, str_size));
			start_idx = *idx_itr + 1;
		}
		ret_substr_vec.push_back(input_string.substr(start_idx));

		return ret_substr_vec;
	}
}