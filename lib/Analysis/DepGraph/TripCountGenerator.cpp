/*
 * TripCountGenerator.cpp
 *
 *  Created on: Dec 10, 2013
 *      Author: raphael
 */

#ifndef DEBUG_TYPE
#define DEBUG_TYPE "TripCountGenerator"
#endif

#include "TripCountGenerator.h"

STATISTIC(NumInstrumentedLoops, 	"Number of Instrumented Loops");
STATISTIC(NumUnknownConditions, 	"Number of Unknown Loop Conditions");
STATISTIC(NumNonIntegerConditions, 	"Number of Non-Integer Loop Conditions");
STATISTIC(ExitBlocksNotFound, 		"Number of Exit Blocks not found");

using namespace llvm;

//#define enabledebug

#ifdef enabledebug
#define printdebug(X) do { X } while(false)
#else
#define printdebug(X) do { } while(false)
#endif

bool debugMode;

bool llvm::TripCountGenerator::doInitialization(Module& M) {

	context = &M.getContext();

	NumInstrumentedLoops = 0;
	NumUnknownConditions = 0;
	ExitBlocksNotFound = 0;

	return true;
}

Value* TripCountGenerator::generatePericlesEstimatedTripCount(BasicBlock* header, BasicBlock* entryBlock, Value* Op1, Value* Op2, CmpInst* CI){

	bool isSigned = CI->isSigned();

	BasicBlock* GT = BasicBlock::Create(*context, "", header->getParent(), header);
	BasicBlock* LE = BasicBlock::Create(*context, "", header->getParent(), header);
	BasicBlock* PHI = BasicBlock::Create(*context, "", header->getParent(), header);

	TerminatorInst* T = entryBlock->getTerminator();

	IRBuilder<> Builder(T);

	Value* cmp;

	if (isSigned)
		cmp = Builder.CreateICmpSGT(Op1,Op2,"");
	else
		cmp = Builder.CreateICmpUGT(Op1,Op2,"");

	Builder.CreateCondBr(cmp, GT, LE, NULL);
	T->eraseFromParent();

	/*
	 * estimatedTripCount = |Op1 - Op2|
	 *
	 * We will create the same sub in both GT and in LE blocks, but
	 * with inverted operand order. Thus, the result of the subtraction
	 * will be always positive.
	 */

	Builder.SetInsertPoint(GT);
	Value* sub1;
	if (isSigned) {
		//We create a signed sub
		sub1 = Builder.CreateNSWSub(Op1, Op2);
	} else {
		//We create an unsigned sub
		sub1 = Builder.CreateNUWSub(Op1, Op2);
	}
	Builder.CreateBr(PHI);

	Builder.SetInsertPoint(LE);
	Value* sub2;
	if (isSigned) {
		//We create a signed sub
		sub2 = Builder.CreateNSWSub(Op2, Op1);
	} else {
		//We create an unsigned sub
		sub2 = Builder.CreateNUWSub(Op2, Op1);
	}
	Builder.CreateBr(PHI);

	Builder.SetInsertPoint(PHI);
	PHINode* sub = Builder.CreatePHI(sub2->getType(), 2, "");
	sub->addIncoming(sub1, GT);
	sub->addIncoming(sub2, LE);

	Value* EstimatedTripCount;
	if (isSigned) 	EstimatedTripCount = Builder.CreateSExtOrBitCast(sub, Type::getInt64Ty(*context), "EstimatedTripCount");
	else			EstimatedTripCount = Builder.CreateZExtOrBitCast(sub, Type::getInt64Ty(*context), "EstimatedTripCount");

	switch(CI->getPredicate()){
		case CmpInst::ICMP_UGE:
		case CmpInst::ICMP_ULE:
		case CmpInst::ICMP_SGE:
		case CmpInst::ICMP_SLE:
			{
				Constant* One = ConstantInt::get(EstimatedTripCount->getType(), 1);
				EstimatedTripCount = Builder.CreateAdd(EstimatedTripCount, One);
				break;
			}
		default:
			break;
	}

	//Insert a metadata to identify the instruction as the EstimatedTripCount
	Instruction* i = dyn_cast<Instruction>(EstimatedTripCount);
	MarkAsTripCount(*i);

	Builder.CreateBr(header);

	//Adjust the PHINodes of the loop header accordingly
	for (BasicBlock::iterator it = header->begin(); it != header->end(); it++){
		Instruction* tmp = it;

		if (PHINode* I = dyn_cast<PHINode>(tmp)){
			int i = I->getBasicBlockIndex(entryBlock);
			if (i >= 0){
				I->setIncomingBlock(i,PHI);
			}
		}

	}



	return EstimatedTripCount;
}

Value* TripCountGenerator::getValueAtEntryPoint(Value* source, BasicBlock* loopHeader){

	LoopInfoEx& li = getAnalysis<LoopInfoEx>();
	LoopNormalizerAnalysis& ln = getAnalysis<LoopNormalizerAnalysis>();

	Loop* loop = li.getLoopFor(loopHeader);

	//Option 1: Loop invariant. Return the value itself
	if (loop->isLoopInvariant(source)) return source;

	//Option 2: Sequence of redefinitions with PHI node in the loop header. Return the incoming value from the entry block
	LoopControllersDepGraph& lcd = getAnalysis<LoopControllersDepGraph>();
	GraphNode* node = lcd.depGraph->findNode(source);
	if (!node) return NULL;

	int SCCID = lcd.depGraph->getSCCID(node);
	Graph sccGraph = lcd.depGraph->generateSubGraph(SCCID);
	for(Graph::iterator it =  sccGraph.begin(); it != sccGraph.end(); it++){
		if (VarNode* VN = dyn_cast<VarNode>(*it)) {
			Value* V = VN->getValue();
			if (PHINode* PHI = dyn_cast<PHINode>(V))
				if(PHI->getParent() == loopHeader ) {

					Value* IncomingFromEntry = PHI->getIncomingValueForBlock(ln.entryBlocks[loopHeader]);
					return IncomingFromEntry;

				}
		}
	}

	//Option 3: Sequence of loads/stores in the same memory location. Create load in the entry block and return the loaded value
	//TODO: Implement this option

	//Option 4: unknown. Return NULL
	return NULL;
}


/*
 * Given a natural loop, find the basic block that is more likely block that
 * controls the number of iteration of a loop.
 *
 * In for-loops and while-loops, the loop header is the controller, for instance
 * In repeat-until-loops, the loop controller is a basic block that has a successor
 * 		outside the loop and the loop header as a successor.
 *
 *  Otherwise, return the first exit block
 */
BasicBlock* TripCountGenerator::findLoopControllerBlock(Loop* l){

	BasicBlock* header = l->getHeader();

	SmallVector<BasicBlock*, 2>  exitBlocks;
	l->getExitingBlocks(exitBlocks);

	if (!exitBlocks.size()) {
		errs() << "Empty!\n";
	}

	//Case 1: for/while (the header is an exiting block)
	for (SmallVectorImpl<BasicBlock*>::iterator It = exitBlocks.begin(), Iend = exitBlocks.end(); It != Iend; It++){
		BasicBlock* BB = *It;
		if (BB == header) {
			return BB;
		}
	}

	//Case 2: repeat-until/do-while (the exiting block can branch directly to the header)
	for (SmallVectorImpl<BasicBlock*>::iterator It = exitBlocks.begin(), Iend = exitBlocks.end(); It != Iend; It++){

		BasicBlock* BB = *It;

		//Here we iterate over the successors of BB to check if it is a block that leads the control
		//back to the header.
		for(succ_iterator s = succ_begin(BB); s != succ_end(BB); s++){

			if (*s == header) {
				return BB;
			}
		}

	}

	//Otherwise, return the first exit block
	return *(exitBlocks.begin());
}


bool TripCountGenerator::runOnFunction(Function &F){




	IRBuilder<> Builder(F.getEntryBlock().getTerminator());

	LoopInfoEx& li = getAnalysis<LoopInfoEx>();
	LoopNormalizerAnalysis& ln = getAnalysis<LoopNormalizerAnalysis>();

	for(LoopInfoEx::iterator lit = li.begin(); lit != li.end(); lit++){

		//Indicates if we don't have ways to determine the trip count
		bool unknownTC = false;

		Loop* loop = *lit;

		BasicBlock* header = loop->getHeader();
		BasicBlock* entryBlock = ln.entryBlocks[header];

		/*
		 * Here we are looking for the predicate that stops the loop.
		 *
		 * At this moment, we are only considering loops that are controlled by
		 * integer comparisons.
		 */
		BasicBlock* exitBlock = findLoopControllerBlock(loop);

		if (!exitBlock){

			errs() << *header;

			/*
			 * FIXME: Theoretically, every loop must have at least one exit point.
			 * Understand better the cases in which this is not true.
			 */
			ExitBlocksNotFound++;
			continue;
		}

		TerminatorInst* T = exitBlock->getTerminator();
		BranchInst* BI = dyn_cast<BranchInst>(T);
		ICmpInst* CI = BI ? dyn_cast<ICmpInst>(BI->getCondition()) : NULL;

		Value* Op1 = NULL;
		Value* Op2 = NULL;

		if (!CI) unknownTC = true;
		else {
			Op1 = getValueAtEntryPoint(CI->getOperand(0), header);
			Op2 = getValueAtEntryPoint(CI->getOperand(1), header);


			if((!Op1) || (!Op2) ) {
				NumUnknownConditions++;
				unknownTC = true;
			} else {


				if (!(Op1->getType()->isIntegerTy() && Op2->getType()->isIntegerTy())) {

					/*
					 * We only handle integer loop conditions
					 */
					NumNonIntegerConditions++;
					unknownTC = true;
				} else if (Op1->getType() != Op2->getType()) {

					/*
					 * FIXME: Try to put both operators in the same type
					 */
					NumUnknownConditions++;
					unknownTC = true;
				}


			}



		}



		if(!unknownTC) {

			printdebug(

					errs() << "Op1: " << *Op1 << "\n";
					errs() << "Op2: " << *Op2 << "\n\n";

				);

			generatePericlesEstimatedTripCount(header, entryBlock, Op1, Op2, CI);
		}

		NumInstrumentedLoops++;
	}

	return true;
}

void TripCountGenerator::MarkAsTripCount(Instruction& inst)
{
	std::vector<Value*> vec;
	ArrayRef<Value*> aref(vec);
    inst.setMetadata("TripCount", MDNode::get(*context, aref));
}

bool TripCountGenerator::IsTripCount(Instruction& inst)
{
    return inst.getMetadata("TripCount") != 0;
}

char llvm::TripCountGenerator::ID = 0;
static RegisterPass<TripCountGenerator> X("tc-generator","Trip Count Generator");
