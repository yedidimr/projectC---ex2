#include <stdlib.h>
#include <stdio.h>
#include "sp_image_proc_util.h"
#include <opencv2/imgproc.hpp> //calcHist
#include <opencv2/core.hpp> //Mat
#include <opencv2/highgui.hpp> //imshow, drawKeypoints, waitKey
#include <opencv2/xfeatures2d.hpp>//SiftDescriptorExtractor
#include <opencv2/features2d.hpp>
#include <iostream>
#include <vector>

#define RGB_CHANNELS 3 // number of channels in RGB format
#define ONE_RGB_CHANNEL_PERCENTAGE 0.33 // one channel from RGB_CHANNELS (percentage)
#define R_INDEX 0 // R channel index
#define G_INDEX 1 // G channel index
#define B_INDEX 2 // B channel index
#define ALLOCATION_FAILURE_MSG "An error occurred - allocation failure\n"

#define HIST_BINS 128 // TODO change name

using namespace cv;
using namespace std;


int** spGetRGBHist(char* str, int nBins){
	int** histMat; 					      // the array that will store the histograms and will be the output
	float range[] = { 0, 256 };
	const float* histRange = { range };
	Mat r_hist, g_hist, b_hist;
	Mat image;
	int i;
	vector<Mat> rgb_planes;

	// input validation
	if ((str==NULL) | (nBins<=0)){
		return NULL;
	}

	// load image and make sure it exists
	image = imread(str, CV_LOAD_IMAGE_COLOR);
	if (image.empty()) { 			// if the image doesn't exist TODO is this needed? - REUT:  I think that if they didn't say to validate this it is not required, we can check in ,Elisheva:in the .h file it says to check this for the second func but to to this func.
			return NULL;
	}

	// split the image to the three planes (red,green,blue)
	split(image, rgb_planes);

	// calculate the histograms for each color, the results are stored in b_hist, g_hist, r_hist, output type of the matrices is CV_32F (float)
	calcHist( &rgb_planes[0], 1, 0, Mat(), b_hist, 1, &nBins, &histRange);
	calcHist( &rgb_planes[1], 1, 0, Mat(), g_hist, 1, &nBins, &histRange);
	calcHist( &rgb_planes[2], 1, 0, Mat(), r_hist, 1, &nBins, &histRange);

	// allocate memory for the array hisMat
	histMat = (int**) malloc(RGB_CHANNELS * sizeof(int*));
	if (histMat==NULL){ // check for allocation error
		printf(ALLOCATION_FAILURE_MSG);
		fflush(NULL);
		return NULL;
	}
	for (i = 0; i < RGB_CHANNELS; i++){ 
		histMat[i] = (int*) malloc(nBins * sizeof(int));
		if (histMat[i] == NULL) {
			printf(ALLOCATION_FAILURE_MSG);
			fflush(NULL);
			free(histMat);
			return NULL;
		}
	}

	//put the data from the three Mat type histograms into one 2D array
	for (i = 0; i < nBins; i++){
		histMat[R_INDEX][i] = (int)r_hist.at<float>(i);
	}
	for (i = 0; i < nBins; i++){
			histMat[G_INDEX][i] = (int)g_hist.at<float>(i);
	}
	for (i = 0; i < nBins; i++){
			histMat[B_INDEX][i] = (int)b_hist.at<float>(i);
	}

	return histMat;
}


double spRGBHistL2Distance(int** histA, int** histB, int nBins) {
	double pair_dist = 0;  // will hold the distance between each bin 
	double row_count = 0;  // temp argument: distance of channel i in histA and channel i in histB
	double total_dist = 0; // distance between histA and histB
	int r,c;

	//return -1 if nBins <= 0 or histA/histB is null
	if ((nBins <= 0) | !histB | !histA) {
		return -1;
	}

	// calculate distance
	for (r = 0; r < RGB_CHANNELS; r++) {
		for (c = 0; c < nBins; c++) {
			pair_dist = (double)histA[r][c] - (double)histB[r][c];
			row_count = row_count + (pair_dist*pair_dist); // count distance of channels
		}
		total_dist = total_dist + (ONE_RGB_CHANNEL_PERCENTAGE * row_count); // add distance of channel to total distance
		row_count = 0;
	}
	return total_dist;
}


double** spGetSiftDescriptors(char* str, int maxNFeautres, int *nFeatures){
	double** feature_matrix;              					// the array that will be the output
	std::vector<cv::KeyPoint> keyPoints;  					//Key points will be stored in keyPoints;
	cv::Mat FeatureValues;               					//Feature values will be stored in FeatureValues;
	Mat image;
	int i; int j;
	cv::Ptr<cv::xfeatures2d::SiftDescriptorExtractor> detect = cv::xfeatures2d::SIFT::create(maxNFeautres); //Creating  a Sift Descriptor extractor

	//input validation
	if ((str==NULL) | (nFeatures==NULL) | (maxNFeautres <= 0)){
		return NULL;
	}

	// load image and make sure it exists
	image = imread(str, CV_LOAD_IMAGE_GRAYSCALE);
	if (image.empty()) {
			return NULL;
	}

	//Extracting features, they  will be stored in FeatureValues
	detect->detect(image, keyPoints, cv::Mat());
	detect->compute(image, keyPoints, FeatureValues);

	//nFeatures is how many features were actually extracted
	*nFeatures = FeatureValues.rows;

	//allocate memory for feature_matrix
	feature_matrix = (double**) malloc(*nFeatures * sizeof(double*));
	if (feature_matrix==NULL){
		printf(ALLOCATION_FAILURE_MSG);
		fflush(NULL);
		return NULL;
	}
	for (i = 0; i < *nFeatures; i++){
		feature_matrix[i] = (double *) malloc(HIST_BINS * sizeof(double));
		if (feature_matrix[i] == NULL){
			printf(ALLOCATION_FAILURE_MSG);
			fflush(NULL);
			free(feature_matrix);
			return NULL;
		}
	}

	//put the feature data in feature_matrix
	for (i=0; i<FeatureValues.rows; i++){
		for (j=0; j<FeatureValues.cols; j++){
			feature_matrix[i][j]=(double)FeatureValues.at<float>(0,j); //TODO at(i,j)?
		}
	}

	return feature_matrix;
}


double spL2SquaredDistance(double* featureA, double* featureB){
	double pair_dist = 0;
	double total_dist = 0;
	int i;

	//return -1 in case featureA or featureB is NULL
	if (!featureB | !featureA) {
		return -1;
	}

	// calculate distance
	for (i = 0; i < HIST_BINS; i++) {
		pair_dist = featureA[i] - featureB[i];
		total_dist += pair_dist * pair_dist;
	}
	return total_dist;
}


//TODO add description of this helper function
int compare(const void *a, const void * b) {
	double* p1 = *(double**) a;
	double* p2 = *(double**) b;
	
	if (p1[0] > p2[0]) {
		return 1;
	}

	else if (p1[0] < p2[0]) {
		return -1;
	}

	else { // p1[0] == p2[0]
		if (p1[1] > p2[1]) {
			return 1;
		}

		else if (p1[1] < p2[1]) {
			return -1;
		}
		else {
			return 0;

		}
	}

}


int* spBestSIFTL2SquaredDistance(int bestNFeatures, double* featureA,
		double*** databaseFeatures, int numberOfImages,
		int* nFeaturesPerImage) {

	int image, feature; 			// for iteration
	double **all_features_dist; 	// will hold distance and image index for every feature
	double current_dist; 			// will hold the current features distance
	int total_num_of_features = 0;  // amount of all features in all images
	int feature_counter = 0; 		// count number of features passed
	int * best_n_features_images;   // will hold bestNFeatures images indexes
	
	// return NULL if one of the parameters is not initialized
	if (!featureA | !databaseFeatures | (numberOfImages <=1) | !nFeaturesPerImage) {
		return NULL;
	}

	// find total amount of features (including all images)
	for (image = 0; image < numberOfImages; image++) {
		total_num_of_features = total_num_of_features+ nFeaturesPerImage[image];
	}

	// allocate dynamic memory and return NULL if allocation fails
	if ((all_features_dist = (double**)malloc(total_num_of_features*sizeof(double*))) == NULL) {
		printf(ALLOCATION_FAILURE_MSG);
		fflush(NULL);
		return NULL;
	}

	for (feature = 0; feature < total_num_of_features; feature++){
		if ((all_features_dist[feature] = (double *)malloc(2*sizeof(double))) == NULL ) {
			printf(ALLOCATION_FAILURE_MSG);
			fflush(NULL);
			free(all_features_dist);
			return NULL;
		}
	}

	if ((best_n_features_images = (int *)malloc(bestNFeatures*sizeof(int))) == NULL ){
		printf(ALLOCATION_FAILURE_MSG);
		free(all_features_dist);
		fflush(NULL);
		return NULL;
	}


	// calc distance of each feature with featureA
	for (image = 0; image < numberOfImages; image++) {
		for (feature = 0; feature < nFeaturesPerImage[image]; feature++) {

			current_dist = spL2SquaredDistance(featureA, databaseFeatures[image][feature]);
			// save distance (at index 0) and image index (at index 1) for every feature
			all_features_dist[feature_counter][0] = current_dist;
			all_features_dist[feature_counter][1] = (double)image;
			feature_counter++;
		}
	}
	// now feature_counter is equal to total_num_of_features
	if (feature_counter != total_num_of_features) //TODO REUT - remove before submission
	{
		printf("REUT FAIL %d %d", feature_counter, total_num_of_features);
		fflush(NULL);
		return NULL;		
	}

	// sort by distances (from lowest to highest) and keep images indexes sorted too
	qsort(all_features_dist, total_num_of_features, sizeof(double*), compare);

	// take only bestNFeatures first features images indexes 
	for (feature = 0; feature < bestNFeatures; feature++) {
		best_n_features_images[feature] = (int)all_features_dist[feature][1];
	}

	free(all_features_dist); // free unused memory
	return best_n_features_images;
}


