#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "sp_image_proc_util.h"
#include "main_aux.h"
#define EXIT_MSG "Exiting...\n"
#define ALLOCATION_FAILURE_MSG "An error occurred - allocation failure\n"

int main()
	{


	char dir[10] = "./images/";
	char prefix[4] = "img";
        char suffix[5] =".png";
        char query[11]="queryA.png";
int number_of_images = 17;
int number_of_bins = 16;
int max_number_of_features = 100;
	//char dir[1024]; char prefix[1024]; char suffix[1024]; char query[1024];
	//int number_of_images; int number_of_bins; int max_number_of_features;
	int i; int j;
	int best_n_features = 5;
	char image_full_path[1024];
	int actual_num_of_features;
	int ***RGB_hist_per_image;                       // array of pointers to the RGB histogram of each image
	double *** SIFT_descriptors_per_image;           // array of pointers to the sift descriptors of each image
	int** RGB_hist_of_query;                         // the RGB histogram of the query image
	double** sift_descriptors_of_query;              // the sift descriptors of the query image
	double* arr_of_RGB_distances;
	int* nearest_images_global;
	int* nearest_images_local;
	double* mach_feature_cnt;
	int* feature_square_distance_result;
	int* nFeaturesPerImage;
	int nFeaturesOfQuery;

/*
	printf("Enter images directory path:\n");
	fflush(NULL);
	if (scanf("%s",dir) == 0){
		return 1;
	}

	printf("(%s)\n", dir);
	fflush(NULL);
	printf("Enter images prefix:\n");
	fflush(NULL);
	if (scanf("%s",prefix) == 0){
		return 1;
	}

	printf("Enter number of images:\n");
	fflush(NULL);
	if (scanf("%d",&number_of_images) == 0){
		return 1;
	}
	if (number_of_images < 1) {
		printf("An error occurred - invalid number of images\n");
		fflush(NULL);
		return 1;
	}

	printf("Enter images suffix:\n");
	fflush(NULL);
	if (scanf("%s", suffix) == 0){
		return 1;
	}

	printf("Enter number of bins:\n");
	fflush(NULL);
	if (scanf("%d", &number_of_bins) == 0){
		return 1;
	}

	if (number_of_bins < 1){
		printf("An error occurred - invalid number of bins\n");
		fflush(NULL);
		return 1;
	}

	printf("Enter number of features:\n");
	fflush(NULL);
	if (scanf("%d", &max_number_of_features) == 0){
		return 1;
	}
	fflush(NULL);

	if (max_number_of_features < 1){
		printf("An error occurred - invalid number of features\n");
		fflush(NULL);
		return 1;
	}
*/
	//allocate memory for RGB_hist_per_image and SIFT_descriptors_per_image
	if ((RGB_hist_per_image = (int***)malloc(number_of_images*sizeof(int**))) == NULL) {
		printf(ALLOCATION_FAILURE_MSG);
		fflush(NULL);
		return 1; 
	}
	if ((SIFT_descriptors_per_image = (double***)malloc(number_of_images*sizeof(double**))) == NULL) {
		free(RGB_hist_per_image);
		printf(ALLOCATION_FAILURE_MSG);
		fflush(NULL);
		return 1; 
	}

	// store all the RGB histograms and all the features of all images from the directory given by the user
	for (i=0; i< number_of_images; i++) {
		sprintf(image_full_path, "%s%s%d%s", dir, prefix , i ,suffix );
		RGB_hist_per_image[i] = spGetRGBHist(image_full_path, number_of_bins);
		SIFT_descriptors_per_image[i] = spGetSiftDescriptors(image_full_path, max_number_of_features, &actual_num_of_features);
	}

	//allocate memory for nFeaturesPerImage and check allocation
	if ((nFeaturesPerImage = (int*)malloc(number_of_images*sizeof(int))) == NULL) {
		printf(ALLOCATION_FAILURE_MSG);
		fflush(NULL);
		return 1;
	}

	// nFeaturesPerImage[i] is the number of rows in SIFT_descriptors_per_image[i] i.e the number of features for image i
	for (i=0; i<number_of_images; i++){
		nFeaturesPerImage[i] = sizeof SIFT_descriptors_per_image[i] / sizeof SIFT_descriptors_per_image[i][0];
	}

	while (1){
		/*
		printf("Enter a query image or # to terminate:\n");
		fflush(NULL);
		// TODO: what happens here?
		if (scanf("%s",query) == 0){
			free(RGB_hist_per_image);
			free(SIFT_descriptors_per_image);
			return 1;
		}
		// the user requested termination
		if (strcmp (query,"#") == 0) {
			printf(EXIT_MSG);
			fflush(NULL);
			free(RGB_hist_per_image);
			free(SIFT_descriptors_per_image);
			return 1;
		}
*/
		//calculate the RGB histogram and sift descriptors of the query image
		RGB_hist_of_query = spGetRGBHist(query, number_of_bins);
		sift_descriptors_of_query = spGetSiftDescriptors(query, max_number_of_features, &actual_num_of_features);

		// count the number of features of query image
		nFeaturesOfQuery = sizeof sift_descriptors_of_query / sizeof sift_descriptors_of_query[0];

		//allocate memory for arr_of_RGB_distances
		arr_of_RGB_distances = (double*)malloc(number_of_images*sizeof(double));
		if (arr_of_RGB_distances == NULL) {
			printf(ALLOCATION_FAILURE_MSG);
			fflush(NULL);
			free(RGB_hist_per_image);
			free(SIFT_descriptors_per_image);
			free(RGB_hist_of_query);
			free(sift_descriptors_of_query);
			return 1;
		}

		// calculate the L2-Squared distances between the query image histogram and the histograms of each image and store them in arr_of_RGB_distances
		for (i=0; i<number_of_images; i++){
			arr_of_RGB_distances[i] = spRGBHistL2Distance(RGB_hist_per_image[i], RGB_hist_of_query, number_of_bins);
		}

		// find the nearest images using global and report to user
		nearest_images_global = nearestImages(arr_of_RGB_distances, number_of_images, best_n_features);
		printf("Nearest images using global descriptors:\n");
		printf("%d, %d, %d, %d, %d\n", nearest_images_global[0],nearest_images_global[1],nearest_images_global[2],nearest_images_global[3],nearest_images_global[4]);
		fflush(NULL);

		// free memory of nearest global that is no longer needed
		free(RGB_hist_of_query);
		free(arr_of_RGB_distances);
		free(nearest_images_global);

		// allocate memory for mach_feature_cnt and set all cells to zero because were using this array as a counter array
		mach_feature_cnt = (double*)calloc(number_of_images, sizeof(double)); //TODO should allocate BEST_N_FEATURES *max_number_of_features and not number_of_images
		if (mach_feature_cnt==NULL){
			printf("An 6666error occurred - allocation failure\n"); //TODO: switch to ALLOCATION_FAILURE_MSG
			fflush(NULL);
			free(RGB_hist_per_image);
			free(SIFT_descriptors_per_image);
			free(RGB_hist_of_query);
			free(sift_descriptors_of_query);
			free(arr_of_RGB_distances);
			free(nearest_images_global);
			return 1;
		}

		// for each feature of query calculate the spBestSIFTL2SquaredDistance
		for (i=0; i<nFeaturesOfQuery; i++){
printf("reut1 \n");
			feature_square_distance_result = spBestSIFTL2SquaredDistance(best_n_features, sift_descriptors_of_query[i],
				SIFT_descriptors_per_image, number_of_images, nFeaturesPerImage);
			//add the results from spBestSIFTL2SquaredDistance to the (minus) counting array
			for (j=0; j<best_n_features; j++){
				printf("%d",j);
				fflush(NULL);
				mach_feature_cnt[feature_square_distance_result[j]]--; 
			}
		}
printf("reut2\n");
		// find the nearest images using local and report to user
		nearest_images_local = nearestImages(mach_feature_cnt, number_of_images, best_n_features);
		printf("Nearest images using local descriptors:\n");
		printf("%d, %d, %d, %d, %d\n", nearest_images_local[0],nearest_images_local[1], nearest_images_local[2],nearest_images_local[3], nearest_images_local[4]);
		fflush(NULL);

		// free memory allocated inside while loop (that wasn't already freed)
		free(sift_descriptors_of_query);
		free(feature_square_distance_result);
		free(mach_feature_cnt);
		free(nFeaturesPerImage);
	
	}

	// free memory allocated outside while loop
	free(RGB_hist_per_image);
	free(SIFT_descriptors_per_image);
	return 0;

}


