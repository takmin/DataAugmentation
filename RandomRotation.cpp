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

#include "RandomRotation.h"
#include "Util.h"
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>



//!ヨー／ピッチ／ロールと平行移動成分から、カメラ外部行列（回転＋並進）を求める
/*!
\param[in] yaw ヨー
\param[in] pitch ピッチ
\param[in] roll ロール
\param[in] trans_x 並進移動X成分
\param[in] trans_y 並進移動Y成分
\param[in] trans_z 並進移動Z成分
\param[out] external_matrix 外部行列
*/
void composeExternalMatrix(float yaw, float pitch, float roll, float trans_x, float trans_y, float trans_z, cv::Mat& external_matrix)
{
	external_matrix.release();
	external_matrix.create(3, 4, CV_64FC1);

	double sin_yaw = sin((double)yaw * CV_PI / 180);
	double cos_yaw = cos((double)yaw * CV_PI / 180);
	double sin_pitch = sin((double)pitch * CV_PI / 180);
	double cos_pitch = cos((double)pitch * CV_PI / 180);
	double sin_roll = sin((double)roll * CV_PI / 180);
	double cos_roll = cos((double)roll * CV_PI / 180);

	external_matrix.at<double>(0, 0) = cos_pitch * cos_yaw;
	external_matrix.at<double>(0, 1) = -cos_pitch * sin_yaw;
	external_matrix.at<double>(0, 2) = sin_pitch;
	external_matrix.at<double>(1, 0) = cos_roll * sin_yaw + sin_roll * sin_pitch * cos_yaw;
	external_matrix.at<double>(1, 1) = cos_roll * cos_yaw - sin_roll * sin_pitch * sin_yaw;
	external_matrix.at<double>(1, 2) = -sin_roll * cos_pitch;
	external_matrix.at<double>(2, 0) = sin_roll * sin_yaw - cos_roll * sin_pitch * cos_yaw;
	external_matrix.at<double>(2, 1) = sin_roll * cos_yaw + cos_roll * sin_pitch * sin_yaw;
	external_matrix.at<double>(2, 2) = cos_roll * cos_pitch;

	external_matrix.at<double>(0, 3) = trans_x;
	external_matrix.at<double>(1, 3) = trans_y;
	external_matrix.at<double>(2, 3) = trans_z;
}


//! 矩形の四隅の座標をそれぞれ斉次座標系へ変換
cv::Mat Rect2Mat(const cv::Rect& img_rect)
{
	// 画像プレートの四隅の座標
	cv::Mat srcCoord(3, 4, CV_64FC1);
	srcCoord.at<double>(0, 0) = img_rect.x;
	srcCoord.at<double>(1, 0) = img_rect.y;
	srcCoord.at<double>(2, 0) = 1;
	srcCoord.at<double>(0, 1) = img_rect.x + img_rect.width;
	srcCoord.at<double>(1, 1) = img_rect.y;
	srcCoord.at<double>(2, 1) = 1;
	srcCoord.at<double>(0, 2) = img_rect.x + img_rect.width;
	srcCoord.at<double>(1, 2) = img_rect.y + img_rect.height;
	srcCoord.at<double>(2, 2) = 1;
	srcCoord.at<double>(0, 3) = img_rect.x;
	srcCoord.at<double>(1, 3) = img_rect.y + img_rect.height;
	srcCoord.at<double>(2, 3) = 1;

	return srcCoord;
}


//! 入力画像の四隅をtransMに沿って透視変換し、出力画像の外接長方形を求める
/*!
\param[in] img_size 入力画像サイズ
\param[in] transM 3x3の透視変換行列(CV_64FC1)
\param[out] CircumRect 出力画像の外接長方形
*/
void CircumTransImgRect(const cv::Size& img_size, const cv::Mat& transM, cv::Rect_<double>& CircumRect)
{
	// 入力画像の四隅を斉次座標へ変換
	cv::Mat cornersMat = Rect2Mat(cv::Rect(0, 0, img_size.width, img_size.height));

	// 座標変換し、範囲を取得
	cv::Mat dstCoord = transM * cornersMat;
	double min_x = std::min(dstCoord.at<double>(0, 0) / dstCoord.at<double>(2, 0), dstCoord.at<double>(0, 3) / dstCoord.at<double>(2, 3));
	double max_x = std::max(dstCoord.at<double>(0, 1) / dstCoord.at<double>(2, 1), dstCoord.at<double>(0, 2) / dstCoord.at<double>(2, 2));
	double min_y = std::min(dstCoord.at<double>(1, 0) / dstCoord.at<double>(2, 0), dstCoord.at<double>(1, 1) / dstCoord.at<double>(2, 1));
	double max_y = std::max(dstCoord.at<double>(1, 2) / dstCoord.at<double>(2, 2), dstCoord.at<double>(1, 3) / dstCoord.at<double>(2, 3));

	CircumRect.x = min_x;
	CircumRect.y = min_y;
	CircumRect.width = max_x - min_x;
	CircumRect.height = max_y - min_y;
}



//! 入力画像と出力画像の座標の対応関係を計算
/*!
\param[in] src_size 入力画像サイズ
\param[in] dst_rect 入力画像を透視変換した時の出力画像の外接長方形
\param[in] transMat 4x4の回転/平行移動行列(CV_64FC1)。原点で回転させて、Z軸方向に平行移動したもの
\param[out] map_x 出力画像の各座標に対する入力画像のx座標
\param[out] map_y 出力画像の各座標に対する入力画像のy座標

transMatは入力画像を３次元的に回転し、その中心を(0,0,Z)に置くように変換する行列。
出力画像は焦点距離が1のカメラを想定し、入力画像をここに透視変換する。
ただし、スケールを合わせるために出力画像のX,Y座標を1/Zする。
出力画像上の座標が(dx, dy)で与えられた時、原点とその点を結ぶ直線は(dx*r, dy*r, Z*r)で表される。
入力画像上の座標(sx,sy)を３次元座標で表すとtransMat*(sx, sy, 0, 1)^T となるので、(sx, sy)と(dx, dy)の関係は
(sx, sy, 0, 1)^T = transMat^(-1) * (dx*r, dy*r, Z*r)
となる。
ここから、rを消すことでdxとdyに対応するsxとsyが求まる。
*/
void CreateMap(const cv::Size& src_size, const cv::Rect_<double>& dst_rect, const cv::Mat& transMat, cv::Mat& map_x, cv::Mat& map_y)
{
	map_x.create(dst_rect.size(), CV_32FC1);
	map_y.create(dst_rect.size(), CV_32FC1);

	double Z = transMat.at<double>(2, 3);

	cv::Mat invTransMat = transMat.inv();	// 逆行列
	cv::Mat dst_pos(3, 1, CV_64FC1);	// 出力画像上の座標
	dst_pos.at<double>(2, 0) = Z;
	for (int dy = 0; dy<map_x.rows; dy++){
		dst_pos.at<double>(1, 0) = dst_rect.y + dy;
		for (int dx = 0; dx<map_x.cols; dx++){
			dst_pos.at<double>(0, 0) = dst_rect.x + dx;
			cv::Mat rMat = -invTransMat(cv::Rect(3, 2, 1, 1)) / (invTransMat(cv::Rect(0, 2, 3, 1)) * dst_pos);
			cv::Mat src_pos = invTransMat(cv::Rect(0, 0, 3, 2)) * dst_pos * rMat + invTransMat(cv::Rect(3, 0, 1, 2));
			map_x.at<float>(dy, dx) = src_pos.at<double>(0, 0) + (float)src_size.width / 2;
			map_y.at<float>(dy, dx) = src_pos.at<double>(1, 0) + (float)src_size.height / 2;
		}
	}
}


void RotateImage(const cv::Mat& src, cv::Mat& dst, float yaw, float pitch, float roll,
	float Z = 1000, int interpolation = cv::INTER_LINEAR, int boarder_mode = cv::BORDER_CONSTANT, const cv::Scalar& border_color = cv::Scalar(0, 0, 0))
{
	// rotation matrix
	cv::Mat rotMat_3x4;
	composeExternalMatrix(yaw, pitch, roll, 0, 0, Z, rotMat_3x4);

	cv::Mat rotMat = cv::Mat::eye(4, 4, rotMat_3x4.type());
	rotMat_3x4.copyTo(rotMat(cv::Rect(0, 0, 4, 3)));

	// From 2D coordinates to 3D coordinates
	// The center of image is (0,0,0)
	cv::Mat invPerspMat = cv::Mat::zeros(4, 3, CV_64FC1);
	invPerspMat.at<double>(0, 0) = 1;
	invPerspMat.at<double>(1, 1) = 1;
	invPerspMat.at<double>(3, 2) = 1;
	invPerspMat.at<double>(0, 2) = -(double)src.cols / 2;
	invPerspMat.at<double>(1, 2) = -(double)src.rows / 2;

	// ３次元座標から２次元座標へ透視変換
	cv::Mat perspMat = cv::Mat::zeros(3, 4, CV_64FC1);
	perspMat.at<double>(0, 0) = Z;
	perspMat.at<double>(1, 1) = Z;
	perspMat.at<double>(2, 2) = 1;

	// 座標変換し、出力画像の座標範囲を取得
	cv::Mat transMat = perspMat * rotMat * invPerspMat;
	cv::Rect_<double> CircumRect;
	CircumTransImgRect(src.size(), transMat, CircumRect);

	// 出力画像と入力画像の対応マップを作成
	cv::Mat map_x, map_y;
	CreateMap(src.size(), CircumRect, rotMat, map_x, map_y);
	cv::remap(src, dst, map_x, map_y, interpolation, boarder_mode, border_color);
}


// Keep center and expand rectangle for rotation
cv::Rect ExpandRectForRotate(const cv::Rect& area)
{
	cv::Rect exp_rect;
	
	int w = (double)(area.width + area.height) / std::sqrt(2.0) + 0.5;
	
	exp_rect.width = w;
	exp_rect.height = w;
	exp_rect.x = area.x - (exp_rect.width - area.width) / 2;
	exp_rect.y = area.y - (exp_rect.height - area.height) / 2;

	return exp_rect;
}


void RandomRotateImage(const cv::Mat& src, cv::Mat& dst, float yaw_sigma, float pitch_sigma, float roll_sigma, const cv::Rect& area, cv::RNG& rng,
	float Z, int interpolation, int boarder_mode, const cv::Scalar& boarder_color)
{
	double yaw = rng.gaussian(yaw_sigma);
	double pitch = rng.gaussian(pitch_sigma);
	double roll = rng.gaussian(roll_sigma);
	//double yaw = rng.uniform(-yaw_range / 2, yaw_range / 2);
	//double pitch = rng.uniform(-pitch_range / 2, pitch_range / 2);
	//double roll = rng.uniform(-roll_range / 2, roll_range / 2);

	cv::Rect rect = (area.width <= 0 || area.height <= 0) ? cv::Rect(0, 0, src.cols, src.rows) : 
		ExpandRectForRotate(area);
	rect = util::TruncateRectKeepCenter(rect, src.size());

	cv::Mat rot_img;
	RotateImage(src(rect).clone(), rot_img, yaw, pitch, roll, Z, interpolation, boarder_mode, boarder_color);

	cv::Rect dst_area((rot_img.cols - area.width) / 2, (rot_img.rows - area.height) / 2, area.width, area.height);
	dst_area = util::TruncateRectKeepCenter(dst_area, rot_img.size());
	dst = rot_img(dst_area).clone();
}

