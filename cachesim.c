#include <math.h>  
#include <stdlib.h>
#include <stdio.h>
#include <string.h> 
#include <stdbool.h>

/*
Author: Lindsay Maggioncalda lnm22
Homework #5, due 12 April 2019
*/


/*
Models a cache.
*/


/* declare block struct */
struct block {
	int data[64];
	int isDirty; //default is clean
	int isValid; //default is invalid
	int tag;
	int recency;
} ;

/* declare functions */

void doStore(char* addy, int sizeOfAccess, char* valToWrite, int numBlockOffsetBits, int numSetIndexBits, int setIndBits, int tagBits, struct block** cache, int numWays, int isWB, int blockOffsetBits, int* memory, int blockSize);

void doLoad(char* addy, int sizeOfAccInt, int numBlockOffsetBits, 
			int numSetIndexBits, int setIndBits, int tagBits, struct block** cache, int numWays, int isWB, int blockOffsetBits, int* memory, int blockSize);

char* lastcharDel(char* strang);

int getTagBits(char* addy, int numBlockOffsetBits, int numSetIndexBits);
int getSIBits(char* addy, int numBlockOffsetBits, int numSetIndexBits);
int getBOBits(char* addy, int numBlockOffsetBits);

int isWriteBack(char type);

int isHit(int setIndBits, int tagBits, struct block** cache, int numWays);

void writeToBlock(int isHit, struct block** cache, int setIndBits, char* valToWrite, int blockOffsetBits, int sizeOfAcc);
void writeToMemory(char* addy, char* valToWrite, int sizeOfAccInt, int* memory);

int* getDataFromCache(int isHit, struct block** cache, int setIndBits, int blockOffsetBits, int sizeOfAcc, int* dataArray);

struct block getBlock(char* addy, int tagBits, int blockSize, int blockOffsetBits, int* memory);

int isSetFull(int setIndBits, struct block** cache, int numWays);

int findLRU(int setIndBits, struct block** cache, int numWays);

int main (int argc, char *argv[])  {

	int wordSize = 16;


/* GET INPUT FROM COMMAND LINE */

	/* get filename from command line */
	char* fileName = argv[1];
	/* open file for reading */
	FILE* filePtr = fopen(fileName, "r"); /* r means reading text */

	/* get capacity, set-associativity, wb/wt, and block size */
	char* a = argv[2];
	//printf("fname is is %s\n", fileName);
	int capacityKB = atoi(a);
	//printf("capacity is %d\n", capacityKB);
	char* b = argv[3];
	int numWays = (int) atoi(b);
	//printf("set assoc is %d\n", numWays);

	char wbOrwt = argv[4][1];
	int isWB = isWriteBack(wbOrwt);

	char* c = argv[5];
	int blockSize = (int) atoi(c);
	//printf("block size is %d\n", blockSize);


/* SET UP CACHE */
	//printf("SETTING UP!!!\n");
	/* calc num of frames */
	int numFrames = (capacityKB * pow(2, 10))/(blockSize);
	//printf("num frames is %d\n", numFrames);

	/* calc num of sets */
	int numSets = (numFrames/numWays);
	//printf("num sets is %d\n", numSets);

	/* Create array of blocks, set all values to 0 */
	struct block** myCache = (struct block**) malloc(numSets * sizeof (struct block*));
	int i = 0;
	while (i < numSets){
		myCache[i] = (struct block*) malloc(numWays * sizeof(struct block));
		i ++;
	}
	int k = 0;

	//for every row...
	while (k < numSets){
		int y = 0;

		//and every column...
		//fill in values for this block
		while (y < numWays){
			myCache[k][y].isDirty = 0;
			myCache[k][y].isValid = 0;
			myCache[k][y].tag = 0;
			myCache[k][y].recency = 0;
			int z = 0;

			while (z < 64){
				//fill in the array of this block
				myCache[k][y].data[z] = 0;
				z += 1;
			}
			y += 1;
		}
		k += 1;
	}
 
 
/* ESTABLISH MAIN MEMORY */
	//check this...
	int numElinMem = (int) pow(2, 16);
	//printf("number of elements in main mem is %d\n", numElinMem);
	int* mainMem = (int*) malloc(numElinMem * sizeof(int));
	int r = 0;
	while (r < numElinMem){
		mainMem[r] = 0;
		r++;
	}

	/* READ FILE */
	char line[100];
	while (fgets(line, 100, filePtr) != NULL){


	//printf("READING FILE!!\n");
	/* get information from the file */
	if (fileName != NULL){
		//printf("line is:\n");
		//printf("%s\n", line);
	}

	/* assign strings in line to variables */
	char* type = strtok(line, " "); 
	//printf("type is %s\n",type);
	char* addy = strtok(NULL, " "); 
	char* sizeOfAcc = strtok(NULL, " "); 
	//printf("sizeOfAcc = %s\n",sizeOfAcc);
	int sizeOfAccInt;
	sscanf(sizeOfAcc,"%x",&sizeOfAccInt);

/* SET UP ADDRESS BITS */
	int numBlockOffsetBits = (log(blockSize))/(log(2));
	//printf("num block offset bits is %d\n", numBlockOffsetBits);

	int numSetIndexBits = (log(numSets))/(log(2));
	//printf("num set index bits is %d\n", numSetIndexBits);

	int numTagBits = wordSize - numBlockOffsetBits - numSetIndexBits;
	//printf("num tag bits is %d\n", numTagBits);


	int blockOffsetBits = getBOBits(addy,numBlockOffsetBits);
	int tagBits = getTagBits(addy,numBlockOffsetBits, numSetIndexBits);
	int setIndBits = getSIBits(addy,numBlockOffsetBits, numSetIndexBits);
	//printf("block offset bits are %d\n",blockOffsetBits);
	//printf("tag bits are %d\n",tagBits);
	//printf("set index bits are %d\n",setIndBits);



	/* increment recency by 1 */
	int h = 0;
	while (h < numWays){
		myCache[setIndBits][h].recency += 1;
		h++;
	}

	if (((int)strlen(sizeOfAcc) == 1) && (strcmp(type, "load")==0)){
		doLoad(addy, sizeOfAccInt, numBlockOffsetBits, numSetIndexBits, setIndBits, tagBits, myCache, numWays, isWB, blockOffsetBits, mainMem, blockSize);
	}

	
	/* if this string contains \n, we know this is a LOAD */
	else if (sizeOfAcc[strlen(sizeOfAcc) - 1] == 10){
		//printf("this is a LOAD\n");
		sizeOfAcc = lastcharDel(sizeOfAcc);
		doLoad(addy, sizeOfAccInt, numBlockOffsetBits, numSetIndexBits, setIndBits, tagBits, myCache, numWays, isWB, blockOffsetBits, mainMem, blockSize);
	}

	else {
	/* it's a store */
		//get last value from line
		char* val = strtok(NULL, " "); 
		val = lastcharDel(val);
		//printf("we will now store the value: %s\n", val);
		doStore(addy, sizeOfAccInt, val, numBlockOffsetBits, numSetIndexBits, setIndBits, tagBits, myCache, numWays, isWB, blockOffsetBits, mainMem, blockSize);
	}

	//printf("u are successful bb\n");

}
    return 0;

}


/* simulates a store */
void doStore(char* addy, int sizeOfAccInt, char* valToWrite, int numBlockOffsetBits, 
			int numSetIndexBits, int setIndBits, int tagBits, struct block** cache, int numWays, int isWB, int blockOffsetBits, int* memory, int blockSize) {
	//printf("store\n");
	int isAHit = isHit(setIndBits, tagBits, cache, numWays);
	//printf("is a hit? %d\n",isAHit);

	//STORE HIT
	if (isAHit >= 0){
		//printf("Hit!\n");

		//STORE HIT ON WB
		if (isWB == 1){
			//printf("WRITE BACK\n");
			//write data to block in cache.
			writeToBlock(isAHit, cache, setIndBits, valToWrite, blockOffsetBits, sizeOfAccInt);
			//block = dirty.
			cache[setIndBits][isAHit].isDirty = 1;
			cache[setIndBits][isAHit].recency = 0;

		}
		//STORE HIT ON WT
		else {
			//printf("WRITE THROUGH\n");
			//write data to block in cache
			writeToBlock(isAHit, cache, setIndBits, valToWrite, blockOffsetBits, sizeOfAccInt);
			//printf("wrote to block in cache.\n");
			// and memory.
			writeToMemory(addy, valToWrite, sizeOfAccInt, memory);
			cache[setIndBits][isAHit].recency = 0;

		}

	//printf("*******************\n");
	printf("store %s hit\n",addy);
	//printf("*******************\n");

	}

	//STORE MISS
	else {
		//printf("not a hit. miss\n");
		//STORE MISS ON WB
		if (isWB == 1){

			//if miss, write the data to memory.
			writeToMemory(addy, valToWrite, sizeOfAccInt, memory);


			//PROBLEM MAY BE HERE*******
			//now get the entire block from memory
			struct block blocky = getBlock(addy, tagBits, blockSize, blockOffsetBits, memory);




			int isFull;
			isFull = isSetFull(setIndBits, cache, numWays);
			if (isFull >= 0){
				cache[setIndBits][isFull] = blocky;
			}

			else {
				int d;
				//Find LRU
				d = findLRU(setIndBits, cache, numWays);
				if (cache[setIndBits][d].isDirty == 1){
				int lruTag = cache[setIndBits][d].tag;
				int lruMemAddress = lruTag * ((int)pow(2,numBlockOffsetBits+numSetIndexBits));
				//printf("lru's address is %2hhx\n",lruMemAddress);
				//Get ~~ALL INFO~~ from the LRU
				//should have same number of bytes as block size
				int lruEmptyArray[blockSize];
				//no block offset; need all information from all bytes in whole block
				int* lruData = getDataFromCache(d, cache, setIndBits, 0, blockSize, lruEmptyArray);
				int b = 0;
				while (b < blockSize){
					memory[lruMemAddress + b] = lruData[b];
					b++;
					}
				}

				cache[setIndBits][d] = blocky;
			}





		}
		//STORE MISS ON WT
		else {
			//printf("store miss wt\n");
			// write to memory.
			writeToMemory(addy, valToWrite, sizeOfAccInt, memory);

		}

	//printf("**************\n");
	printf("store %s miss\n",addy);
	//printf("**************\n");


	}
	
}

/* simulates a load */
void doLoad(char* addy, int sizeOfAccInt, int numBlockOffsetBits, 
			int numSetIndexBits, int setIndBits, int tagBits, struct block** cache, int numWays, int isWB, int blockOffsetBits, int* memory, int blockSize) {
	//printf("LOAD\n");
	int isAHit = isHit(setIndBits, tagBits, cache, numWays);
	//printf("is a hit? %d\n",isAHit);

	//LOAD HIT
	if (isAHit >= 0){
		//printf("Hit!\n");
		int dataArray[sizeOfAccInt];
		int* myData;

		//LOAD HIT ON WB
		if (isWB == 1){
			//printf("WRITE BACK\n");
			myData = getDataFromCache(isAHit, cache, setIndBits, blockOffsetBits, sizeOfAccInt, dataArray);
			cache[setIndBits][isAHit].recency = 0;
		}

		//LOAD HIT ON WT
		else {
			//printf("WRITE THROUGH\n");
			myData = getDataFromCache(isAHit, cache, setIndBits, blockOffsetBits, sizeOfAccInt, dataArray);
			cache[setIndBits][isAHit].recency = 0;
		}

	//printf("*************\n");
	printf("load %s hit ",addy);
	int county = 0;
	while (county < sizeOfAccInt){
		printf("%.2hhx", myData[county]);
		county++;
		}
	printf("\n");
	//printf("*************\n");
	}

	//LOAD MISS
	else {

		//printf("not a hit. miss\n");
		int dataArray[sizeOfAccInt];
		int* myData;


		//LOAD MISS ON WB
		if (isWB == 1){
			//printf("LOAD MISS WB\n");
			struct block wbBlock = getBlock(addy, tagBits, blockSize, blockOffsetBits, memory);
			int isFull;
			isFull = isSetFull(setIndBits, cache, numWays);
			//printf("isFull? %d\n",isFull);
			if (isFull >= 0){
				//if set is not full, there's no need to evict any blocks.
				//printf("not full, block will move into way %d\n",isFull);
				cache[setIndBits][isFull] = wbBlock;
				//printf("%d\n",cache[setIndBits][isFull].data[2]);
				myData = getDataFromCache(isFull, cache, setIndBits, blockOffsetBits, sizeOfAccInt, dataArray);

			}
			else {
				//if full, get rid of LRU.
				//FIND LRU
				int d;
				d = findLRU(setIndBits, cache, numWays);
				//printf("the lru is at index %d\n",d);

				if (cache[setIndBits][d].isDirty == 1){
				int lruTag = cache[setIndBits][d].tag;
				//printf("lru's tag is %d\n",lruTag);
				int lruMemAddress = lruTag * ((int)pow(2,numBlockOffsetBits+numSetIndexBits));
				//printf("lru's address is %2hhx\n",lruMemAddress);
				//Get ~~ALL INFO~~ from the LRU
				//should have same number of bytes as block size
				int lruEmptyArray[blockSize];
				//no block offset; need all information from all bytes in whole block
				int* lruData = getDataFromCache(d, cache, setIndBits, 0, blockSize, lruEmptyArray);
				int b = 0;
				while (b < blockSize){
					memory[lruMemAddress + b] = lruData[b];
					b++;
					}
				}
				//REPLACE LRU. Go into cache
				cache[setIndBits][d] = wbBlock;
				//get info from self
				//printf("came here!!!!\n");
				//printf("my data of interest is: %d",cache[setIndBits][d].data[63]);
				//printf("access size is %d\n", sizeOfAccInt);
				myData = getDataFromCache(d, cache, setIndBits, blockOffsetBits, sizeOfAccInt, dataArray);
			}

		}
		//LOAD MISS ON WT
		else {
			//printf("LOAD MISS WT\n");
			struct block wtBlock = getBlock(addy, tagBits, blockSize, blockOffsetBits, memory);
			int isFull;
			isFull = isSetFull(setIndBits, cache, numWays);
			//printf("isFull? %d\n",isFull);
			if (isFull >= 0){
				//if set is not full, there's no need to evict any blocks.
				//printf("not full, block will move into way %d\n",isFull);
				cache[setIndBits][isFull] = wtBlock;
				myData = getDataFromCache(isFull, cache, setIndBits, blockOffsetBits, sizeOfAccInt, dataArray);
			}

			else {
				//if full, get rid of LRU.
				//FIND LRU
				int d;
				d = findLRU(setIndBits, cache, numWays);
				/*
				//printf("the lru is at index %d\n",d);
				int lruTag = cache[setIndBits][d].tag;
				//printf("lru's tag is %d\n",lruTag);
				int lruMemAddress = lruTag * ((int)pow(2,numBlockOffsetBits+numSetIndexBits));
				//printf("lru's address is %2hhx\n",lruMemAddress);
				//Get ~~ALL INFO~~ from the LRU
				//should have same number of bytes as block size
				int lruEmptyArray[blockSize];
				//no block offset; need all information from all bytes in whole block
				int* lruData = getDataFromCache(d, cache, setIndBits, 0, blockSize, lruEmptyArray);
				int b = 0;
				while (b < blockSize){
					memory[lruMemAddress + b] = lruData[b];
					b++;
				}

				*/

				//REPLACE LRU. Go into cache
				cache[setIndBits][d] = wtBlock;

				//get info from self
				myData = getDataFromCache(d, cache, setIndBits, blockOffsetBits, sizeOfAccInt, dataArray);


				//fix all of this!!!
			}


		}

	//printf("*************\n");
	printf("load %s miss ",addy);
	int county = 0;
	while (county < sizeOfAccInt){
		printf("%.2hhx", myData[county]);
		county++;
		}
	printf("\n");
	//printf("*************\n");



	}
}

/* deletes the last character of a string */
char* lastcharDel(char* strang) {
    int i = 0;
    while(strang[i] != '\0') {
        i++; 
    }
    strang[i-1] = '\0';
    return strang;
}

int getBOBits(char* addy, int numBlockOffsetBits){
	int x;
	sscanf(addy,"%x",&x);
	//printf("the hex number is %d in decimal\n",x);
	int bottomBits = (x)%((int)pow(2,numBlockOffsetBits));
	return bottomBits;
}

int getTagBits(char* addy, int numBlockOffsetBits, int numSetIndexBits){
	int x;
	sscanf(addy,"%x",&x);
	int tagBits = (x)/((int)pow(2,numBlockOffsetBits+numSetIndexBits));
	return tagBits;

}

int getSIBits(char* addy, int numBlockOffsetBits, int numSetIndexBits){
	int x;
	sscanf(addy,"%x",&x);
	int siBits;

	siBits = (x)/((int)pow(2,numBlockOffsetBits)) % ((int)(pow(2,numSetIndexBits)));
	return siBits;
}

int isHit(int setIndBits, int tagBits, struct block** cache, int numWays){
	int k = 0;
	//printf("num ways is %d\n", numWays);
	//printf("looking for a hit\n");
	while (k < numWays){
		//printf("k is %d\n", k);
		//printf("the tag at cache[%d][%d] is: %d\n",setIndBits,k,cache[setIndBits][k].tag);
		if (cache[setIndBits][k].tag == tagBits && cache[setIndBits][k].isValid){
			return k;
		}
		k += 1;
	}
	return -1;
}

int isWriteBack(char type){
	if (type == 'b'){
		return 1;
	}
	return 0;
}

void writeToBlock(int isHit, struct block** cache, int setIndBits, char* valToWrite, int blockOffsetBits, int sizeOfAcc){
	int k = isHit;
	int offset = blockOffsetBits;
	int counter = 0;
	char* value = valToWrite;
	while (counter < sizeOfAcc){
		char oneByte;
		sscanf(value, "%2hhx", &oneByte);
		cache[setIndBits][k].data[offset + counter] = oneByte;
		//printf("wrote byte %2hhx to cache[%d][%d].data[%d]\n",oneByte,setIndBits,k,offset + counter);
		counter ++;
		value += 2;
	}
}

void writeToMemory(char* addy, char* valToWrite, int sizeOfAccInt, int* memory){
	int address;
	sscanf(addy,"%x",&address);
	int counter = 0;
	char* value = valToWrite;
	while (counter < sizeOfAccInt){
		char oneByte;
		sscanf(value, "%2hhx", &oneByte);
		memory[address + counter] = oneByte;
		//printf("wrote byte %2hhx to mainMem[%x]\n",oneByte,address + counter);
		counter ++;
		value += 2;
	}

}

int* getDataFromCache(int isHit, struct block** cache, int setIndBits, int blockOffsetBits, int sizeOfAcc, int* dataArray){
	//printf("WE IN HERE\n");
	int k = isHit;
	int offset = blockOffsetBits;
	//printf("offset is %d\n", offset);
	int counter = 0;
	while (counter < sizeOfAcc){
		int theByte = cache[setIndBits][k].data[offset + counter];
		dataArray[counter] = theByte;
		//printf("data[%d] is %2hhx\n",counter,theByte);
		counter ++;
	}
	return dataArray;
}

struct block getBlock(char* addy, int tagBits, int blockSize, int blockOffsetBits, int* memory){
	//printf("getting block");
	struct block blocky;
	int address;
	sscanf(addy,"%x",&address);
	int k = 0;
	//printf("address is %d\n",address);
	//printf("offset is %d\n",blockOffsetBits);
	int start = address - blockOffsetBits;
	while (k < blockSize){
		blocky.data[k] = memory[start];
		//printf("just filled element %d with value %2hhx at address %d in mem\n",k,memory[start],start);
		k++;
		start++;
	}
	blocky.isValid = 1;
	blocky.isDirty = 0;
	blocky.tag = tagBits;
	blocky.recency = 0;
	return blocky;
}

int isSetFull(int setIndBits, struct block** cache, int numWays){
	int index = 0;
	while (index < numWays){
		if (cache[setIndBits][index].isValid == 0){
			return index;
		}
		index++;
	}
	return -1;
}

int findLRU(int setIndBits, struct block** cache, int numWays){
	int max = 0;
	int maxInd = 0;
	int index = 0;
	while (index < numWays){
		if (cache[setIndBits][index].recency > max){
			max = cache[setIndBits][index].recency;
			maxInd = index;
		}
		index++;
	}
	return maxInd;
}




