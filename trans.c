/* 
 * trans.c - Matrix transpose B = A^T
 *
 * Each transpose function must have a prototype of the form:
 * void trans(int M, int N, int A[N][M], int B[M][N]);
 *
 * A transpose function is evaluated by counting the number of misses
 * on a 1KB direct mapped cache with a block size of 32 bytes.
 */ 
#include <stdio.h>
#include "cachelab.h"

int is_transpose(int M, int N, int A[N][M], int B[M][N]);

/* 
 * transpose_submit - This is the solution transpose function that you
 *     will be graded on for Part B of the assignment. Do not change
 *     the description string "Transpose submission", as the driver
 *     searches for that string to identify the transpose function to
 *     be graded. 
 */
char transpose_submit_desc[] = "Transpose submission";
void transpose_submit(int M, int N, int A[N][M], int B[M][N])
{

int i, j, k, l, blocksize;

 if(M == 32) {
  
  // Submatrix size of 8X8 gives the best cache performance
  blocksize = 8;

  // Loop through all of the submatricies horizontally
  for(i = 0; i < N; i+= blocksize) {

    // Loop through all of the submatricies vertically
    for(j = 0; j < M; j+=blocksize) {

      //Loop through all of the rows in each submatrix
      for(k = i; k < i + blocksize; k++) {

        // Loop through all of the columns in the submatrix
        for(l = j; l < j + blocksize; l++) {
          B[l][k] = A[k][l];
        }
      }
    }
  }
 } else if(M == 64) {
  
  // Sub matrix size of 4X4 gives the best cache performance
  blocksize = 4;
  
  //Loop through M/blocksize columns
  for(i = 0; i < M; i+= blocksize) {

    // Loop through N/blocksize rows
    for(j = 0; j < N; j+=blocksize) {

      // Loop through matrix row starting at first index in submatrix and ending at first index + blocksize
      for(k = j; k < j + blocksize; k++) {

        // Loop through matrix columns starting at first index in submatrix and ending at first index + blocksize
	for(l = i; l < i + blocksize; l++) {
	  B[l][k] = A[k][l];
        }
      }
    }
  } 
 } else if(M == 61) {

   // block size of 20 gives best cache performance
   blocksize = 20;  

    // Loop through all of the rows incrementing i by the blocksize
    for (i = 0; i < N; i += blocksize) {

	// Loop through all of the Columns
        for(j = 0; j < M; j++) {

	    // Keep looping until k is larger than blocksize and k + i is greater than or equal to the row size
            for(k = 0; (k < blocksize) && (i + k < N); ++k) {
                B[j][i + k] = A[i + k][j];
            }
        }
    }
}
}
/* 
 * You can define additional transpose functions below. We've defined
 * a simple one below to help you get started. 
 */ 

/* 
 * trans - A simple baseline transpose function, not optimized for the cache.
 */
char trans_desc[] = "Simple row-wise scan transpose";
void trans(int M, int N, int A[N][M], int B[M][N])
{
    int i, j, tmp;

    for (i = 0; i < N; i++) {
        for (j = 0; j < M; j++) {
            tmp = A[i][j];
            B[j][i] = tmp;
        }
    }    

}

/*
 * registerFunctions - This function registers your transpose
 *     functions with the driver.  At runtime, the driver will
 *     evaluate each of the registered functions and summarize their
 *     performance. This is a handy way to experiment with different
 *     transpose strategies.
 */
void registerFunctions()
{
    /* Register your solution function */
    registerTransFunction(transpose_submit, transpose_submit_desc); 

    /* Register any additional transpose functions */
    registerTransFunction(trans, trans_desc); 

}

/* 
 * is_transpose - This helper function checks if B is the transpose of
 *     A. You can check the correctness of your transpose by calling
 *     it before returning from the transpose function.
 */
int is_transpose(int M, int N, int A[N][M], int B[M][N])
{
    int i, j;

    for (i = 0; i < N; i++) {
        for (j = 0; j < M; ++j) {
            if (A[i][j] != B[j][i]) {
                return 0;
            }
        }
    }
    return 1;
}

