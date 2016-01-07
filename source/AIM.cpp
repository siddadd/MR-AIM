/*
 * AIM.cpp
 *
 *  Created on: Oct 28, 2012
 *      Author: siddharthadvani
 *
 *  Histogram Bins = 1000
 *  Pixel Masking Threshold = 80%
 *
 *  References :
 *
 *   1. Bruce, N.D.B., Tsotsos, J.K., Saliency,
 *   Attention, and Visual Search: An Information Theoretic
 *   Approach, Journal of Vision 9:3, pp.1-24, 2009,
 *   http://journalofvision.org/9/3/5/, doi:10.1167/9.3.5.
 *
 *   2. Bruce, N.D.B., Tsotsos, J.K., Saliency based on
 *   Information Maximization. Advances in Neural
 *   Information Processing Systems, 18.

 * 	 3. Bruce, N. Features that draw visual attention: An
 *   information theoretic perspective. Neurocomputing,
 *   v. 65-66, pp. 125-133, May 2005.
 *
 */

#include "opencv2/opencv.hpp"

#include "AIM.h"

using namespace cv;
using namespace std;


AIM::AIM() {
	// TODO Auto-generated constructor stub
	UseFixedPoint = false;
	Debug = false;
//	Filename = "basis.yml";
}

AIM::AIM(std::string BasisFilename, int NumFixedPointFractionalBits) {
	UseFixedPoint = true;
	FixedPointScaleFactor = pow((double)2,NumFixedPointFractionalBits);
	Debug = false;
//	Filename = BasisFilename;
}

AIM::~AIM() {
	// TODO Auto-generated destructor stub
}


Mat AIM::LoadBasis(std::string Filename){

	FileStorage fs(Filename, FileStorage::READ);

	if (UseFixedPoint)
	{
		fs["B"] >> basis;
	 	Mat Basis(basis.rows, basis.cols, CV_16UC1);
		basis = basis * FixedPointScaleFactor;
	 	Basis = basis;
	 	fs.release();
	 	return Basis;
	}
	else
	{
		fs["B"] >> basis;
	 	Mat Basis(basis.rows, basis.cols, CV_32FC1);
		Basis = basis;
		fs.release();
		return Basis;
	}
}


Mat AIM::GetConvolution2D(Mat source, Mat kernel) {

	Mat dest = Mat::zeros(source.rows, source.cols, kernel.type());

	Point anchor(-1,-1);  // Anchor is at Filter center

    int borderMode = BORDER_CONSTANT;

    filter2D(source, dest, kernel.depth(), kernel, anchor, 0, borderMode);

    /*
    dest = dest.colRange((kernel.cols-1)/2-1, dest.cols - (kernel.cols-1)/2-1)
               .rowRange((kernel.rows-1)/2-1, dest.rows - (kernel.rows-1)/2-1);
    */

    return dest;
}


/* Input: Set of Convolved Images, SizeofInfoMap
 * Output: InfoMap
 */
Mat AIM::GetInfoMapfromConvolution(vector <Mat> BasisImages, Size InfoMapSize) {

	Mat InfoMap = Mat::zeros(InfoMapSize.height, InfoMapSize.width, CV_32FC1);
	Mat ConstImage = Mat::ones(InfoMapSize.height, InfoMapSize.width, CV_32FC1)*0.000001;

	// How many pixels correspond to 1 degree visual angle
	double sigval = 8;

	// What size of window is needed to contain the above
	int ksize = 31; // In the MATLAB version it is 30x30 window. But OpenCV likes odd size kernels


	for (unsigned int i = 0; i < BasisImages.capacity(); i++)
	{
		/**************************************************
		 * STEP 2 : Get Density
		 ***************************************************/
		Mat DenMap = GetDensity(BasisImages[i]);

		/**************************************************
		 * STEP 3 : Convert to Log Likelihood
		 ***************************************************/
    	// Don't know why we do this step but we do it
       	add(InfoMap, ConstImage, InfoMap);

    	subtract(InfoMap, DenMap, InfoMap);
	}

	/**************************************************
	 * Post-Processing of Information Map
	 ***************************************************/
	double minVal, maxVal;

	minMaxLoc(InfoMap, &minVal, &maxVal, NULL, NULL);

	// Min values scaled to 0
	InfoMap = InfoMap - minVal;

	// Normalize
	InfoMap.convertTo(InfoMap,-1,1/(maxVal-minVal),0);

	Mat gker = getGaussianKernel(ksize,sigval, CV_32F);

	sepFilter2D(InfoMap, InfoMap, -1, gker, gker);   // separable convolution (speeds up execution)

	return InfoMap;
}


/* Input: InfoMap, DisplayThreshold
 * Output: ThreshMap
 */
Mat AIM::GetThreshMap (Mat InfoMap, double dispThresh) {

	Mat InfomapLine = InfoMap.clone();
	Mat InfomapVector = InfomapLine.reshape(1,InfoMap.rows * InfoMap.cols);

	double disp_rank = CalcPercentile(InfomapVector, dispThresh);

	Mat ThreshMap;
	compare(InfoMap,disp_rank,ThreshMap,CMP_GT);

	// ThreshMap.convertTo(ThreshMap, CV_32FC1, 1/double(255), 0);

	return ThreshMap;
}


/* Inputs: InputImage, ThreshMap
 * Output: SaliencyMap
 */
Mat AIM::DoPixelMasking (Mat InputImage, Mat ThreshMap) {

	int NumColorChannels = InputImage.channels();
	int i = 0;
	std::vector <Mat> SaliencyMap (NumColorChannels);

	Mat InputImageRGBColor = Mat::zeros(InputImage.rows, InputImage.cols, InputImage.type());
	cvtColor(InputImage,InputImageRGBColor,cv::COLOR_BGR2RGB);

	std::vector <Mat> InputImageRGB(NumColorChannels);
	split(InputImageRGBColor, InputImageRGB);

	for (i = 0; i < NumColorChannels; i++)
	{
		SaliencyMap[i] = Mat::zeros(InputImage.rows, InputImage.cols, CV_32FC1);
	}

	multiply(InputImageRGB[0],ThreshMap,SaliencyMap[2]);  // R-R channel
	multiply(InputImageRGB[1],ThreshMap,SaliencyMap[1]);  // G-G channel
	multiply(InputImageRGB[2],ThreshMap,SaliencyMap[0]);  // B-B channel

	Mat SalienceMap;
	merge(SaliencyMap,SalienceMap);

	return SalienceMap;
}


/* Input: InputImage
 * Output: DensityMap
 */
Mat AIM::GetDensity(Mat InputImage) {

	int bins = 1000;

	double minVal,  maxVal;

	int numPixels = InputImage.rows * InputImage.cols;

	minMaxLoc(InputImage, &minVal, &maxVal, NULL, NULL);

	Mat LogMap;

	int hbins [] = {bins};
	int plane [] = {0}; // channel no 0
	float range[] = { 0, 1.001 };  // All pixels were normalized while converting to double bit precision, so range is 0 to 1 (1.001 because OpenCV likes exclusiveness in the upper range)
	const float *hranges[] = {range};
	Mat HistImage;

	//Check that we have a well formed input range (i.e. the max-min value should not be zero)
	if (maxVal == minVal)
	{
		cout << "values are same" << endl;
		return Mat::zeros(InputImage.rows, InputImage.cols, InputImage.type());
	}

	// Normalize (needed because range in float)
	InputImage = InputImage - minVal;
	InputImage.convertTo(InputImage, CV_32FC1, 1/(maxVal-minVal),0);

	// Calculate the Histogram
	calcHist(&InputImage, 1,  // one image
				 plane, Mat(), // do no use mask
				 HistImage, 1, hbins,
				 hranges);

	if (Debug == true)
	{
		cout << "value = " << HistImage.at<double>(105) << endl;
		cout << "Histogram calculated" << endl;
	}

	Mat ProbMap;

	calcBackProject(&InputImage,1,0,
					HistImage,ProbMap,
					hranges,1,true);

	if (Debug == true)
		cout << "Back projection calculated" << endl;

	// Get Density Estimate between 0 and 1
	ProbMap.convertTo(ProbMap,-1,1/(double)numPixels,0);

	if (Debug == true)
		cout << "Division done" << endl;

	// Take Logs
	log(ProbMap, LogMap);

	if (Debug == true)
		cout << "Log calculated" << endl;

	return LogMap;
}


/* Inputs: InputImage, BasisFile
 * Output: Structure containing InfoMap, SalienceMap and ThreshMap
 */
AIM::IST AIM::GetSaliency(Mat InputImage, Mat Basis) {

	// How many pixels correspond to 1 degree visual angle
	double sigval = 8;

	// What size of window is needed to contain the above
	int ksize = 31; // In the MATLAB version it is 30x30 window. But OpenCV likes odd size kernels

	int NumColorChannels = InputImage.channels();
	int p = sqrt(basis.cols/3);
	
	int i,j = 0;

	if (Debug == true)
		cout << "p = " << p << endl;

	Mat basis_patch;
	Mat InputImageRGBColor = Mat::zeros(InputImage.rows, InputImage.cols, InputImage.type());
	std::vector <Mat> InputImageRGB(NumColorChannels);
	std::vector <Mat> OutputImageRGB(NumColorChannels);
	Mat InfoMap = Mat::zeros(InputImage.rows, InputImage.cols, CV_32FC1);
	Mat ConstImage = Mat::ones(InputImage.rows, InputImage.cols, CV_32FC1)*0.000001;

	cvtColor(InputImage,InputImageRGBColor,cv::COLOR_BGR2RGB);

	if (Debug == true)
		cout << "Color converted" << endl;

	split(InputImageRGBColor, InputImageRGB);

	int NumBasis = basis.rows;

	std::vector <Mat> BasisImage(NumBasis);
	
	for (i = 0; i < NumBasis; i++)
	{

	/**************************************************
	 * STEP 1 : Convolution of Basis Kernel with Input Image
	 ***************************************************/
		
		for (j = 0; j < NumColorChannels; j++)
		{
			Mat basis_row = Basis.operator ()(Range(i,i+1),Range(j*p*p,(j+1)*p*p));

			basis_patch = basis_row.reshape(1,p);

			if (Debug==true)
				cout << basis_patch.at<double>(0) << endl;
				BasisImage[i] = Mat::zeros(InputImage.rows,InputImage.cols,CV_32FC1);

			BasisImage[i] = Mat::zeros(InputImage.rows, InputImage.cols, Basis.type());

			OutputImageRGB[j] = Mat::zeros(InputImage.rows, InputImage.cols, Basis.type());
			OutputImageRGB[j] = GetConvolution2D(InputImageRGB[j], basis_patch);
			double minConvVal, maxConvVal;
			minMaxLoc(OutputImageRGB[j], &minConvVal, &maxConvVal, NULL, NULL);
			OutputImageRGB[j].convertTo(OutputImageRGB[j],-1, 255/maxConvVal,1);

			if (Debug == true)
			{
				cout << "Convolution took some time" << endl;
				//imshow("InputImageSingleChannel", InputImageRGB[j]);
				//imshow("Kernel", basis_patch);
				//imshow("ConvImage", OutputImageRGB[j]);
				//waitKey(0);
			}
			BasisImage[i] = BasisImage[i] + OutputImageRGB[j];
		}
		//imshow("BasisImage", BasisImage[i]);
		//waitKey(0);
	/**************************************************
   	 * STEP 2 : Get Density
     ***************************************************/
		Mat DenMap = GetDensity(BasisImage[i]);
		if (Debug == true)
			cout << "Density done" << endl;
	/**************************************************
     * STEP 3 : Convert to Log Likelihood
     ***************************************************/
    	// Don't know why we do this step but we do it
       	add(InfoMap, ConstImage, InfoMap);

    	subtract(InfoMap, DenMap, InfoMap);

    	if (Debug == true)
    		cout << "Step 2 and 3 done" << endl;
	}

	/**************************************************
	 * Post-Processing of Information Map
	 ***************************************************/
	double minVal, maxVal;

	minMaxLoc(InfoMap, &minVal, &maxVal, NULL, NULL);

	// Min values scaled to 0
	InfoMap = InfoMap - minVal;

	// Normalize
	InfoMap.convertTo(InfoMap,-1,1/(maxVal-minVal),0);

	Mat gker = getGaussianKernel(ksize,sigval, CV_32F);

	sepFilter2D(InfoMap, InfoMap, -1, gker, gker);   // separable convolution (speeds up execution)

	/**************************************************
	 * Pixel Masking
	 ***************************************************/
	std::vector <Mat> SaliencyMap (NumColorChannels);

	for (i = 0; i < NumColorChannels; i++)
	{
		SaliencyMap[i] = Mat::zeros(InputImage.rows, InputImage.cols, InputImage.type());
	}

	Mat InfomapLine = InfoMap.clone();
	Mat InfomapVector = InfomapLine.reshape(1,InfoMap.rows * InfoMap.cols);

	Mat ThreshMap = GetThreshMap(InfoMap, 0.8);

	// double disp_rank = CalcPercentile(InfomapVector, 0.8);

	Mat NormThreshMap;
	ThreshMap.convertTo(NormThreshMap, CV_8UC1, 1/double(255), 0);  // Just for Pixel Masking

	multiply(InputImageRGB[0],NormThreshMap,SaliencyMap[2]);  // R-R channel
	multiply(InputImageRGB[1],NormThreshMap,SaliencyMap[1]);  // G-G channel
	multiply(InputImageRGB[2],NormThreshMap,SaliencyMap[0]);  // B-B channel

	Mat SalienceMap;
	merge(SaliencyMap,SalienceMap);

	// Scale InfoMap to 8 bit precision
	//InfoMap.convertTo(InfoMap, CV_8UC3, 255, 0);
	// Only single channel needed - ska - 01/07/2015
	InfoMap.convertTo(InfoMap, CV_8UC1, 255, 0);
	/**************************************************
	 * Return Values
	 ***************************************************/
	ist.InfoImage = InfoMap;
	ist.SaliencyImage = SalienceMap;
	ist.ThreshImage = ThreshMap;

	return ist;
}


/* Inputs: sequence (1D vector), percentile (0 <= percentile <= 1)
 * Output: Percentile rank
 * Example Use : double n = Percentile (infomap, 0.98);
 * Refer : http://en.wikipedia.org/wiki/Percentile
 */
double AIM::CalcPercentile(Mat sequence, double percentile) {

	// Get the length of infomap
    double N = sequence.rows;

	// Sort the sequence
	cv::sort(sequence, sequence, SORT_EVERY_COLUMN + SORT_ASCENDING);

    // Calculate Rank (To check for extremes i.e. 0 and 1)
    double n1 = (N-1)*percentile + 1;

    // Calculate Rank (For linear interpolation)
    double n2 = (N*percentile) + 0.5;

    if (n1 == 1) return sequence.at<float>(0);

    else if (n1 == N)
    	{
    	return sequence.at<float>(N - 1);
    	}
    else  // Linear Interpolation
    {
         int k = (int)n1;  // round down to nearest integer

         double d = n2 - k;

         return sequence.at<float>(k - 1) + d * (sequence.at<float>(k) - sequence.at<float>(k - 1));
    }
}
