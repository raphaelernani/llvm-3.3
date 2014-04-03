#include "RangedAliasSets.h"

using namespace llvm;

RangedAliasSets::MemRange* RangedAliasSets::MemRange::FindByValue(Value* element, std::set<MemRange*> MRSet){
	for(std::set<MemRange*>::iterator i = MRSet.begin(), e = MRSet.end(); i != e; i++){
		if(element == (*i)->mem)
			return *i;
	}
	return NULL;
}

//Verifies if the instruction verify is present in vector arr
bool RangedAliasSets::isPresent(Instruction* verify, std::vector<Instruction*> arr){
	int i;
	for(std::vector<Instruction*>::iterator it = arr.begin() ; 
	it != arr.end(); it++)
		if(verify == *it) return true;
	return false;
}
//Takes a vector of instructions (ptr) and orders them
std::vector<Instruction*> RangedAliasSets::orderInstructions(std::vector<Instruction*> unordered, 
Module* M){
	std::vector<Instruction*> ordered(unordered.size());
	int i = 0;
	for (Module::iterator F = M->begin(), Fe = M->end(); F != Fe; F++)
	for (inst_iterator I = inst_begin(F), E = inst_end(F); I != E; ++I){
		if(isPresent(&(*I), unordered)){
			ordered[i] = &(*I);
			i++;
		}
	}
	return ordered;
}
/////////////////////////////////////////////////
bool RangedAliasSets::runOnModule(Module &M){
	//Does range analysis
	//InterProceduralRA<Cousot> &ra = getAnalysis<InterProceduralRA<Cousot> >();
	errs() << "Cousot Inter Procedural analysis (Values -> Ranges):\n";
	for(Module::iterator F = M.begin(), Fe = M.end(); F != Fe; ++F)
	for(Function::iterator bb = F->begin(), bbEnd = F->end(); bb != bbEnd; ++bb){
		for(BasicBlock::iterator I = bb->begin(), IEnd = bb->end(); I != IEnd; ++I){
			const Value *v = &(*I);
//			Range r = ra.getRange(v);
//			if(!r.isUnknown()){
//				r.print(errs());
//				I->dump();
//			}
		}
	}
	
/*	AliasSets &AS = getAnalysis<AliasSets>();
	llvm::DenseMap<int, std::set<Value*> > AliasSets = AS.getValueSets();
	//Printing Alias Sets
	errs() << "Alias Sets" << "\n";
	for (llvm::DenseMap<int, std::set<Value*> >::iterator i = AliasSets.begin(), e = AliasSets.end(); 
	i != e; ++i) {
        errs() << "Set " << i->first << " : size? "<< i->second.size() <<"\n";
        for (std::set<Value*>::iterator ii = i->second.begin(), ee = i->second.end(); 
        ii != ee; ++ii) {

            errs() << "	" << **ii <<"  inst? " << isa<Instruction>(**ii) << "\n";
        }

        errs() << "\n";
	}
	
	
	llvm::DenseMap<int, std::set<Value*> > InterestingSets;
	int set_number = 0;
	//Checks for interesting sets who must have more than one element, be comprised of Instructions and just one alloca
	for (llvm::DenseMap<int, std::set<Value*> >::iterator i = AliasSets.begin(), e = AliasSets.end(); 
	i != e; ++i){
		bool non_inst = false;
		//If size of the set is higher than 1
		if(i->second.size() > 1){
			//Need to check if the set represents a continuos memory and only instructions
			int alloca_count = 0;
			for (std::set<Value*>::iterator ii = i->second.begin(), ee = i->second.end(); 
        	ii != ee; ++ii){ 
        		if(isa<AllocaInst>(**ii))
        			alloca_count++;
        		if(!isa<Instruction>(**ii))
        			non_inst = true;
        	}
        	if(alloca_count == 1 and non_inst == false){
        		set_number++;
        		InterestingSets[set_number] = i->second;
        	}
		}
	}
	
	//Printing Insteresting Sets
	errs() << "Insteresting Sets" << "\n";	
	for (llvm::DenseMap<int, std::set<Value*> >::iterator i = InterestingSets.begin(), e = InterestingSets.end(); 
	i != e; ++i) {
        errs() << "Set " << i->first << "\n";
        for (std::set<Value*>::iterator ii = i->second.begin(), ee = i->second.end(); 
        ii != ee; ++ii) {

            errs() << "	" << **ii << "\n";
        }

        errs() << "\n";
	}
	
	//Transforming interesting sets of Value* into interesting vectors of ordered Instruction*
	llvm::DenseMap<int, std::vector<Instruction*> > InterestingVectors;
	int InterestingVectors_i = 1;
	for (llvm::DenseMap<int, std::set<Value*> >::iterator i = InterestingSets.begin(), e = InterestingSets.end(); 
	i != e; ++i){
		std::vector<Instruction*> unordered(i->second.size());
		int vector_i = 0;
		for (std::set<Value*>::iterator ii = i->second.begin(), ee = i->second.end(); 
      ii != ee; ++ii){ 
      	unordered[vector_i] = (Instruction*) *ii;
      	vector_i++;
      }
      InterestingVectors[InterestingVectors_i] = orderInstructions(unordered, &M); 
      InterestingVectors_i++;
	}
	
	//Printing Interesting Vectors
	errs() << "Insteresting Vectors" << "\n";
	for (llvm::DenseMap<int, std::vector<Instruction*> >::iterator i = InterestingVectors.begin(), e = InterestingVectors.end(); 
	i != e; ++i) {
        errs() << "Vector " << i->first << ":\n";
        for (std::vector<Instruction*>::iterator ii = i->second.begin(), ee = i->second.end(); 
        ii != ee; ++ii) {

            errs() << **ii << "\n";
        }

        errs() << "\n";
	}
	
	//Calculating Memory Ranges for each interesting vector
	llvm::DenseMap<int, std::set<MemRange*> > MemRangeSets;
	for (llvm::DenseMap<int, std::vector<Instruction*> >::iterator i = InterestingVectors.begin(), e = InterestingVectors.end(); 
	i != e; ++i){
		int MemRangeSets_i = 1;
		std::set<MemRange*> memSet;
		for (std::vector<Instruction*>::iterator ii = i->second.begin(), ee = i->second.end(); 
      ii != ee; ++ii){
      	if(isa<AllocaInst>(**ii)){
      		//Mem Range [0,0]
      		memSet.insert(new MemRange(*ii));
         }
         if(isa<GetElementPtrInst>(**ii)){
         	//Mem Range basePtrMemRange + indexes range
         	Value* base_ptr = ((GetElementPtrInst*)*ii)->getPointerOperand();
         	MemRange* base_range = MemRange::FindByValue(base_ptr, memSet);
         	memSet.insert(new MemRange(*ii, base_range->lower, base_range->higher));
         }
      }
      MemRangeSets[MemRangeSets_i] = memSet;
      MemRangeSets_i++;
	}
	
	//Printing Memory Ranges
	errs() << "Memory Ranges" << "\n";
	for (llvm::DenseMap<int, std::set<MemRange*> >::iterator i = MemRangeSets.begin(), e = MemRangeSets.end(); 
	i != e; ++i){
		errs() << "Set " << i->first << ":\n";
		for (std::set<MemRange*>::iterator ii = i->second.begin(), ee = i->second.end(); 
      ii != ee; ++ii){
      	errs() << *((*ii)->mem) << "   ->   ";
      	errs() << " [" << (*ii)->lower << "," << (*ii)->higher << "]\n";
      }
	}*/
}
void RangedAliasSets::getAnalysisUsage(AnalysisUsage &AU) const
{
	AU.addRequired<AliasSets>();
	AU.addRequired<InterProceduralRA<Cousot> >();
	AU.setPreservesAll();
}
char RangedAliasSets::ID = 0;
static RegisterPass<RangedAliasSets> X("ranged-alias-sets",
"Get alias sets with memory range from pointer analysis pass", false, false);
