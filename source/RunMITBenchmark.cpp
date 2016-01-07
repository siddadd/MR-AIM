/*
 * RunMITBenchmark.cpp
 *
 *  Created on: Jan 7, 2016
 *      Author: siddharthadvani
 *
 * Comments: This code emulates MATLAB version of ICASSP 2013.
 * 			 Benchmarking on MIT Dataset
 */

#include <iostream>
#include <dirent.h>
#include <sys/stat.h>
#include <sys/types.h>

#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/features2d/features2d.hpp"

#include "RunMR_AIM.h"

using namespace cv;
using namespace std;

// Helper Function
static string toLowerCase(const string& in)
{
	string t;
	for (string::const_iterator i = in.begin(); i != in.end(); i++)
	{
		t += tolower(*i);
	}
	
	return t;
}

// MAIN
int main (int argc, char* argv[])
{
	///////////////////////////////
	//{ // Initialize variables
	//////////////////////////////

    if (argc < 3)
	{
		cout << "Usage: ./cvMR-AIM IN_DIR OUT_DIR <0>" << endl;
		return 0; 
	}
	String IN_DIR = argv[1];
	String OUT_DIR = argv[2];

	int num_pyramid_levels; 
	
	Mat InfoImage; // Output

	///////////////////////////////
	//} // Initialize variables
	//////////////////////////////


	///////////////////////////////
	//{ // Check OpenCV Version
	//////////////////////////////

	cout << "Using OpenCV version " << CV_VERSION << endl;

	///////////////////////////////
	//} // Check OpenCV Version 
	//////////////////////////////


	///////////////////////////////
	//{ // Set defaults for arguments
	//////////////////////////////

	String image_file_name;
	String out_file_name;

	if (argc == 4)
		num_pyramid_levels = 0;     // Baseline AIM
	else
		num_pyramid_levels = 3;    	// MR-AIM (3 levels)

	static vector <string> imgFileNames;
	static vector <string> outFileNames;

	static vector <string> validExtensions;
	validExtensions.push_back("jpg");
	validExtensions.push_back("png");

	///////////////////////////////
	//} // Set defaults for arguments
	//////////////////////////////

	///////////////////////////////
	//{ // Get Files
	//////////////////////////////

	cout << "Opening directory " << IN_DIR.c_str() << endl;
	struct dirent* ep;
	size_t extensionLocation;
	DIR* dp = opendir(IN_DIR.c_str());
	
	if (dp!=NULL)
	{
		while((ep = readdir(dp)))
		{
			// Ignore sub-directories
			if (ep->d_type & DT_DIR) 
			{
				continue;
			}
			
			extensionLocation = string(ep->d_name).find_last_of("."); // Assume the last point marks beginning of file extension

			// Check if extension matches the wanted one
			string tempExt = toLowerCase(string(ep->d_name).substr(extensionLocation + 1));
			if (find(validExtensions.begin(), validExtensions.end(), tempExt) != validExtensions.end())
			{
				cout << "[INFO]: Found matching data file " << ep->d_name << endl;
				imgFileNames.push_back(IN_DIR + ep->d_name);
				outFileNames.push_back(OUT_DIR + ep->d_name);
			}
			else 
				cout << "[INFO]: Found file does not match required file type, skipping: " << ep->d_name << endl;
		}
			
		(void) closedir (dp);
		cout << "[INFO]: Total " << imgFileNames.size() << " image files" << endl;
	}
	else
	{
		cout << "[ERROR]: Could not open directory: " << IN_DIR.c_str() << endl;
		return 0;
	}
	///////////////////////////////
	//} // Get Files
	//////////////////////////////
	

	///////////////////////////////
	//{ // MAIN
	//////////////////////////////

	for (int fn = 0; fn < imgFileNames.size(); fn++)
	{
		image_file_name = imgFileNames.at(fn);
		out_file_name = outFileNames.at(fn);

		cout << "[INFO]: Processing " << image_file_name << endl;

		InfoImage = RunMR_AIM(image_file_name, num_pyramid_levels);

		// check if output directory exists, if not make it
		DIR* od = opendir(OUT_DIR.c_str());
		if (od == NULL)
		{
			cout << "[INFO]: Output Directory does not exist. Creating it" << endl;
			mkdir(OUT_DIR.c_str(), 0777);
		}		
		(void) closedir (od);
	
		try
		{
			imwrite(out_file_name, InfoImage);
		}
		catch (runtime_error& ex) 
		{	
			fprintf(stderr, "[ERROR]: Unable to write image %s\n", ex.what());
			return 1;
		}

		fprintf(stdout, "[INFO]: Saved output infomap %s\n", out_file_name.c_str());
	}

	///////////////////////////////
	//} // MAIN
	//////////////////////////////
	
	return 0;
}

