#ifndef __RANGED_ALIAS_SETS_H__
#define __RANGED_ALIAS_SETS_H__
#include "llvm/Pass.h"
#include "llvm/IR/Instruction.h"
#include "llvm/IR/Module.h"
#include "llvm/Support/Casting.h"
#include "llvm/Support/InstIterator.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/ADT/DenseMap.h"
#include <set>
#include <cstdlib>
#include <cstring>
#include <vector>
#include "../DepGraph/AliasSets.h"
#include "../../Transforms/RangeAnalysis/RangeAnalysis.h"
using namespace std;

namespace llvm {

class RangedAliasSets: public ModulePass{
	public:
	static char ID;
	RangedAliasSets() : ModulePass(ID) {}
	bool runOnModule(Module &M);
	void getAnalysisUsage(AnalysisUsage &AU) const;
	private:
	bool isPresent(Instruction* verify, std::vector<Instruction*> arr);
	std::vector<Instruction*> orderInstructions(std::vector<Instruction*> unordered, Module* M);
	struct MemRange {
		Value* mem;
		int lower;
		int higher;
		MemRange(Value* Mem, int Lower = 0, int Higher = 0){mem = Mem; lower = Lower; higher = Higher;}
		static MemRange* FindByValue(Value* element, std::set<MemRange*> MRSet);
	};
};

}
#endif
