#ifndef __RANGED_ALIAS_SETS_H__
#define __RANGED_ALIAS_SETS_H__
#include "llvm/Pass.h"
#include "llvm/IR/Instruction.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/Module.h"
#include "llvm/Support/Casting.h"
#include "llvm/Support/InstIterator.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/ADT/DenseMap.h"
#include "llvm/ADT/APInt.h"
#include <set>
#include <cstdlib>
#include <cstring>
#include <vector>
#include "../../AliasSets/AliasSets.h"
#include "../../RangeAnalysis/RangeAnalysis/RangeAnalysis.h"
using namespace std;

namespace llvm {

class RangedAliasSets: public ModulePass{
	private:
	bool isPresent(Instruction* verify, std::vector<Instruction*> arr);
	std::vector<Instruction*> orderInstructions(std::vector<Instruction*> unordered, Module* M);
	struct MemRange {
		Value* mem;
		APInt lower;
		APInt higher;
		MemRange(Value* Mem, APInt Lower, APInt Higher){mem = Mem; lower = Lower; higher = Higher;}
		static MemRange* FindByValue(Value* element, std::set<MemRange*> MRSet);
	};
	llvm::DenseMap<int, std::set<MemRange*> > MemRangeSetsF;
	llvm::DenseMap<int, std::set<MemRange*> > RangedAliasSetsF;
	llvm::DenseMap<int, std::set<Value*> > NewAliasSetsF;
	public:
	static char ID;
	RangedAliasSets() : ModulePass(ID) {}
	bool runOnModule(Module &M);
	void getAnalysisUsage(AnalysisUsage &AU) const;
	llvm::DenseMap<int, std::set<MemRange*> > getRangedAliasSets();
	llvm::DenseMap<int, std::set<Value*> > getAliasSets();
};

}
#endif
