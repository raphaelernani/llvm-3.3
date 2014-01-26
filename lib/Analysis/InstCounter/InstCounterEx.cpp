/*
 * InstCounterEx.cpp
 *
 *  Created on: Jan 26, 2014
 *      Author: raphael
 */
#ifndef DEBUG_TYPE
#define DEBUG_TYPE "InstCounterEx"
#endif

#include "InstCounterEx.h"

STATISTIC(NumInstructions, "Number of Instructions");
STATISTIC(NumInstructions2, "Number of Instructions 2");
STATISTIC(NumInstructions3, "Number of Instructions 3");

using namespace llvm;

int instCount(Module& M){

	int result = 0;

	for (auto& F : M){
		for (auto &BB: F){
			for (auto &I: BB){
				result++;
			}
		}
	}

	return result;
}


bool InstCounterEx::runOnModule(Module& M) {
	NumInsts = instCount(M);
	NumInstructions = NumInsts;
	return false;
}

char InstCounterEx::ID = 0;
RegisterPass<InstCounterEx> X("InstCounterEx","Instruction Counter");


bool InstCounterEx2::runOnModule(Module& M) {
	NumInsts = instCount(M);
	NumInstructions2 = NumInsts;
	return false;
}

char InstCounterEx2::ID = 0;
RegisterPass<InstCounterEx2> Y("InstCounterEx2","Instruction Counter 2");


bool InstCounterEx3::runOnModule(Module& M) {
	NumInsts = instCount(M);
	NumInstructions3 = NumInsts;
	return false;
}

char InstCounterEx3::ID = 0;
RegisterPass<InstCounterEx3> Z("InstCounterEx3","Instruction Counter 3");



