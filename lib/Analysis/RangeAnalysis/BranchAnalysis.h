/*
 * BranchAnalysis.h
 *
 *  Created on: Mar 21, 2014
 *      Author: raphael
 */

#ifndef BRANCHANALYSIS_H_
#define BRANCHANALYSIS_H_

#include "llvm/IR/Instructions.h"

#include "ValueBranchMap.h"
#include "SymbolicInterval.h"
#include <set>


using namespace std;

namespace llvm {

class BranchAnalysis: public llvm::FunctionPass {
private:
	static char ID;

	std::set<ValueSwitchMap> IntervalConstraints;

public:

	BranchAnalysis(): FunctionPass(ID) {}
	virtual ~BranchAnalysis() {}

	bool runOnFunction(Function &F);

	void buildValueSwitchMap(const SwitchInst *sw);
	void buildValueBranchMap(const BranchInst *br);

	virtual void getAnalysisUsage(AnalysisUsage & AU){
		AU.setPreservesAll();
	}

};

} /* namespace llvm */

#endif /* BRANCHANALYSIS_H_ */
