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
#include <boost/program_options.hpp>
#include <boost/filesystem/path.hpp>
#include <boost/filesystem/operations.hpp>
#include <iostream>
#include "Util.h"
#include "DataAugmentation.h"

using namespace boost::program_options;

void print_help(int argc, char * argv[], const options_description& opt)
{
	std::cout << argv[0] << " <input annotation> <output folder> [option]" << std::endl;
	std::cout << opt << std::endl;
}


bool ParseCommandLine(int argc, char * argv[], std::string& conf_file,
	std::string& input_anno_file, std::string& output_folder, std::string& output_anno_file)
{
	// option argments
	options_description opt("option");
	opt.add_options()
		("help,h", "Print help")
		("conf,c", value<std::string>()->default_value("config.txt"), "configuration file")
		("anno,a", value<std::string>()->default_value("annotation.txt"), "output annotation file");

	variables_map argmap;
	try{
		// get command line argments
		store(parse_command_line(argc, argv, opt), argmap);
		notify(argmap);

		// print help
		if (argmap.count("help") || argc < 3){
			print_help(argc, argv, opt);
			return false;
		}

		input_anno_file = argv[1];
		output_folder = argv[2];
		if (input_anno_file.find("-") == 0 || output_folder.find("-") == 0){
			print_help(argc, argv, opt);
			return false;
		}
		conf_file = argmap["conf"].as<std::string>();
		output_anno_file = argmap["anno"].as<std::string>();
	}
	catch (std::exception& e)
	{
		std::cout << std::endl << e.what() << std::endl;
		print_help(argc, argv, opt);
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
	// set argments of command options
	options_description opt("option");
	opt.add_options()
		("generate_num", value<int>(), "number to generate per an input image")
		("yaw_sigma", value<double>()->default_value(0), "sigma of yaw rotation angle (degree)")
		("pitch_sigma", value<double>()->default_value(0), "sigma of pitch rotation angle (degree)")
		("roll_sigma", value<double>()->default_value(0), "sigma of roll rotation angle (degree)")
		("blur_max_sigma", value<double>()->default_value(0), "maximum value of sigma for gaussian blur (pixel)")
		("noise_max_sigma", value<double>()->default_value(0), "maximum value of sigma for gaussian noise (pixel value)")
		("x_slide_sigma", value<double>()->default_value(0), "sigma of slide in x direction (ratio of width)")
		("y_slide_sigma", value<double>()->default_value(0), "sigma of slide in y direction (ratio of height)")
		("aspect_ratio_sigma", value<double>()->default_value(0), "sigma of aspect ratio deformation")
		("horizontal_flip", value<double>()->default_value(0), "probability to flip image from left to right (from 0 to 1)")
		("vertical_flip", value<double>()->default_value(0), "probability to flip image from up to down (from 0 to 1)");

	variables_map argmap;
	try{
		std::ifstream ifs(conf_file);
		if (!ifs.is_open()){
			std::string err_msg = "Fail to open config file \"" + conf_file + "\".";
			throw std::exception(err_msg.c_str());
		}

		// ÉRÉ}ÉìÉhà¯êîÇÃéÊìæ
		store(parse_config_file(ifs, opt), argmap);
		notify(argmap);

		num_generate = argmap["generate_num"].as<int>();
		yaw_sigma = argmap["yaw_sigma"].as<double>();
		pitch_sigma = argmap["pitch_sigma"].as<double>();
		roll_sigma = argmap["roll_sigma"].as<double>();
		blur_max_sigma = argmap["blur_max_sigma"].as<double>(); 
		noise_max_sigma = argmap["noise_max_sigma"].as<double>();
		x_slide_sigma = argmap["x_slide_sigma"].as<double>(); 
		y_slide_sigma = argmap["y_slide_sigma"].as<double>();
		aspect_sigma = argmap["aspect_ratio_sigma"].as<double>();
		hflip_ratio = argmap["horizontal_flip"].as<double>();
		vflip_ratio = argmap["vertical_flip"].as<double>();

		if (num_generate < 0 || yaw_sigma < 0 || pitch_sigma < 0 || roll_sigma < 0 ||
			blur_max_sigma < 0 || noise_max_sigma < 0 ||
			x_slide_sigma < 0 || y_slide_sigma < 0 || aspect_sigma < 0){
			throw std::exception("All value must NOT be negative.");
		}
		if (hflip_ratio < 0 || hflip_ratio > 1) {
			throw std::exception("\"horizontal_flip\" must be between 0 and 1");
		}
		if (vflip_ratio < 0 || vflip_ratio > 1) {
			throw std::exception("\"vertical_flip\" must be between 0 and 1");
		}

		return true;
	}
	catch (std::exception& e)
	{
		std::cout << std::endl << e.what() << std::endl;
		return false;
	}

	return true;
}


void GetImageFileNames(const std::string& input_name, std::vector<std::string>& img_files, std::vector<std::vector<cv::Rect>>& positions)
{
	using namespace boost::filesystem;

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


