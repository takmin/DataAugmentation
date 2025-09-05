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

/**********************************************
DataAugmentation:
Add rotation, slide, blur, and nose into input images
DataAugmentation <annotation name> <output folder> -a <output annnotation> -c <config file>
***********************************************/


#include <opencv2/highgui/highgui.hpp>
#include <opencv2/objdetect/objdetect.hpp>
#include <opencv2/core/utility.hpp>
#include <filesystem>
#include <fstream>
#include <iostream>
#include "Util.h"
#include "DataAugmentation.h"

void print_help(const cv::CommandLineParser& parser)
{
	std::cout << "Usage: DataAugmentation <input annotation> <output folder> [-a=<output annotation>] [-c=<config file>] [--help]" << std::endl;
	parser.printMessage();
}

bool ParseCommandLine(int argc, char* argv[], std::string& conf_file,
	std::string& input_anno_file, std::string& output_folder, std::string& output_anno_file)
{
	const std::string keys =
		"{help h |        | Print help }"
		"{@input |        | Input annotation or directory }"
		"{@output|        | Output folder }"
		"{a      |annotation.txt| Output annotation file }"
		"{c      |config.txt    | Configuration file }";

	cv::CommandLineParser parser(argc, argv, keys);

	if (parser.has("help") || argc < 2) {
		print_help(parser);
		return false;
	}

	input_anno_file = parser.get<std::string>(0);
	output_folder = parser.get<std::string>(1);
	conf_file = parser.get<std::string>("c");
	output_anno_file = parser.get<std::string>("a");

	if (!parser.check()) {
		print_help(parser);
		return false;
	}
	return true;
}


bool LoadConf(const std::string& conf_file, int& num_generate,
	double& yaw_sigma, double& pitch_sigma, double& roll_sigma,
	double& blur_max_sigma, double& noise_max_sigma,
	double& x_slide_sigma, double& y_slide_sigma, double& aspect_sigma,
	double& hflip_ratio, double& vflip_ratio)
{
	std::ifstream ifs(conf_file);
	if (!ifs.is_open()) {
		std::cout << "Fail to open config file \"" << conf_file << "\"." << std::endl;
		return false;
	}

	// デフォルト値
	num_generate = 1;
	yaw_sigma = pitch_sigma = roll_sigma = 0;
	blur_max_sigma = noise_max_sigma = 0;
	x_slide_sigma = y_slide_sigma = aspect_sigma = 0;
	hflip_ratio = vflip_ratio = 0;

	std::string line;
	while (std::getline(ifs, line)) {
		std::istringstream iss(line);
		std::string key, eq, value;
		if (!(iss >> key >> eq >> value)) continue;
		if (eq != "=") continue;
		if (key == "generate_num") num_generate = std::stoi(value);				// number to generate per an input image
		else if (key == "yaw_sigma") yaw_sigma = std::stod(value);				// sigma of yaw rotation angle (degree)
		else if (key == "pitch_sigma") pitch_sigma = std::stod(value);			// sigma of pitch rotation angle (degree)
		else if (key == "roll_sigma") roll_sigma = std::stod(value);			// sigma of roll rotation angle (degree)
		else if (key == "blur_max_sigma") blur_max_sigma = std::stod(value);	// maximum value of sigma for gaussian blur (pixel)
		else if (key == "noise_max_sigma") noise_max_sigma = std::stod(value);	// maximum value of sigma for gaussian noise (pixel value)
		else if (key == "x_slide_sigma") x_slide_sigma = std::stod(value);		// sigma of slide in x direction (ratio of width)
		else if (key == "y_slide_sigma") y_slide_sigma = std::stod(value);		// sigma of slide in y direction (ratio of height)
		else if (key == "aspect_ratio_sigma") aspect_sigma = std::stod(value);	// sigma of aspect ratio deformation
		else if (key == "horizontal_flip") hflip_ratio = std::stod(value);		// probability to flip image from left to right (from 0 to 1)
		else if (key == "vertical_flip") vflip_ratio = std::stod(value);		// probability to flip image from up to down (from 0 to 1)
		else {
			std::cout << "Unknown key \"" << key << "\" in config file." << std::endl;
			return false;
		}
	}

	// 簡易チェック
	if (num_generate < 0 || yaw_sigma < 0 || pitch_sigma < 0 || roll_sigma < 0 ||
		blur_max_sigma < 0 || noise_max_sigma < 0 ||
		x_slide_sigma < 0 || y_slide_sigma < 0 || aspect_sigma < 0) {
		std::cout << "All value must NOT be negative." << std::endl;
		return false;
	}
	if (hflip_ratio < 0 || hflip_ratio > 1) {
		std::cout << "\"horizontal_flip\" must be between 0 and 1" << std::endl;
		return false;
	}
	if (vflip_ratio < 0 || vflip_ratio > 1) {
		std::cout << "\"vertical_flip\" must be between 0 and 1" << std::endl;
		return false;
	}
	return true;
}


void GetImageFileNames(const std::string& input_name, std::vector<std::string>& img_files, std::vector<std::vector<cv::Rect>>& positions)
{
	using namespace std::filesystem;

	// if input_name is directory
	if (is_directory(path(input_name))){
		util::ReadImageFilesInDirectory(input_name, img_files);
	}
	else if (util::hasImageExtention(input_name)){
		img_files.push_back(input_name);
	}
	else{
		util::LoadAnnotationFile(input_name, img_files, positions);
	}
}


int main(int argc, char * argv[])
{
	std::string conf_file, input_name, output_folder, output_anno_file;
	if (!ParseCommandLine(argc, argv, conf_file, input_name, output_folder, output_anno_file))
		return -1;

	int num_generate;
	double yaw_range, pitch_range, roll_range, x_slide, y_slide, 
		blur_sigma, noise_sigma, aspect_range, hflip_ratio, vflip_ratio;
	if (!LoadConf(conf_file, num_generate, yaw_range, pitch_range, roll_range,
		blur_sigma, noise_sigma, x_slide, y_slide, aspect_range, hflip_ratio, vflip_ratio))
		return -1;

	std::vector<std::string> img_files;
	std::vector<std::vector<cv::Rect>> obj_positions;
	GetImageFileNames(input_name, img_files, obj_positions);

	DataAugmentation(img_files, obj_positions, output_folder, output_anno_file, num_generate, yaw_range, pitch_range, roll_range,
		blur_sigma, noise_sigma, x_slide, y_slide, aspect_range, hflip_ratio, vflip_ratio);

	return 0;
}


