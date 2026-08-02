#include "merge.h"

// Helper: find index of minimum record among active iterators
// ChatGPT: static int as return value -> unnecessary
int findMin(Record *records, bool *active, int n) {
    int min = -1;
    for (int i = 0; i < n; i++) {
        if (!active[i]) continue;
        if (min == -1 || shouldSwap(&records[min], &records[i])) {
            min = i;
        }
    }
    return min;
}

void merge(int input_FileDesc, int chunkSize, int bWay, int output_FileDesc) {
    if (input_FileDesc < 0 || output_FileDesc < 0 || chunkSize <= 0 || bWay <= 1)
        return;

    CHUNK_Iterator chunkIt = CHUNK_CreateIterator(input_FileDesc, chunkSize);
    CHUNK chunks[bWay];

    while (1) {
        int actualChunks = 0;
        // Fetch up to bWay chunks
        for (int i = 0; i < bWay; i++) {
            if (CHUNK_GetNext(&chunkIt, &chunks[i]) == 0) {
                actualChunks++;
            } else {
                // We have reached the end of the file
                break;
            }
        }

        // No more chunks to merge in current pass
        if (actualChunks == 0) break;

        CHUNK_RecordIterator recIts[bWay];
        Record current[bWay];
        bool active[bWay];

        for (int i = 0; i < actualChunks; i++) {
            // Create a record iterator for each loaded chunk and load the first record
            recIts[i] = CHUNK_CreateRecordIterator(&chunks[i]);
            if (CHUNK_GetNextRecord(&recIts[i], &current[i]) == 0) {
                active[i] = true;
            } else {
                active[i] = false;
            }
        }

        int activeCount = actualChunks;

        // 
        while (activeCount > 0) {
            int minIdx = findMin(current, active, actualChunks);

            // ChatGPT: if (minIdx == -1) break;
            // if activeCount > 0, then findMin must find a valid record
            assert(minIdx >= 0);

            HP_InsertEntry(output_FileDesc, current[minIdx]);

            if (CHUNK_GetNextRecord(&recIts[minIdx], &current[minIdx]) != 0) {
                active[minIdx] = false;
                activeCount--;
            }
        }
    }
}
