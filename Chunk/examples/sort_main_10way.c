#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "merge.h"

#define RECORDS_NUM 10000
#define FILE_NAME "data_10way.db"
#define OUT_NAME "out"

static int createAndPopulateHeapFile(char* filename);

static void sortPhase(int file_desc,int chunkSize);

static void mergePhasesWithCount(int inputFileDesc,int chunkSize,int bWay,int* fileCounter,int* mergePasses);

static int nextOutputFile(int* fileCounter);

int main() {
  int chunkSize = 5;   // συρμοί των 5 blocks
  int bWay = 10;       // 10-way συγχώνευση
  int fileIterator = 0;
  int mergePasses = 0;

  BF_Init(LRU);
  int file_desc = createAndPopulateHeapFile(FILE_NAME);

  sortPhase(file_desc, chunkSize);
  mergePhasesWithCount(file_desc, chunkSize, bWay, &fileIterator, &mergePasses);

  printf("Params: chunkSize=%d blocks, bWay=%d\n", chunkSize, bWay);
  printf("Merge passes: %d\n", mergePasses);
  printf("Total passes (sort + merges): %d\n", mergePasses + 1);
  BF_Close();
}

static int createAndPopulateHeapFile(char* filename){
  HP_CreateFile(filename);
  int file_desc;
  HP_OpenFile(filename, &file_desc);

  Record record;
  srand(12569874);
  for (int id = 0; id < RECORDS_NUM; ++id) {
    record = randomRecord();
    HP_InsertEntry(file_desc, record);
  }
  return file_desc;
}

static void sortPhase(int file_desc,int chunkSize){
  sort_FileInChunks(file_desc, chunkSize);
}

static void mergePhasesWithCount(int inputFileDesc,int chunkSize,int bWay,int* fileCounter,int* mergePasses){
  int outputFileDesc = inputFileDesc;
  while(chunkSize <= HP_GetIdOfLastBlock(inputFileDesc)){
    outputFileDesc = nextOutputFile(fileCounter);
    merge(inputFileDesc, chunkSize, bWay, outputFileDesc);
    HP_CloseFile(inputFileDesc);
    chunkSize *= bWay;
    inputFileDesc = outputFileDesc;
    (*mergePasses)++;
  }
  HP_CloseFile(outputFileDesc);
}

static int nextOutputFile(int* fileCounter){
    char mergedFile[50];
    char tmp[] = "out";
    sprintf(mergedFile, "%s%d.db", tmp, (*fileCounter)++);
    int file_desc;
    HP_CreateFile(mergedFile);
    HP_OpenFile(mergedFile, &file_desc);
    return file_desc;
}
