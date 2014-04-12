/*
 * RangeAnalysis.cpp
 *
 *  Created on: Mar 19, 2014
 *      Author: raphael
 */

#include "RangeAnalysis.h"

namespace llvm {

} /* namespace llvm */

void llvm::RangeAnalysis::solve() {

	depGraph->recomputeSCCs();

	std::list<int> SCCorder = depGraph->getSCCTopologicalOrder();

	for(std::list<int>::iterator SCCit = SCCorder.begin(), SCCend = SCCorder.end(); SCCit != SCCend; SCCit++ ){

		int SCCid = *SCCit;

		growthAnalysis(SCCid);
		fixFutures(SCCid);
		narrowingAnalysis(SCCid);

	}

}

void llvm::RangeAnalysis::growthAnalysis(int SCCid) {
}

void llvm::RangeAnalysis::fixFutures(int SCCid) {
}

void llvm::RangeAnalysis::narrowingAnalysis(int SCCid) {
}

void llvm::RangeAnalysis::addConstraints(
		std::map<const Value*, std::list<ValueSwitchMap> > constraints) {
}

Range llvm::RangeAnalysis::getRange(Value* V) {

	if (GraphNode* Node = depGraph->findNode(V) )
		return computedRanges[Node];

	//Value not found. Return [-inf,+inf]
	errs() << "Requesting range for unknown value: " << V << "\n";
	return Range();
}

/*************************************************************************
 * Class IntraProceduralRA
 *************************************************************************/

bool IntraProceduralRA::runOnFunction(Function& F) {

	//Build intra-procedural dependence graph
	functionDepGraph& M_DepGraph = getAnalysis<functionDepGraph>();
	depGraph = M_DepGraph.depGraph;

	//Extract constraints from branch conditions
	BranchAnalysis& brAnalysis = getAnalysis<BranchAnalysis>();
	std::map<const Value*, std::list<ValueSwitchMap> > constraints = brAnalysis.getIntervalConstraints();

	//Add branch information to the dependence graph. Here we add the future values
	addConstraints(constraints);

	//Solve range analysis
	solve();

	return false;
}

char IntraProceduralRA::ID = 0;
static RegisterPass<IntraProceduralRA> X("ra-intra",
		"Intra-procedural Range Analysis");

/*************************************************************************
 * Class InterProceduralRA
 *************************************************************************/

bool InterProceduralRA::runOnModule(Module& M) {

	//Build inter-procedural dependence graph
	moduleDepGraph& M_DepGraph = getAnalysis<moduleDepGraph>();
	depGraph = M_DepGraph.depGraph;

	for(Module::iterator Fit = M.begin(), Fend = M.end(); Fit != Fend; Fit++){

		//Skip functions without body (externally linked functions, such as printf)
		if (Fit->begin() == Fit->end()) continue;

		//Extract constraints from branch conditions
		BranchAnalysis& brAnalysis = getAnalysis<BranchAnalysis>(*Fit);
		std::map<const Value*, std::list<ValueSwitchMap> > constraints = brAnalysis.getIntervalConstraints();

		//Add branch information to the dependence graph. Here we add the future values
		addConstraints(constraints);
	}

	//Solve range analysis
	solve();

	return false;
}

char InterProceduralRA::ID = 0;
static RegisterPass<InterProceduralRA> Y("ra-inter",
		"Inter-procedural Range Analysis");
