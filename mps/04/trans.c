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

void trans64(int M, int N, int A[N][M], int B[M][N]);
void trans32(int M, int N, int A[N][M], int B[M][N]); 


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
  switch(N) { //introduced for different sized blocks
    case 32:
      trans32(M, N, A, B);
      break;
    case 64:
      trans64(M, N, A, B);
      break;
    default:
      trans32(M,N,A,B);
      break;
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
char trans64_desc[]= "Row by row transpose-64 byte-sized blocks";
void trans64(int M, int N, int A[N][M], int B[M][N]){

       int i, j, col, row, temp[8]; //initialize variables
       for (col = 0; col < N; col += 8) {
         for (row = 0; row < N; row += 8) {// transpose first 4 rows of the 8x8 block and store each row in temp
           for (i = 0; i < 4; i++) {
             for (j = 0; j < 8; j++) { // store a row in temp
               temp[j] = A[col + i][row + j];
             }
             for (j = 0; j < 4; j++) {
               B[row + j][col + i] = temp[j];
             }
             for (j = 0; j < 3; j++) {
               B[row + j][col + i + 4] = temp[j + 5];
             }
             B[row + 3][col + i + 4] = temp[4];
          }

           for (j = 0; j < 4; j++) { // any remaining values not in final pos
             temp[j] = A[col + j + 4][row + 4];
           }
           for (j = 0; j < 4; j++) {
             temp[j + 4] = A[col + j + 4][row + 3];
           }
           for (j = 0; j < 4; j++) {
             B[row + 4][col + j] = B[row + 3][col + j + 4];
           }
           for (j = 0; j < 4; j++) {
             B[row + 4][col + j + 4] = temp[j];
           }
           for (j = 0; j < 4; j++) {
             B[row + 3][col + j + 4] = temp[j + 4];
           }
           //first four rows done
           for (i = 0; i < 3; i++) {   // transpose lower half of the 8x8 block
              for (j = 0; j < 4; j++) {
                temp[j] = A[col + j + 4][row + i + 5];
              }
              for (j = 0; j < 4; j++) {
                temp[j + 4] = A[col + j + 4][row + i];
              }
              for (j = 0; j < 4; j++) {
                B[row + i + 5][col + j] = B[row + i][col + j + 4];
              }
              for (j = 0; j < 4; j++) {
                B[row + i + 5][col + j + 4] = temp[j];
              }
              for (j = 0; j < 4; j++) {
                B[row + i][col + j + 4] = temp[j + 4];
              }
            }
        }
    }

}

char trans32_desc[]= "Row by row transpose- 32 byte-sized blocks";
void trans32(int M, int N, int A[N][M], int B[M][N]){

  int size= 256; // 2^5blocks
  int blockSize;
  int tmp;

  if (M==N){
    blockSize= size/N;
  } else {
    blockSize=16;
  }

  for (int j1 = 0; j1 < M; j1 += blockSize) {
    for (int i1 = 0; i1 < N; i1 += blockSize) { //transpose each block
      for (int i = i1; i < i1 + blockSize && i < N; i++) {
        for (int j = j1; j < j1 + blockSize && j < M; j++) {
          if (i != j)
            B[j][i] = A[i][j];
          else
            tmp = A[i][i];
        }
        if (i1 == j1)
          B[i][i] = tmp;
      }
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
