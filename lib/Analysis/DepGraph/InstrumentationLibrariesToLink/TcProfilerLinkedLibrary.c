/*
 * TcProfilerLinkedLibrary.cpp
 *
 *  Created on: Dec 16, 2013
 *      Author: raphael
 */

#define __STDC_FORMAT_MACROS

#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

/*
 * Here we implement a tree of tuples <int,int>
 *
 * This list works like a std::map<int,int>
 *
 */
typedef struct _loopResults {

	struct _loopResults *left;   //Smaller GroupIDs
	struct _loopResults *right;  //Larger GroupIDs

	int64_t GroupID;
	int numInstances;

} LoopResults;

void printInOrder(LoopResults* root, FILE* outStream, int LoopClass, char* moduleIdentifier, int64_t ID){

	//stop recursion
	if (!root) return;

	printInOrder(root->left, outStream, LoopClass, moduleIdentifier, ID);

	fprintf(outStream, "TripCount %d %s.%" PRId64 " %" PRId64 " %d\n",
				LoopClass,
				moduleIdentifier,
				ID,
				root->GroupID,
				root->numInstances);

	printInOrder(root->right, outStream, LoopClass, moduleIdentifier, ID);

}


LoopResults* createLoopResults(int64_t GroupID, int numInstances){

	LoopResults* result = (LoopResults*)malloc(sizeof(LoopResults));

	result->GroupID = GroupID;
	result->numInstances = numInstances;
	result->left = NULL;
	result->right = NULL;

	return result;
}

LoopResults* findLoopResults(int64_t GroupID, LoopResults* root){

	//Stop recursion
	if (!root) return NULL;
	if (GroupID == root->GroupID) return root;

	if (GroupID < root->GroupID ) return findLoopResults(GroupID, root->left);
	return findLoopResults(GroupID, root->right);

}

void freeLoopResults(LoopResults* root){

	if(root){
		freeLoopResults(root->left);
		freeLoopResults(root->right);

		free(root);
	}

}

typedef struct {

	LoopResults* root;

} LoopResultTree;

void initLoopResultTree(LoopResultTree * T){
	T->root = NULL;
}

LoopResults* getOrInsertLoopResult(LoopResultTree * T, int64_t GroupID){

	LoopResults* result = findLoopResults(GroupID, T->root);

	if (!result) {

		result = createLoopResults(GroupID, 0);


		//Now we have to insert it into the tree
		if (!T->root) T->root = result; //base case
		else {

			int found = 0;

			LoopResults* previousNode;
			LoopResults* currentNode = T->root;

			//Find the node to do the insertion
			while(currentNode){

				previousNode = currentNode;

				if (GroupID < currentNode->GroupID) {
					currentNode = currentNode->left;
				} else {
					currentNode = currentNode->right;
				}

			}

			//Insert the node in the correct child
			if (GroupID < previousNode->GroupID) {
				previousNode->left = result;
			} else {
				previousNode->right = result;
			}

		}



	}

	return result;
}


/*
 * Here we implement a linked list of loop statistics
 *
 * _loopStats is the type that makes possible to do the chain of elements
 */
typedef struct _loopStats {

	struct _loopStats *next;

	int64_t ID;
	int LoopClass;
	LoopResultTree T;

} LoopStats;

LoopStats* createLoopStats(int64_t new_ID, int LoopClass){

	LoopStats* result = (LoopStats*)malloc(sizeof(LoopStats));

	result->ID = new_ID;
	result->LoopClass = LoopClass;
	initLoopResultTree(&(result->T));
	result->next = NULL;

	return result;
}

void addInstance(LoopStats* Stats, int64_t tripCount, int64_t estimatedTripCount ){


	int64_t GroupID;

	if (estimatedTripCount <= sqrt((double)tripCount))
		GroupID = 0;
	else if (estimatedTripCount <= tripCount/2)
		GroupID = 1;
	if (estimatedTripCount <= tripCount-2)
		GroupID = 2;
	else if (estimatedTripCount >= tripCount-1 && estimatedTripCount <= tripCount-1)
		GroupID = 3;
	else if (estimatedTripCount <= tripCount*2)
		GroupID = 4;
	else if (estimatedTripCount >= tripCount*tripCount)
		GroupID = 5;
	else
		GroupID = 6;

	LoopResults* lr = getOrInsertLoopResult(&(Stats->T), GroupID);

	lr->numInstances++;
}

typedef struct {

	LoopStats* First;

} LoopList;


void freeList(LoopList *L){

	LoopStats* currentNode = L->First;

	while (currentNode != NULL){
		LoopStats* nextNode = currentNode->next;

		freeLoopResults(currentNode->T.root);

		free(currentNode);
		currentNode = nextNode;
	}

	L->First = NULL;
}

LoopStats* getOrInsertLoop(LoopList *L, int64_t ID, int LoopClass){

	//Get
	LoopStats* currentNode = L->First;
	LoopStats* lastNode = NULL;

	while (currentNode != NULL){
		if (currentNode->ID == ID) break;
		lastNode = currentNode;
		currentNode = currentNode->next;
	}

	if (currentNode != NULL) return currentNode;

	//Not found; insert
	LoopStats* newNode = createLoopStats(ID, LoopClass);
	if (lastNode != NULL) {
		lastNode->next = newNode;
	} else {
		L->First = newNode;
	}

	return newNode;
}

LoopList loops;

void initLoopList(){
	loops.First = NULL;
}

void collectLoopData(int64_t LoopHeaderBBPointer, int64_t tripCount, int64_t estimatedTripCount, int LoopClass){
	addInstance(getOrInsertLoop(&loops, LoopHeaderBBPointer, LoopClass), tripCount, estimatedTripCount);
//	fprintf(stderr, "Actual:%" PRId64 " Estimated:%" PRId64 "\n", tripCount, estimatedTripCount);
}

void flushLoopStats(char* moduleIdentifier){

	FILE* outStream;
	outStream = fopen("loops.out", "a");

	if(!outStream){
		fprintf(stderr, "Error opening file loops.out");
	}else{

		LoopStats* currentNode = loops.First;

		while (currentNode != NULL){

			printInOrder(currentNode->T.root, outStream, currentNode->LoopClass, moduleIdentifier, currentNode->ID);

			currentNode = currentNode->next;
		}

		freeList(&loops);

		fclose(outStream);
	}



}
