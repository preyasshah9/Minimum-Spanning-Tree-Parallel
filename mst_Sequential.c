#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <limits.h>
#include <mpi.h>
#define INF INT_MAX

int **processInput(FILE *inputFile, int *numOfNodes) {
    int j,i, num,arr_size;
    if(inputFile) {
        fscanf(inputFile, "%d", &arr_size);
        *numOfNodes = arr_size;
        //Create first two dimensional array
        //after reading array size from file
        int **arr = malloc(sizeof(int *) * arr_size);
        for(j = 0; j < arr_size; j++) {
            arr[j] = malloc(sizeof(int) * arr_size);
        }
        i = 0, j = 0;
        while(!feof(inputFile)) {
            fscanf(inputFile,"%d", &num);
            arr[i][j] = num;
            //If no edge or node itself, we want to represent
            //it as infinity instead of 0
            if(num == 0) {
                arr[i][j] = 65536;   //Arbitrarily large values
            }
            j++;
            if(j == arr_size) {
                i++;
                j = 0;
            }
        }
        fclose(inputFile);
        return arr;
    }
    else {
        perror("Cannot find the input file");
        exit(0);
    }
}


int getNextVertex(int *D, int *V, int numOfNodes) {
    int i, minIndex = 0;
    int minValue = 65536; //Arbitrarily large Value
    int processFlag = 0;
    for(i = 0; i < numOfNodes; i++) {
        if(V[i] == 1) {
            continue;
        }
        processFlag = 1;
        if(D[i] < minValue) {
            minValue = D[i];
            minIndex = i;
        }
    }
    if(processFlag == 0) {
        minIndex = numOfNodes;
        return minIndex;
    }
    return minIndex;
}

int checkForExit(int *V, int numOfNodes) {
    int exitFlag = 0;
    int i = 0;
    for(i = 0; i < numOfNodes; i++) {
        if(V[i] == 1) {
            continue;
        }
        //There is still node left to explore
        //MST is not done yet
        exitFlag = 1;
        break;
    }
    return exitFlag;
}
//Find the minimum spanning tree based on the input adjacency matrix
void min_Spanning_Tree_Seq(int **inputArray, int numOfNodes) {
    int curNode, i;
    int *D = malloc(sizeof(int) * numOfNodes);
    int *V = malloc(sizeof(int) * numOfNodes);
    //Initialize D and V
    for(i = 0; i < numOfNodes; i++) {
        D[i] = INF;
        V[i] = 0;
    }
    //First randomly select a node
    curNode = rand() % numOfNodes;
    D[curNode] = 0;
    while(1) {
        V[curNode] = 1; //Mark Node as visited
        printf("Visited Node: %d\n", curNode);
        for(i = 0; i < numOfNodes; i++) {
            if (V[i] == 1) continue;
            if (D[i] > inputArray[curNode][i]) {
                D[i] = inputArray[curNode][i];
            }
        }
        if(checkForExit(V, numOfNodes) == 0) {
            break;
        }
        curNode = getNextVertex(D,V, numOfNodes);
    }
    //Done executing MST Algorithm
    for(i = 0; i < numOfNodes; i++) {
        printf("%d\t", D[i]);
    }
}

void main(int argc, char **argv) {
    struct timeval start_time, end_time, diff;
    gettimeofday(&start_time,NULL);
    FILE *inFile;
    int numOfNodes;
    int **inputArray;
    inFile = fopen("input.txt","r");
    inputArray = processInput(inFile, &numOfNodes);
    int i,j;
    min_Spanning_Tree_Seq(inputArray, numOfNodes);
    gettimeofday(&end_time,NULL);
    timersub(&start_time, &end_time, &diff);
    printf("Execution Time: %ld.%06ld\n", (long int)diff.tv_sec - (long int)diff.tv_usec);
}
