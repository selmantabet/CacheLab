/* Selmane Tabet - 210004050
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
    int rCount;
    int cCount;
    int rSector;
    int cSector;
    //This is to prevent cache misses along the diagonal
    int diag = 0;
    int tmp = 0;
    //For the 64x64 matrix case
    //We use the same looping structure for 64x64 or 32x32 matrices, block size changes depending on that though.
    
    int blocker; //Block size var, changes by input matrix size.

    if(N == 32 && M == 32){
        blocker = 8; //Blocks of 8
            for(cSector = 0; cSector < N; cSector += blocker){
                for(rSector = 0; rSector < N; rSector += blocker){
                    //Now we cycle within one block, so we loop sector at a time
                    for(rCount = rSector; rCount < rSector + blocker; rCount++){
                        for(cCount = cSector; cCount < cSector + blocker; cCount++){
                            //As long as we are not on the diagonal, we put the value
                            //from A[row][column] into B[column][row], the heart of transposing
                            if(rCount != cCount){
                                B[cCount][rCount] = A[rCount][cCount];
                            
                            }
                            else{
                                //The digonal variable keeps to data in a cache utilizing spatial locality
                                diag = rCount;
                                tmp = A[rCount][cCount];
                            }
                        }
                        //In a square matrix, if the sector we are blocking off contains 
                        //the diagonal, then we keep those elements the same since they don't change in trans.
                        if(cSector == rSector){
                            B[diag][diag] = tmp;
                        }
                    }
                }
            }
        }
        else if(N == 64){
            blocker = 4; //Blocks of 4
            for(cSector = 0; cSector < M; cSector += blocker){
                for(rSector = 0; rSector < N; rSector += blocker){
                    for(rCount = rSector; rCount < rSector + blocker ; rCount++){
                        for(cCount = cSector; cCount < cSector + blocker ; cCount++){
                            if(rCount != cCount){
                                B[cCount][rCount] = A[rCount][cCount];  
                            }
                            else{
                                diag = rCount;
                                tmp = A[rCount][cCount];
                            }
                        }
                        //In a square matrix, if the sector we are blocking off contains 
                        //the diagonal, then we keep those elements the same since they don't change in trans.
                        if(cSector == rSector){
                            B[diag][diag] = tmp;
                        }
                    }
            }
        }
    }
    //For the prime numbered uneven matrices, we use 16 to find a medium between larger matrices and smaller ones.
    else{
        blocker = 16; //Blocks of 16
        for (cSector = 0; cSector < N;  cSector += blocker){
            for(rSector = 0; rSector < N; rSector += blocker){
                //We check blocks in square format, but this will not work for matrices like 61x67
                //They are uneven non-square matrices
                for(rCount = rSector; (rCount < rSector + blocker)  && (rCount < N); rCount++){
                    for(cCount = cSector; (cCount < cSector + blocker)  && (cCount < M); cCount++){
                        if(rCount != cCount){
                            B[cCount][rCount] = A[rCount][cCount];
                        }
                        else{
                            diag = rCount;
                            tmp = A[rCount][cCount];
                        }
                    }

                    if(cSector == rSector){
                        B[diag][diag] = tmp;
                    }
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

