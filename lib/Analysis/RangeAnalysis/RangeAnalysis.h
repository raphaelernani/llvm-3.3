/*
 * RangeAnalysis.h
 *
 *  Created on: Mar 19, 2014
 *      Author: raphael
 */

#ifndef RANGEANALYSIS_H_
#define RANGEANALYSIS_H_

//std includes
#include <list>
#include <map>

//LLVM includes
#include "llvm/Pass.h"


//Our own stuff
#include "../DepGraph/DepGraph.h"

#include "BranchAnalysis.h"
#include "Range.h"


namespace llvm {

class RangeAnalysis {
private:
	void growthAnalysis(int SCCid);
	void fixFutures(int SCCid);
	void narrowingAnalysis(int SCCid);

	std::map<SigmaOpNode*, BasicInterval> branchConstraints;

	std::map<GraphNode*,Range> computedRanges;
protected:
	void solve();
	void addConstraints(std::map<const Value*, std::list<ValueSwitchMap> > constraints);

public:
	DepGraph* depGraph;

	RangeAnalysis(): depGraph(NULL) {};
	virtual ~RangeAnalysis() {};

	Range getRange(Value* V);
};

} /* namespace llvm */



class IntraProceduralRA: public FunctionPass, public RangeAnalysis {
public:
	static char ID;
	IntraProceduralRA():FunctionPass(ID){}
	virtual ~IntraProceduralRA(){};

    void getAnalysisUsage(AnalysisUsage &AU) const {
        AU.addRequired<functionDepGraph> ();
        AU.addRequired<BranchAnalysis> ();
        AU.setPreservesAll();
    }

    bool runOnFunction(Function &F);
};

class InterProceduralRA: public ModulePass, public RangeAnalysis {
public:
	static char ID;
	InterProceduralRA():ModulePass(ID){}
	virtual ~InterProceduralRA(){};

    void getAnalysisUsage(AnalysisUsage &AU) const {
        AU.addRequired<moduleDepGraph> ();
        AU.addRequired<BranchAnalysis> ();
        AU.setPreservesAll();
    }

    bool runOnModule(Module &M);
};




#endif /* RANGEANALYSIS_H_ */
