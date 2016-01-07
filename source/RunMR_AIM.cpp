/*
 * RunMR_AIM.cpp
 *
 *  Created on: Dec 3, 2015
 *      Author: siddharthadvani
 *
 * Comments: This code emulates MATLAB version of ICASSP 2013.
 */

#include <iostream>
#include "time.h"

#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/features2d/features2d.hpp"

#include "RunMR_AIM.h"
#include "AIM.h"

class AIM;

using namespace cv;

Mat RunMR_AIM (String ImgFileName, int NumPyrLevels)
{
	///////////////////////////////
	//{ // Initialize variables
	//////////////////////////////

	Mat OriginalInputImage, Basis;

	// Temp Outputs
	Mat InfoImage, InfoImage0, InfoImage1, InfoImage2;

	// Final Output
	Mat MRInfoImage;

	int resize_percentage;
	int num_pyramid_levels;

	String basis_file_name;

	clock_t t1, t2;

	///////////////////////////////
	//} // Initialize variables
	//////////////////////////////


	///////////////////////////////
	//{ // Set defaults for arguments
	//////////////////////////////

	OriginalInputImage = imread(ImgFileName, IMREAD_UNCHANGED);

	num_pyramid_levels = NumPyrLevels;   // only 0 is supported (AIM) 

	resize_percentage = 100;    	// no resize

	basis_file_name = "basis.yml";

	///////////////////////////////
	//} // Set defaults for arguments
	//////////////////////////////


	///////////////////////////////
	//{ // Pre-Processing
	//////////////////////////////

	Mat InputImage, NormInputImage;

	if (resize_percentage < 100)
	{
		Size im_size(256, 256);

		resize(OriginalInputImage, InputImage, im_size, 0, 0);
	}
	else
		InputImage = OriginalInputImage;

	double minVal, maxVal;
	minMaxLoc(InputImage, &minVal, &maxVal, NULL, NULL);

	InputImage.convertTo(NormInputImage,CV_8UC3,1,0);

	///////////////////////////////
	//} // Pre-Processing
	//////////////////////////////


	///////////////////////////////
	//{ // MAIN
	//////////////////////////////

	Mat I0 = NormInputImage;
	
	// Instantiate AIM
	AIM mkAIM("basis.yml", 19);
	Basis = mkAIM.LoadBasis(basis_file_name);
		
	switch (num_pyramid_levels)
	{
		case 3:
	    {		
			cout << "Build Pyramid with " << num_pyramid_levels << " levels" << endl;
			vector<Mat> pyramid_images(num_pyramid_levels);
			// Recursively applies pyrDown
			cv::buildPyramid(I0,pyramid_images,num_pyramid_levels-1);
			
			// Level 0 processing
			int I0Height = pyramid_images[0].rows;
			int I0Width = pyramid_images[0].cols;
			int fp_row0 = round(I0Height/2);
			int fp_col0 = round(I0Width/2);
			Mat MR_Foveated_Infomap0(I0Height,I0Width, CV_32SC1);
			cout << "Level 0: " << I0Height << "x" << I0Width << endl;
			// Level 1 processing
			int I1Height = pyramid_images[1].rows;
			int I1Width = pyramid_images[1].cols;
			int fp_row1 = round(I1Height/2);
			int fp_col1 = round(I1Width/2);
			Mat MR_Foveated_Infomap1(I1Height,I1Width, CV_32SC1);
			cout << "Level 1: " << I1Height << "x" << I1Width << endl;
			// Level 2 processing
			int I2Height = pyramid_images[2].rows;
			int I2Width = pyramid_images[2].cols;
			int fp_row2 = round(I2Height/2);
			int fp_col2 = round(I2Width/2);
			Mat MR_Foveated_Infomap2(I2Height,I2Width, CV_32SC1);
			cout << "Level 2: " << I2Height << "x" << I2Width << endl;

			int foveal_range = floor(min(I0Height,I0Width)/2) - 1;
	
			cout << "Foveal Range " << foveal_range << endl;

			// Get Foveas
			Mat ifovea0 = pyramid_images[0].operator ()(Range(fp_row0-floor(foveal_range/2),fp_row0+floor(foveal_range/2)),Range(fp_col0-floor(foveal_range/2),fp_col0+floor(foveal_range/2)));
			cout << "Fovea 0: " << ifovea0.rows << "x" << ifovea0.cols << endl;
			Mat ifovea1 = pyramid_images[1].operator ()(Range(fp_row1-floor(foveal_range/3),fp_row1+floor(foveal_range/3)),Range(fp_col1-floor(foveal_range/3),fp_col1+floor(foveal_range/3)));
			cout << "Fovea 1: " << ifovea1.rows << "x" << ifovea1.cols << endl;
			Mat ifovea2 = pyramid_images[2].operator ()(Range(fp_row2-floor(foveal_range/4),fp_row2+floor(foveal_range/4)),Range(fp_col2-floor(foveal_range/4),fp_col2+floor(foveal_range/4)));
			cout << "Fovea 2: " << ifovea2.rows << "x" << ifovea2.cols << endl;

			//imshow("Fovea 0", ifovea0);
			//imshow("Fovea 1", ifovea1);
			//imshow("Fovea 2", ifovea2);

			// Run AIM
			t1 = clock();
			mkAIM.ist = mkAIM.GetSaliency(ifovea0, Basis);
			InfoImage0 = mkAIM.ist.InfoImage;
			t2 = clock();
			
			float diff = ((float)t2-(float)t1);
			float seconds = diff / CLOCKS_PER_SEC;
			float accum_seconds = seconds;
			cout << "Fovea 0 Computation took " << seconds << " seconds" << endl;

			t1 = clock();
			mkAIM.ist = mkAIM.GetSaliency(ifovea1, Basis);
			InfoImage1 = mkAIM.ist.InfoImage;
			t2 = clock();
			
			diff = ((float)t2-(float)t1);
			seconds = diff / CLOCKS_PER_SEC;
			accum_seconds += seconds;
			cout << "Fovea 1 Computation took " << seconds << " seconds" << endl;

			t1 = clock();
			mkAIM.ist = mkAIM.GetSaliency(ifovea2, Basis);
			InfoImage2 = mkAIM.ist.InfoImage;
			t2 = clock();
			
			diff = ((float)t2-(float)t1);
			seconds = diff / CLOCKS_PER_SEC;
			accum_seconds += seconds;
			cout << "Fovea 2 Computation took " << seconds << " seconds" << endl;
	
			cout << "Total Computation took " << accum_seconds << " seconds" << endl;
	
			// Normalize
			double max0, min0, max1, min1, max2, min2;
			minMaxLoc(InfoImage0, &min0, &max0, NULL, NULL);				
			minMaxLoc(InfoImage1, &min1, &max1, NULL, NULL);				
			minMaxLoc(InfoImage2, &min2, &max2, NULL, NULL);				

			//cout << max0 << " , " << max1 << " , " << max2 << " , " << endl;
			//cout << "Channels: " << InfoImage0.channels() << endl;
			
			InfoImage0.convertTo(InfoImage0, CV_32FC1, 1/double(max0),0);
			InfoImage1.convertTo(InfoImage1, CV_32FC1, 1/double(max1),0);
			InfoImage2.convertTo(InfoImage2, CV_32FC1, 1/double(max2),0);

			//imshow("Info 0", InfoImage0);
			//imshow("Info 1", InfoImage1);
			//imshow("Info 2", InfoImage2);

			// Expand
			Mat iO0 = InfoImage0;
			Mat iO1, iO2, iO2temp;
			pyrUp(InfoImage1, iO1, Size(InfoImage1.cols*2, InfoImage1.rows*2)); 
			pyrUp(InfoImage2, iO2temp, Size(InfoImage2.cols*2, InfoImage2.rows*2)); 
			pyrUp(iO2temp, iO2, Size(iO2temp.cols*2, iO2temp.rows*2)); 

			int iH0 = iO0.rows;
			int iW0 = iO0.cols;

			int iH1 = iO1.rows;
			int iW1 = iO1.cols;

			int iH2 = iO2.rows;
			int iW2 = iO2.cols;

			// Fill in borders
			Mat iOut0 = Mat::zeros(I0Height,I0Width,CV_32FC1);
			Mat iOut1 = Mat::zeros(I0Height,I0Width,CV_32FC1);
			Mat iOut2 = Mat::zeros(I0Height,I0Width,CV_32FC1);

			Mat roi0(iOut0, Rect(fp_col0-floor(iW0/2),fp_row0-floor(iH0/2),iW0, iH0));
			iO0.copyTo(roi0);
			
			Mat roi1(iOut1, Rect(fp_col0-floor(iW1/2),fp_row0-floor(iH1/2),iW1, iH1));
			iO1.copyTo(roi1);
			
			Mat roi2(iOut2, Rect(fp_col0-floor(iW2/2),fp_row0-floor(iH2/2),iW2, iH2));
			iO2.copyTo(roi2);

			//imshow("InfoExp 0", iOut0);
			//imshow("InfoExp 1", iOut1);
			//imshow("InfoExp 2", iOut2);

			//waitKey(0);
			
			// Weight and add 
//			Mat temp;
//			add(iOut0*1/3, iOut1*2/3, temp);
//			Mat MRInfoImage;
//			add(temp, iOut2, MRInfoImage); 

			// Weight
        	MRInfoImage = iOut0*1/3 + iOut1*2/3 + iOut2*3/3;  
						
			// Convert to 8 bit 
			MRInfoImage.convertTo(MRInfoImage, CV_8UC1, 255, 0);

			//imshow("MR-AIM Output", MRInfoImage);
			//waitKey(0);
			return MRInfoImage;
		}
		case 0:
		{
			// Run AIM	
			t1 = clock();
			mkAIM.ist = mkAIM.GetSaliency(I0, Basis);
			InfoImage = mkAIM.ist.InfoImage;
			t2 = clock();

			float diff = ((float)t2-(float)t1);
			float seconds = diff / CLOCKS_PER_SEC;
			cout << "[INFO]: Base Computation took " << seconds << " seconds" << endl;
			
			// Normalize
			double max, min;
			minMaxLoc(InfoImage, &min, &max, NULL, NULL);				
			InfoImage.convertTo(InfoImage, CV_32FC1, 1/double(max),0);

			// Convert to 8 bit
			InfoImage.convertTo(InfoImage, CV_8UC1, 255, 0);
			//imshow("AIM Output", InfoImage);
			//waitKey(0);
			return InfoImage; 
		}
		default:
			return MRInfoImage;
	}
}

