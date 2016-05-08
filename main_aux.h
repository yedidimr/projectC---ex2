
/** given an array of doubles returns an array containing the indexes of the
 *  lowest nearestNImages numbers from the original array
  * @param arr - array of L2-Squared distances 
  * @param size - the size of array arr
  * @param nearestNImages the number of the best (closest) distances to find
  */
int* nearestImages (double* arr, int size, int nearestNImages);


/*
* this function is called from qsort and it sorts array which each cell contains array of length two
* this function sorts in ascending order by the first element of each cell.
* if two elements are equal it sorts by the second element of each cell.
*
* this function is useful to sort array of doubles by ascending order, in case there are two elements
* that are equal, then the one with the smallest index will be the smaller one.
* to do so create a new array where it's first row is your original array and it's second row is the indexes of each element
* and sort it with this function.
*
* @param a - array of doubles of size two.
* @param b - array of doubles of size two.
* @return 1 if a is bigger than b
*		  -1 if b is bigger than a
*		  0 if a is equal to be (both the first and second elelments)
*

*/
int compare(const void *a, const void * b);
