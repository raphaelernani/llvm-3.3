/*
 * BranchAnalysis.cpp
 *
 *  Created on: Mar 21, 2014
 *      Author: raphael
 */

#ifndef DEBUG_TYPE
#define DEBUG_TYPE "BranchAnalysis"
#endif

#include "BranchAnalysis.h"

using namespace llvm;

STATISTIC(numBranchesAnalyzed,"Number of Conditional Branches Analyzed");
STATISTIC(numSwitchesAnalyzed,"Number of Switches Analyzed");

void BranchAnalysis::buildValueSwitchMap(const SwitchInst *sw) {
	const Value *condition = sw->getCondition();

	// Verify conditions
	const Type* opType = sw->getCondition()->getType();
	if (!opType->isIntegerTy()) {
		return;
	}

	numSwitchesAnalyzed++;

	SmallVector<std::pair<BasicInterval*, const BasicBlock*>, 4> BBsuccs;

	// Treat when condition of switch is a cast of the real condition (same thing as in buildValueBranchMap)
	const CastInst *castinst = NULL;
	const Value *Op0_0 = NULL;
	if ((castinst = dyn_cast<CastInst>(condition))) {
		Op0_0 = castinst->getOperand(0);
	}

	// Handle 'default', if there is any
	BasicBlock *succ = sw->getDefaultDest();

	if (succ) {
		APInt sigMin = Min;
		APInt sigMax = Max;

		Range Values = Range(sigMin, sigMax);

		// Create the interval using the intersection in the branch.
		BasicInterval* BI = new BasicInterval(Values);

		BBsuccs.push_back(std::make_pair(BI, succ));
	}

	// Handle the rest of cases
	for(SwitchInst::ConstCaseIt CI = sw->case_begin(); CI != sw->case_end(); CI++){
		if (CI == sw->case_default()) continue;

		const ConstantInt *constant = CI.getCaseValue();

		APInt sigMin = constant->getValue();
		APInt sigMax = sigMin;

		if (sigMin.getBitWidth() < MAX_BIT_INT) {
			sigMin = sigMin.sext(MAX_BIT_INT);
		}
		if (sigMax.getBitWidth() < MAX_BIT_INT) {
			sigMax = sigMax.sext(MAX_BIT_INT);
		}

//		if (sigMax.slt(sigMin)) {
//			sigMax = APInt::getSignedMaxValue(MAX_BIT_INT);
//		}

		Range Values = Range(sigMin, sigMax);

		// Create the interval using the intersection in the branch.
		BasicInterval* BI = new BasicInterval(Values);

		BBsuccs.push_back(std::make_pair(BI, succ));

	}

	ValueSwitchMap* VSM = new ValueSwitchMap(condition, BBsuccs);
	//valuesSwitchMap.insert(std::make_pair(condition, VSM));

	IntervalConstraints[condition].push_back(VSM);

	if (Op0_0) {
		ValueSwitchMap* VSM_0 = new ValueSwitchMap(Op0_0, BBsuccs);
		IntervalConstraints[Op0_0].push_back(VSM_0);
	}
}

void BranchAnalysis::buildValueBranchMap(const BranchInst *br) {
	// Verify conditions
	if (!br->isConditional())
		return;

	ICmpInst *ici = dyn_cast<ICmpInst>(br->getCondition());
	if (!ici)
		return;

	const Type* op0Type = ici->getOperand(0)->getType();
	const Type* op1Type = ici->getOperand(1)->getType();
	if (!op0Type->isIntegerTy() || !op1Type->isIntegerTy()) {
		return;
	}

	numBranchesAnalyzed++;

	// Gets the successors of the current basic block.
	const BasicBlock *TBlock = br->getSuccessor(0);
	const BasicBlock *FBlock = br->getSuccessor(1);

	// We have a Variable-Constant comparison.
	if (ConstantInt * CI = dyn_cast<ConstantInt>(ici->getOperand(1))) {

		// Calculate the range of values that would satisfy the comparison.
		ConstantRange CR(CI->getValue(), CI->getValue() + 1);
		unsigned int pred = ici->getPredicate();

		ConstantRange tmpT = ConstantRange::makeICmpRegion(pred, CR);
		APInt sigMin = tmpT.getSignedMin();
		APInt sigMax = tmpT.getSignedMax();

		if (sigMin.getBitWidth() < MAX_BIT_INT) {
			sigMin = sigMin.sext(MAX_BIT_INT);
		}
		if (sigMax.getBitWidth() < MAX_BIT_INT) {
			sigMax = sigMax.sext(MAX_BIT_INT);
		}

		if (sigMax.slt(sigMin)) {
			sigMax = Max;
		}

		Range TValues = Range(sigMin, sigMax);

		// If we're interested in the false dest, invert the condition.
		ConstantRange tmpF = tmpT.inverse();
		sigMin = tmpF.getSignedMin();
		sigMax = tmpF.getSignedMax();

		if (sigMin.getBitWidth() < MAX_BIT_INT) {
			sigMin = sigMin.sext(MAX_BIT_INT);
		}
		if (sigMax.getBitWidth() < MAX_BIT_INT) {
			sigMax = sigMax.sext(MAX_BIT_INT);
		}

		if (sigMax.slt(sigMin)) {
			sigMax = Max;
		}

		Range FValues = Range(sigMin, sigMax);

		// Create the interval using the intersection in the branch.
		BasicInterval* BT = new BasicInterval(TValues);
		BasicInterval* BF = new BasicInterval(FValues);

		errs() << br << " " << BT << " " << BF << "\n";

		const Value *Op0 = ici->getOperand(0);
		ValueBranchMap* VBM = new ValueBranchMap(Op0, TBlock, FBlock, BT, BF);

		IntervalConstraints[Op0].push_back(VBM);
		//valuesBranchMap.insert(std::make_pair(Op0, VBM));

		// Do the same for the operand of Op0 (if Op0 is a cast instruction)
		const CastInst *castinst = NULL;
		if ((castinst = dyn_cast<CastInst>(Op0))) {
			const Value *Op0_0 = castinst->getOperand(0);

			BasicInterval* BT = new BasicInterval(TValues);
			BasicInterval* BF = new BasicInterval(FValues);

			ValueBranchMap* VBM = new ValueBranchMap(Op0_0, TBlock, FBlock, BT, BF);

			IntervalConstraints[Op0_0].push_back(VBM);
			//valuesBranchMap.insert(std::make_pair(Op0_0, VBM));
		}
	} else {
		// Create the interval using the intersection in the branch.
		CmpInst::Predicate pred = ici->getPredicate();
		CmpInst::Predicate invPred = ici->getInversePredicate();

		Range CR(Min, Max, Unknown);

		const Value* Op1 = ici->getOperand(1);

		// Symbolic intervals for op0
		const Value* Op0 = ici->getOperand(0);
		SymbInterval* STOp0 = new SymbInterval(CR, Op1, pred);
		SymbInterval* SFOp0 = new SymbInterval(CR, Op1, invPred);

		errs() << br << " " << STOp0 << " " << SFOp0 << "\n";

		ValueBranchMap* VBMOp0 = new ValueBranchMap(Op0, TBlock, FBlock, STOp0, SFOp0);
		IntervalConstraints[Op0].push_back(VBMOp0);
		//valuesBranchMap.insert(std::make_pair(Op0, VBMOp0));

		// Symbolic intervals for operand of op0 (if op0 is a cast instruction)
		const CastInst *castinst = NULL;
		if ((castinst = dyn_cast<CastInst>(Op0))) {
			const Value* Op0_0 = castinst->getOperand(0);

			SymbInterval* STOp1_1 = new SymbInterval(CR, Op1, pred);
			SymbInterval* SFOp1_1 = new SymbInterval(CR, Op1, invPred);

			errs() << br << " " << STOp1_1 << " " << SFOp1_1 << "\n";

			ValueBranchMap* VBMOp1_1 = new ValueBranchMap(Op0_0, TBlock, FBlock, STOp1_1, SFOp1_1);
			IntervalConstraints[Op0_0].push_back(VBMOp1_1);
			//valuesBranchMap.insert(std::make_pair(Op0_0, VBMOp1_1));
		}

		// Symbolic intervals for op1
		SymbInterval* STOp1 = new SymbInterval(CR, Op0, invPred);
		SymbInterval* SFOp1 = new SymbInterval(CR, Op0, pred);

		errs() << br << " " << STOp1 << " " << SFOp1 << "\n";

		ValueBranchMap* VBMOp1 = new ValueBranchMap(Op1, TBlock, FBlock, STOp1, SFOp1);
		IntervalConstraints[Op1].push_back(VBMOp1);
		//valuesBranchMap.insert(std::make_pair(Op1, VBMOp1));

		// Symbolic intervals for operand of op1 (if op1 is a cast instruction)
		castinst = NULL;
		if ((castinst = dyn_cast<CastInst>(Op1))) {
			const Value* Op0_0 = castinst->getOperand(0);

			SymbInterval* STOp1_1 = new SymbInterval(CR, Op1, pred);
			SymbInterval* SFOp1_1 = new SymbInterval(CR, Op1, invPred);

			errs() << br << " " << STOp1_1 << " " << SFOp1_1 << "\n";

			ValueBranchMap* VBMOp1_1 = new ValueBranchMap(Op0_0, TBlock, FBlock, STOp1_1, SFOp1_1);
			IntervalConstraints[Op0_0].push_back(VBMOp1_1);
			//valuesBranchMap.insert(std::make_pair(Op0_0, VBMOp1_1));
		}
	}
}


bool BranchAnalysis::doInitialization(Module &M){

	numSwitchesAnalyzed = 0;
	numBranchesAnalyzed = 0;

	//We do not change the program; return false
	return false;
}

BranchAnalysis::~BranchAnalysis(){

	std::map<const Value*, std::list<ValueSwitchMap*> >::iterator it, it_end;

	for(it = IntervalConstraints.begin(), it_end = IntervalConstraints.end(); it != it_end; it++){

		std::list<ValueSwitchMap*>::iterator l_it, l_end;
		for(l_it = it->second.begin(), l_end = it->second.end(); l_it != l_end; l_it++){
			ValueSwitchMap* VSM = *l_it;
			delete VSM;
		}
	}
}

bool BranchAnalysis::runOnFunction(Function& F){

	for (Function::const_iterator iBB = F.begin(), eBB = F.end(); iBB != eBB;
			++iBB) {
		const TerminatorInst* ti = iBB->getTerminator();
		const BranchInst* br = dyn_cast<BranchInst>(ti);
		const SwitchInst* sw = dyn_cast<SwitchInst>(ti);

		if (br) {
			buildValueBranchMap(br);
		} else if (sw) {
			buildValueSwitchMap(sw);
		}
	}

	//We do not change the program; return false
	return false;
}


char BranchAnalysis::ID = 0;
static RegisterPass<BranchAnalysis> X("branch-analysis",
		"Branch Analysis");
