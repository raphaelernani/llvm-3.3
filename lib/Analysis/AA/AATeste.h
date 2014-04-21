/*
 * AATeste.h
 *
 *  Created on: Apr 15, 2014
 *      Author: raphael
 */

#ifndef AATESTE_H_
#define AATESTE_H_

#include "llvm/Pass.h"
#include "llvm/Analysis/AliasAnalysis.h"


namespace llvm {

class AATeste: public FunctionPass {
public:
	static char ID;




	AATeste():FunctionPass(ID) {





	}
	virtual ~AATeste() {}

	void getAnalysisUsage(AnalysisUsage& AU) const {
		AU.addRequired<AliasAnalysis>();
		AU.setPreservesAll();
	}

	bool runOnFunction(Function &F){


		return false;
	}



};

} /* namespace llvm */

#endif /* AATESTE_H_ */
