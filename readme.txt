DataAugmentation ver1.0
2014/10/14 takuya minagawa

1. Introduction
This program transform input images with rotation, slide, blur, and noise to create training data of image recognition.
This tool generate random parameters for these transformations.
This image transform proceed in the following order:
- change aspect ratio
- slide
- rotation around Z axis (yaw angle)
- rotation around Y axis (pitch angle)
- rotation around X axis (roll angle)
- Gauss noise
- Gauss blur

The coordinate system is that X axis is horizontal, Y axis is vertical, and Z axis = optical axis of camera in an image.


2. Install
You need boost and OpenCV for build.

boost
http://www.boost.org/

OpenCV
http://opencv.org/

You can use pre-compiled version of windows. Extract DataAugmentation.zip and start "exe" file.
If it does not work, you may need to install VC++2013 runtime.
You can download it at:
http://www.microsoft.com/en-us/download/details.aspx?id=40784


3. How to use
Here is the way to use this program:

DataAugmentation <input> <output directory> [option]

<input>
You can indicate image file, directory/folder, or annotation file as input.  This program automatically judge which type input is.
- Image file can be JPEG, PNG, BMP, PPM, PGM format, etc. 
- Directory can include several image files. This program automatically finds all image files.
- Annotation file must be a text file which has the same format as OpenCV train_cascade uses:
=======================================
<image file path> <number of objects> <top left X> <top left Y> <width> <height> ...
=========================================

For instance, there are two objects in image file "20100915-1/0000004.jpg" and the coordinates are (x,y,w,h)=(10,14,100,120), (141,151,100,120).  Then annotation text must have the following line:
========================================
20100915-1/0000004.jpg 2 10 14 100 120 141 151 100 120
========================================


<output folder>
Generated image files are stored in this folder.



[option]
You can indicate the following options:
-c    configuration file (default: config.txt)
-a    output annotation file (default: annotation.txt)


4. Configuration file
You can indicate parameters for image transformation in a configuration file.
Here is the parameters:

<generate_num>
How many images are generated from one input image?


<aspect_ratio_sigma>
Standard deviation of aspect ratio change.  Aspect ratio is the ratio between width and height written in input annotation file.

<x_slide_sigma>
Standard deviation of slide of annotated position in X direction.  This value is ratio of width of annotated rectangle.
 
<y_slide_sigma>
Standard deviation of slide of annotated position in Y direction.  This value is ratio of height of annotated rectangle.

<yaw_sigma>
Standard deviation of rotation angle (degree of yaw)

<pitch_sigma>
Standard deviation of rotation angle (degree of pitch)

<roll_sigma>
Standard deviation of rotation angle (degree of roll)

<blur_max_sigma>
Maximum standard deviation of Gaussian blur.  Standard deviation of Gauss blur is generated randomly between zero and this value. (pixel)

<noise_max_sigma>
Maximum standard deviation of Gaussian noise.  Standard deviation of Gaussian noise is generated randomly between zero and this value. (pixel value)


5. License
This software is released under "MIT License".
http://opensource.org/licenses/MIT

Notice: The libraries used in this software (OpenCV and Boost) are followed under the license of each.


Takuya MINAGAWA (z.takmin@gmail.com)
