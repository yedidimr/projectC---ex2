#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "sp_image_proc_util.h"
#include "main_aux.h"
#define ALLOCATION_FAILURE_MSG "An error occurred - allocation failure\n"
#define ENTER_IMAGES_DIR_MSG "Enter images directory path:\n"
#define INVALID_IMG_NUM_MSG "An error occurred - invalid number of images\n"
#define ENTER_IMG_PREFIX_MSG "Enter images prefix:\n"
#define ENTER_IMG_NUM_MSG "Enter number of images:\n"
#define ENTER_IMG_SUFFIX_MSG "Enter images suffix:\n"
#define ENTER_NUM_OF_BINS_MSG "Enter number of bins:\n"
#define INVALID_NUM_OF_BIN_ERR_MSG "An error occurred - invalid number of bins\n"
#define ENTER_NUM_OF_FEATURES_MSG "Enter number of features:\n"
#define INVALID_NUM_OF_FEATURES_MSG "An error occurred - invalid number of features\n"
#define ENTER_QUERY_IMG_MSG "Enter a query image or # to terminate:\n"
#define NEAREST_GLOBAL_IMG_MSG "Nearest images using global descriptors:\n"
#define NEAREST_LOCAL_IMG_MSG  "Nearest images using local descriptors:\n"
#define EXIT_MSG "Exiting...\n"
#define EXIT_QUERY "#"
#define  MAX_STR_SIZE 1024	
#define SUCCESS 0
#define FAILURE 1
#define BEST_FEATURES_COUNT 5

int main()
	{
	// user input params
	char dir[MAX_STR_SIZE];
	char prefix[MAX_STR_SIZE];
	char suffix[MAX_STR_SIZE];
	char query[MAX_STR_SIZE];
	int number_of_images;
	int number_of_bins;
	int max_number_of_features;
	
	char image_full_path[MAX_STR_SIZE];				// full path to image
	int actual_num_of_features = 0;					// number of features per img
	int*** RGB_hist_per_image;                       // array of pointers to the RGB histogram of each image
	double*** SIFT_descriptors_per_image;            // array of pointers to the sift descriptors of each image
	int** RGB_hist_of_query;                         // the RGB histogram of the query image
	double** sift_descriptors_of_query;              // the sift descriptors of the query image
	double* arr_of_RGB_distances;                    // holds the L2-Squared distances between the query image histogram and the histograms of each image
	int* nearest_images_global;                      // holds indexes of nearest images globally - will be returned to user
	int* nearest_images_local;                       // holds indexes of nearest images locally - will be returned to user
	double* mach_feature_cnt;                        // holds the count of hits per image
	int* feature_square_distance_result;             // holds result of the call to spBestSIFTL2SquaredDistance
	int * num_of_features_per_image;                 // holds the number of features each image has

	int error = SUCCESS; // indicates whether an error occurred inside while loop
	int i; int j; int k;

	// get input from user
	printf(ENTER_IMAGES_DIR_MSG);
	fflush(NULL);
	if (scanf("%s",dir) == 0){
		return FAILURE;
	}

	fflush(NULL);
	printf(ENTER_IMG_PREFIX_MSG);
	fflush(NULL);
	if (scanf("%s",prefix) == 0){
		return FAILURE;
	}

	printf(ENTER_IMG_NUM_MSG);
	fflush(NULL);
	if (scanf("%d",&number_of_images) == 0){
		return FAILURE;
	}
	if (number_of_images < 1) {
		printf(INVALID_IMG_NUM_MSG);
		fflush(NULL);
		return FAILURE;
	}

	printf(ENTER_IMG_SUFFIX_MSG);
	fflush(NULL);
	if (scanf("%s", suffix) == 0){
		return FAILURE;
	}

	printf(ENTER_NUM_OF_BINS_MSG);
	fflush(NULL);
	if (scanf("%d", &number_of_bins) == 0){
		return FAILURE;
	}

	if (number_of_bins < 1){
		printf(INVALID_NUM_OF_BIN_ERR_MSG);
		fflush(NULL);
		return FAILURE;
	}
	printf(ENTER_NUM_OF_FEATURES_MSG);
	fflush(NULL);
	if (scanf("%d", &max_number_of_features) == 0){
		return FAILURE;
	}
	fflush(NULL);

	if (max_number_of_features < 1){
		printf(INVALID_NUM_OF_FEATURES_MSG);
		fflush(NULL);
		return FAILURE;
	}

	// allocate memory for RGB_hist_per_image
	if ((RGB_hist_per_image = (int***)malloc(number_of_images*sizeof(int**))) == NULL) {
		printf(ALLOCATION_FAILURE_MSG);
		fflush(NULL);
		return FAILURE; 
	}

	// allocate memory for SIFT_descriptors_per_image
	if ((SIFT_descriptors_per_image = (double***)malloc(number_of_images*sizeof(double**))) == NULL) {
		free(RGB_hist_per_image);
		printf(ALLOCATION_FAILURE_MSG);
		fflush(NULL);
		return FAILURE; 
	}

	// allocate memory for num_of_features_per_image
	num_of_features_per_image = (int *)malloc(number_of_images*sizeof(int));
    if (num_of_features_per_image == NULL) { 
        free(RGB_hist_per_image);
        free(SIFT_descriptors_per_image);
        printf(ALLOCATION_FAILURE_MSG);
        fflush(NULL);
        return FAILURE;
    }


	// store all the RGB histograms and all the features of all images from the directory given by the user
	for (i=0; i< number_of_images; i++) {
		sprintf(image_full_path, "%s%s%d%s", dir, prefix , i ,suffix );
		RGB_hist_per_image[i] = spGetRGBHist(image_full_path, number_of_bins);
		SIFT_descriptors_per_image[i] = spGetSiftDescriptors(image_full_path, max_number_of_features, &(num_of_features_per_image[i]));
		
		// if there was an error - free allocated memory and return
		if (SIFT_descriptors_per_image[i] == NULL || RGB_hist_per_image[i] == NULL) {
			for(j=0;j<i;j++) {
				free(RGB_hist_per_image[j]);
				free(SIFT_descriptors_per_image[j]);
			}
			free(SIFT_descriptors_per_image);
			free(RGB_hist_per_image);
			return FAILURE;
		}
	}

	// run while there was no error and while user didn't ask to exit
	while (!error){

		printf(ENTER_QUERY_IMG_MSG);
		fflush(NULL);
		if (scanf("%s",query) == 0){
			free(RGB_hist_per_image);
			free(SIFT_descriptors_per_image);
			return FAILURE;
		}
		
		// the user requested to exit
		if (strcmp (query,EXIT_QUERY) == 0) {
			printf(EXIT_MSG);
			fflush(NULL);
			break;
		}

		//calculate the RGB histogram and sift descriptors of the query image
		RGB_hist_of_query = spGetRGBHist(query, number_of_bins);
		sift_descriptors_of_query = spGetSiftDescriptors(query, max_number_of_features, &actual_num_of_features);
		error = (sift_descriptors_of_query == NULL || RGB_hist_of_query == NULL);
		if (error) {
			break;
		}

		//allocate memory for arr_of_RGB_distances
		arr_of_RGB_distances = (double*)malloc(number_of_images*sizeof(double));
		error = (arr_of_RGB_distances == NULL);
		if (error) {
			break;
		}

		// calculate the L2-Squared distances between the query image histogram and the histograms of each image and store them in arr_of_RGB_distances
		for (i=0; i<number_of_images; i++){
			arr_of_RGB_distances[i] = spRGBHistL2Distance(RGB_hist_per_image[i], RGB_hist_of_query, number_of_bins);
			error = (arr_of_RGB_distances[i] == -1);
			if (error) {
				break;
			} 
		}

		// find the nearest images using global and report to user
		nearest_images_global = nearestImages(arr_of_RGB_distances, number_of_images, BEST_FEATURES_COUNT);
		error = (nearest_images_global == NULL);
		if (error) {
			break;
		} 
		printf(NEAREST_GLOBAL_IMG_MSG);
		printf("%d, %d, %d, %d, %d\n", nearest_images_global[0],nearest_images_global[1],nearest_images_global[2],nearest_images_global[3],nearest_images_global[4]);
		fflush(NULL);
		
		// free memory allocated for global inside while loop
		free(RGB_hist_of_query);
		free(arr_of_RGB_distances);
		free(nearest_images_global);

		// allocate memory for mach_feature_cnt and set all cells to zero because were using this array as a counter array
		mach_feature_cnt = (double*)calloc(number_of_images, sizeof(double));
		error = (mach_feature_cnt==NULL);
		if (error) {
			break;
		}

		// for each feature of query calculate the spBestSIFTL2SquaredDistance
		for (i=0; i<actual_num_of_features; i++){
			feature_square_distance_result = spBestSIFTL2SquaredDistance(BEST_FEATURES_COUNT, sift_descriptors_of_query[i],
				SIFT_descriptors_per_image, number_of_images, num_of_features_per_image);

			error = (feature_square_distance_result == NULL);
			if (error) {
				break;
			}
			// Counting hits
	        for (k = 0; k < number_of_images; k++) {
	            for (j = 0; j < 5; j++) {
	                if (feature_square_distance_result[j] == k) {
	                    mach_feature_cnt[k]--;
	                }
	            }
	        }
		}

		// find the nearest images using local and report to user
		nearest_images_local = nearestImages(mach_feature_cnt, number_of_images, BEST_FEATURES_COUNT);
		error = (nearest_images_local == NULL);
		if (error) {
			break;
		} 
		printf(NEAREST_LOCAL_IMG_MSG);

		printf("%d, %d, %d, %d, %d\n", nearest_images_local[0],nearest_images_local[1], nearest_images_local[2],nearest_images_local[3], nearest_images_local[4]);
		fflush(NULL);

		// free memory allocated for local inside while loop
		free(sift_descriptors_of_query);
		free(feature_square_distance_result);
		free(mach_feature_cnt);
		free(nearest_images_local);

	}

	// in case of error inside while - free memory that wasn't freed
	if (error) {
		if (sift_descriptors_of_query != NULL) free(sift_descriptors_of_query);
		if (RGB_hist_of_query != NULL) free(RGB_hist_of_query);
		if (feature_square_distance_result != NULL) free(feature_square_distance_result);
		if (mach_feature_cnt != NULL) free(mach_feature_cnt);
		if (arr_of_RGB_distances != NULL) free(arr_of_RGB_distances);
		if (nearest_images_global != NULL) free(nearest_images_global);
		if (nearest_images_local != NULL) free(nearest_images_local);
	}


	// free memory allocated outside while loop
	for (i=0; i< number_of_images; i++) {
		free(RGB_hist_per_image[i]);
		free(SIFT_descriptors_per_image[i]);
	}
	free(RGB_hist_per_image);
	free(SIFT_descriptors_per_image);
	free(num_of_features_per_image);

	return error ? FAILURE : SUCCESS;

}


