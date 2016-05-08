
/** given an array of doubles returns an array containing the indexes of the
 *  lowest nearestNImages numbers from the original array
  * @param arr - array of L2-Squared distances 
  * @param size - the size of array arr
  * @param nearestNImages the number of the best (closest) distances to find
  */
int* nearestImages (double* arr, int size, int nearestNImages);


/**
* returns maximum value of the array
* @param arr - array to find the maximum of
* @param size - the size of array arr
*/
int compare(const void *a, const void * b) ;//TODO: not sure where this should be mentioned - asked in forum.
