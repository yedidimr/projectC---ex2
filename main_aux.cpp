#include <stdlib.h>
#include <stdio.h>
#include "main_aux.h"

#define ALLOCATION_FAILURE_MSG "An error occurred - allocation failure\n"

int* nearestImages (double* arr, int size, int nearestNImages){
	/*
	* in order to return lowest nearestNImages numbers from arr, this function sorts arr using qsort and keeping indexes order
	* arr_with_indixes - will hold the matrix of dimensions size X 2 where first row is a copy of arr and second row is the indexes of each value in arr
	*					  which will be sorted
	*/
	double** arr_with_indixes;	// doc above
	int* first_minimums; 		// the nearestNImages indexes that are the minimum values
	int i;

	// if asked to find miminums more than the values in the array, return NULL
	if (nearestNImages > size) {
		return NULL;
	}

	// allocate array of minimums (size of nearestNImages)
	first_minimums = (int*) malloc(nearestNImages*sizeof(int));
	if (first_minimums == NULL){
		printf(ALLOCATION_FAILURE_MSG);
		fflush(NULL);
		return NULL;
	}

	if ((arr_with_indixes = (double**)malloc(size*sizeof(double*))) == NULL) {
		printf(ALLOCATION_FAILURE_MSG);
		free(first_minimums);
		fflush(NULL);
		return NULL;
	}

	for (i = 0; i < size; i++){
		if ((arr_with_indixes[i] = (double *)malloc(2*sizeof(double))) == NULL ) {
			printf(ALLOCATION_FAILURE_MSG);
			free(first_minimums);
			free(arr_with_indixes);
			fflush(NULL);
			return NULL;
		}
	}

	// initialize arr_with_indexes
	for(i = 0; i < size; i++){
		arr_with_indixes[i][0] = arr[i];	// copy value from arr
		arr_with_indixes[i][1] = i;			// set index before sorting
	}

	// sort
	qsort(arr_with_indixes, size, sizeof(double*), compare);

	// copy nearestNImages smallest values' indexes
	for (i=0; i<nearestNImages; i++) {
		first_minimums[i] = (int)arr_with_indixes[i][1];
	}

	free(arr_with_indixes);
	return first_minimums;
}



double max_of_array (double* arr, int size){
	double max = arr[0]; //default max val
	int i;
	//loop through arr to find max
	for (i=0; i<size; i++){
		if (arr[i]>max){
			max = arr[i];
		}
	}
	return 	max;
}





