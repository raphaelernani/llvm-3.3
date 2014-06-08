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

	fixPointIteration(SCCid, loJoin);

}

Range llvm::RangeAnalysis::evaluateNode(GraphNode* Node){

	Range result;

	/*
	 * VarNode: Constants generate constant ranges;
	 * otherwise, the output is a union of the predecessors
	 * */
	if(VarNode* VN = dyn_cast<VarNode>(Node)){
		Value* val = VN->getValue();

		if (ConstantInt* CI = dyn_cast<ConstantInt>(val)){
			APInt value = CI->getValue();
			result = Range(value,value);
		} else {
			result = getUnionOfPredecessors(Node);
		}

	}
	/*
	 * Nodes that do not modify the data: PHI, Sigma, MemNode
	 * >> Just forward the state to the next node.
	 */
	else if (isa<MemNode>(Node) || isa<SigmaOpNode>(Node) || isa<PHIOpNode>(Node) ) {
		result = getUnionOfPredecessors(Node);
	}
	/*
	 * CallNodes: We do not know the output
	 * >> [-inf, +inf]
	 */
	else if (isa<CallNode>(Node)) {
		result = Range(Min, Max);
	}
	/*
	 * Binary operators: we will do the abstract interpretation
	 * to generate the result
	 */
	else if (BinaryOpNode* BOP = dyn_cast<BinaryOpNode>(Node)) {

		GraphNode* Op1 = BOP->getOperand(0);
		GraphNode* Op2 = BOP->getOperand(1);

		result = abstractInterpretation(out_state[Op1], out_state[Op2], BOP->getBinaryOperator());
	}
	/*
	 * Unary operators: we will do the abstract interpretation
	 * to generate the result
	 */
	else if (UnaryOpNode* UOP = dyn_cast<UnaryOpNode>(Node)) {

		GraphNode* Op = UOP->getOperand();

		result = abstractInterpretation(out_state[Op], UOP->getUnaryInstruction());
	}
	/*
	 * Generic operations: treat individually case by case
	 */
	else if (OpNode* OP = dyn_cast<OpNode>(Node)) {

		Instruction* I = OP->getOperation();

		switch (I->getOpcode()) {
		case Instruction::Store:
			GraphNode* Op = OP->getOperand(0);
			result = abstractInterpretation(out_state[Op], I);
			break;
		}

	}

	return result;
}

Range llvm::RangeAnalysis::getUnionOfPredecessors(GraphNode* Node){
	Range result(Min, Max, Unknown);

	std::map<GraphNode*, edgeType> Preds = Node->getPredecessors();
	std::map<GraphNode*, edgeType>::iterator pred, pred_end;
	for(pred = Preds.begin(), pred_end = Preds.end(); pred != pred_end; pred++){

		if(pred->second == etData){
			result = result.unionWith(out_state[pred->first]);
		}
	}

	return result;
}

/*
 * Abstract interpretation of binary operators
 */
Range llvm::RangeAnalysis::abstractInterpretation(Range Op1, Range Op2, Instruction *I){

	switch(I->getOpcode()){
		case Instruction::Add:  return Op1.add(Op2);
		case Instruction::Sub:  return Op1.sub(Op2);
		case Instruction::Mul:  return Op1.mul(Op2);
		case Instruction::SDiv: return Op1.sdiv(Op2);
		case Instruction::UDiv: return Op1.udiv(Op2);
		case Instruction::SRem: return Op1.srem(Op2);
		case Instruction::URem: return Op1.urem(Op2);
		case Instruction::Shl:  return Op1.shl(Op2);
		case Instruction::AShr: return Op1.ashr(Op2);
		case Instruction::LShr: return Op1.lshr(Op2);
		case Instruction::And:  return Op1.And(Op2);
		case Instruction::Or:   return Op1.Or(Op2);
		case Instruction::Xor:  return Op1.Xor(Op2);
		default:
			errs() << "Unhandled Instruction:" << *I;
			return Range(Min,Max);
	}
}

/*
 * Abstract interpretation of unary operators
 */
Range abstractInterpretation(Range Op1, Instruction *I){

	unsigned bw = I->getType()->getPrimitiveSizeInBits();
	Range result(Min, Max, Unknown);

	if (Op1.isRegular()) {
		switch (I->getOpcode()) {
		case Instruction::Trunc:
			result = Op1.truncate(bw);
			break;
		case Instruction::ZExt:
			result = Op1.zextOrTrunc(bw);
			break;
		case Instruction::SExt:
			result = Op1.sextOrTrunc(bw);
			break;
		case Instruction::Load:
			result = Op1;
			break;
		case Instruction::Store:
			result = Op1;
			break;
		default:
			errs() << "Unhandled UnaryInstruction:" << *I;
			result = Range(Min, Max);
			break;
		}
	} else if (Op1.isEmpty()) {
		result = Range(Min, Max, Empty);
	}

	return result;
}


void fixPointIteration(int SCCid, bool(*latticeOperation)(GraphNode*, Range)){

}


/*
 * Here we do the abstract interpretation.
 *
 * We compute the out_state of a node based in the out_state of
 * the predecessors of the node.
 *
 * We also perform the widening operation as needed.
 *
 * Finally, if the out_state is changed, we add to the worklist the successor nodes.
 */
void llvm::RangeAnalysis::computeNode(GraphNode* Node, std::set<GraphNode*> &Worklist, LatticeOperation lo){

	Range new_out_state = evaluateNode(Node);

	bool hasChanged = (lo == loJoin ? join(Node, new_out_state) : meet(Node, new_out_state));

	if (hasChanged) {
		//The range of this node has changed. Add its successors to the worklist.
		addSuccessorsToWorklist(Node, Worklist);
	}

}

void llvm::RangeAnalysis::addSuccessorsToWorklist(GraphNode* Node, std::set<GraphNode*> &Worklist){

	std::map<GraphNode*, edgeType> succs = Node->getSuccessors();

	std::map<GraphNode*, edgeType>::iterator succ, succ_end;
	for(succ = succs.begin(), succ_end = succs.end(); succ != succ_end; succ++){

		if(succ->second == etData){
			Worklist.insert(succ->first);
		}
	}

}

bool llvm::RangeAnalysis::join(GraphNode* Node, Range new_abstract_state){

	/*
	 * Here we perform the join operation in the interval lattice,
	 * with Cousot's widening operator.
	 */

	Range oldInterval = out_state[Node];
	Range newInterval = new_abstract_state;

	APInt oldLower = oldInterval.getLower();
	APInt oldUpper = oldInterval.getUpper();
	APInt newLower = newInterval.getLower();
	APInt newUpper = newInterval.getUpper();

	if (oldInterval.isUnknown())
		out_state[Node] = newInterval;
	else {
		APInt oldLower = oldInterval.getLower();
		APInt oldUpper = oldInterval.getUpper();
		APInt newLower = newInterval.getLower();
		APInt newUpper = newInterval.getUpper();
		if (newLower.slt(oldLower))
			if (newUpper.sgt(oldUpper))
				out_state[Node] = Range(Min, Max);
			else
				out_state[Node] = Range(Min, oldUpper);
		else if (newUpper.sgt(oldUpper))
			out_state[Node] = Range(oldLower, Max);
	}

	bool hasChanged = oldInterval != out_state[Node];

	return hasChanged;
}

bool llvm::RangeAnalysis::meet(GraphNode* Node, Range new_abstract_state){

	APInt oLower = out_state[Node].getLower();
	APInt oUpper = out_state[Node].getUpper();
	Range newInterval = new_abstract_state;

	APInt nLower = newInterval.getLower();
	APInt nUpper = newInterval.getUpper();

	bool hasChanged = false;

	if (oLower.eq(Min) && nLower.ne(Min)) {
		out_state[Node] = Range(nLower, oUpper);
		hasChanged = true;
	} else {
		APInt smin = APIntOps::smin(oLower, nLower);
		if (oLower.ne(smin)) {
			out_state[Node] = Range(smin, oUpper);
			hasChanged = true;
		}
	}

	if (oUpper.eq(Max) && nUpper.ne(Max)) {
		out_state[Node] = Range(out_state[Node].getLower(), nUpper);
		hasChanged = true;
	} else {
		APInt smax = APIntOps::smax(oUpper, nUpper);
		if (oUpper.ne(smax)) {
			out_state[Node] = Range(out_state[Node].getLower(), smax);
			hasChanged = true;
		}
	}

	return hasChanged;
}

void llvm::RangeAnalysis::fixFutures(int SCCid) {

	SCC_Iterator it(depGraph, SCCid);

	while(it.hasNext()){

		if (SigmaOpNode* Node = dyn_cast<SigmaOpNode>(it.getNext())) {

			if (branchConstraints.count(Node) > 0) {

				if (SymbInterval* SI = dyn_cast<SymbInterval>(&(branchConstraints[Node]))) {

					GraphNode* ControlDep = Node->getOperand(0, etControl);

					SI->fixIntersects(out_state[ControlDep]);

					out_state[Node] = out_state[Node].intersectWith(SI->getRange());

				}

			}

		}

	}

}

void llvm::RangeAnalysis::narrowingAnalysis(int SCCid) {

	fixPointIteration(SCCid, loMeet);

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

void llvm::RangeAnalysis::fixPointIteration(int SCCid, LatticeOperation lo) {

	SCC_Iterator it(depGraph, SCCid);

	std::set<GraphNode*> worklist;

	while(it.hasNext()){

		GraphNode* node = it.getNext();
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

		computeNode(currentNode, worklist, lo);

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
