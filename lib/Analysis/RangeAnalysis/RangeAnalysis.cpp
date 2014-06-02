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

	SCC_Iterator it(depGraph, SCCid);

	std::set<GraphNode*> worklist;

	while(it.hasNext()){

		GraphNode* node = it.getNext;
		in_state[node] = Range(Min, Max, Unknown);
		out_state[node] = Range(Min, Max, Unknown);

		std::map<GraphNode*, edgeType> preds = node->getPredecessors();
		if (preds.size() == 0) {
			worklist.insert(node);
		} else {
			for(std::map<GraphNode*, edgeType>::iterator pred = preds.begin(), pred_end = preds.end(); pred != pred_end; pred++){
				//Only data dependence edges
				if(pred->second != etData) continue;

				//looking for nodes that receive information from outside the SCC
				if(depGraph->getSCCID(pred->first) != SCCid) {
					worklist.insert(node);
					break;
				}
			}
		}
	}

	while(worklist.size() > 0){

		GraphNode* currentNode = *(worklist.begin());
		worklist.erase(currentNode);

		compute(currentNode, worklist);

	}

}

void llvm::RangeAnalysis::fixFutures(int SCCid) {
}

void llvm::RangeAnalysis::narrowingAnalysis(int SCCid) {
}

void llvm::RangeAnalysis::addConstraints(
		std::map<const Value*, std::list<ValueSwitchMap> > constraints) {

	//First we iterate through the constraints
	std::map<const Value*, std::list<ValueSwitchMap> >::iterator Cit, Cend;
	for(Cit = constraints.begin(), Cend = constraints.end(); Cit != Cend; Cit++){

		const Value* CurrentValue = Cit->first;

		// For each value, we iterate through its uses, looking for sigmas. We
		// can only learn with conditionals when we insert sigmas, splitting the
		// live ranges of the variables according to the branch results.

		Value::const_use_iterator Uit, Uend;
		for(Uit = CurrentValue->use_begin(), Uend = CurrentValue->use_end(); Uit != Uend; Uit++){

			const User* U = *Uit;
			if (const PHINode* CurrentUse = dyn_cast<const PHINode>(U)){

				if(isSigma(CurrentUse)){

					SigmaOpNode* CurrentSigmaOpNode = dyn_cast<SigmaOpNode>(depGraph->findOpNode(CurrentUse));
					if(!CurrentSigmaOpNode) continue;

					const BasicBlock* ParentBB = CurrentUse->getParent();

					// We will look for the symbolic range of the basic block of this sigma node.
					std::list<ValueSwitchMap>::iterator VSMit, VSMend;
					for(VSMit = Cit->second.begin(), VSMend = Cit->second.end(); VSMit != VSMend; VSMit++){

						ValueSwitchMap* CurrentVSM = &(*VSMit);

						int Idx = CurrentVSM->getBBid(ParentBB);
						if(Idx >= 0){

							//Save the symbolic interval of the opNode. We will use it in the narrowing
							// phase of the range analysis
							BasicInterval* BI = CurrentVSM->getItv(Idx);
							branchConstraints[CurrentSigmaOpNode] = *BI;

							//If it is a symbolic interval, then we have a future value and we must
							//insert a control dependence edge in the graph.
							if(SymbInterval* SI = dyn_cast<SymbInterval>(BI)){
								if (GraphNode* FutureValue = depGraph->findNode(SI->getBound())) {
									depGraph->addEdge(FutureValue, CurrentSigmaOpNode, etControl);
								}
							}

							break;
						}

					}

				}

			}

		}

	}

}

Range llvm::RangeAnalysis::getRange(Value* V) {

	if (GraphNode* Node = depGraph->findNode(V) )
		return out_state[Node];

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
