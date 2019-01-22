#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <limits.h>
#define INF INT_MAX
#define PROC_NUM 2 //Number of processes
#define SERIAL 1

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
                arr[i][j] = INF;   //Arbitrarily large values
            }
            j++;
            if(j == arr_size) {
                i++;
                j = 0;
            }
            if(i == arr_size) 
                break;
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
    int minValue = INF; //Arbitrarily large Value
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
//Find the minimum spanning tree based on the input adjacency matrix
void main(int argc, char **argv) {
    int NbyP, node, i, exitFlag;
    int **partialInputArray;
    int numOfNodes, processCnt;
    MPI_Init(&argc, &argv);
    //Get number of nodes using MPI_Comm_rank Function
    MPI_Comm_rank(MPI_COMM_WORLD, &node);
    if(node == 0) {
        FILE *inFile;
        int j;
        processCnt = 1;
        inFile = fopen("input.txt","r");
        int **inputArray = processInput(inFile, &numOfNodes);
        NbyP =  numOfNodes / PROC_NUM;
        partialInputArray = malloc(sizeof(int *) * numOfNodes);
        for(i = 0; i < numOfNodes; i++) {
            partialInputArray[i] = malloc(sizeof(int) * NbyP);
            for(j = 0; j < NbyP; j++) {
                partialInputArray[i][j] = inputArray[i][j];
            }
        }
        while(processCnt < PROC_NUM) {
            MPI_Send(&numOfNodes, 1, MPI_INT, processCnt, 0, MPI_COMM_WORLD);
            for(i = 0; i < numOfNodes; i++) {
                MPI_Send(&inputArray[i][processCnt*NbyP], NbyP, MPI_INT, processCnt,0,MPI_COMM_WORLD);
            }
            processCnt++;
        }
    }
    else {
        MPI_Recv(&numOfNodes, 1, MPI_INT, 0,0,MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        NbyP =  numOfNodes / PROC_NUM;
        partialInputArray = malloc(sizeof(int *) * numOfNodes);
        for(i = 0; i < numOfNodes; i++) {
            partialInputArray[i] = malloc(sizeof(int) * NbyP);
            MPI_Recv(partialInputArray[i], NbyP, MPI_INT, 0,0,MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        }
    }
    MPI_Barrier(MPI_COMM_WORLD); //Synchronization Primitive
    //Initialize D and V Matrices in each process
    int *D = malloc(sizeof(int) * NbyP);
    int *V = malloc(sizeof(int) * NbyP);
    //Initialize D and V
    for(i = 0; i < NbyP; i++) {
        D[i] = INF;   //Arbitratily large value
        V[i] = 0;
    }
    //Select first node from process 0 randomly
    int curNode = 0;
    if(node == 0) {
        curNode = rand() % NbyP;
        D[curNode] = 0;
    }
    int loopCount;
    //Synchronize processors before starting loop
    MPI_Barrier(MPI_COMM_WORLD);
    for(loopCount = 0; loopCount < numOfNodes; loopCount++) {
        //Broadcast current node to be selected for MST algorithm using process 0
        MPI_Bcast(&curNode, 1, MPI_INT, 0, MPI_COMM_WORLD);
        MPI_Barrier(MPI_COMM_WORLD);

        //Mark Node as visited, based on its location within process
        if(node == curNode/NbyP) {
            printf("Current Node: %d\n", curNode);
            int tmp = curNode % NbyP;
            V[tmp] = 1;
        }
        MPI_Barrier(MPI_COMM_WORLD);
        //Re-calculate the value of D matrix, after selecting a new node
        for(i = 0; i < NbyP; i++) {
            //If node is already visited, don't update corresponding D Matrix
            if(V[i] == 1) continue;
            if (D[i] > partialInputArray[curNode][i]) {
                D[i] = partialInputArray[curNode][i];
            }
        }
        //Find local Minimum from D Matrix
        int x = getNextVertex(D,V,NbyP);
        int minVertex;
        int lDataPair[2], gDataPair[2];
        //If all the nodes have been visited previously for any process
        //set min Value to be INF, so that other processes gets chance
        if(x >= NbyP) {
            minVertex = INF;
        }
        else {
            minVertex = D[x];
        }
        lDataPair[0] = minVertex;
        lDataPair[1] = x + node * NbyP;
        MPI_Allreduce(lDataPair, gDataPair, 1, MPI_2INT, MPI_MINLOC, MPI_COMM_WORLD);
        curNode = gDataPair[1];
        if(node == curNode/NbyP) {
            V[curNode % NbyP] = 1;
        } 
        MPI_Barrier(MPI_COMM_WORLD);
    }

    MPI_Barrier(MPI_COMM_WORLD);
    //Get the final result in the finalD Matrix in process 0
    if(node == 0) {
        int *finalD = malloc(sizeof(int) * numOfNodes);
        processCnt = 1;
        for(i = 0; i < NbyP; i++) {
            finalD[i] = D[i];
        }
        while(processCnt < PROC_NUM) {
            MPI_Recv(&finalD[processCnt*NbyP], NbyP , MPI_INT, processCnt, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            processCnt++;
        }
        for(i = 0; i < numOfNodes; i++) {
            printf("%d\t", finalD[i]);
        }
    }
    else {
        MPI_Send(&D[0], NbyP, MPI_INT, 0, 0, MPI_COMM_WORLD);
    }
    MPI_Finalize();
}

