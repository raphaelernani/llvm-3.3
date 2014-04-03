#define DEBUG_TYPE "ranged-alias-sets"
#include "RangedAliasSets.h"

using namespace llvm;
// These macros are used to get stats regarding the precision of our analysis.
STATISTIC(NAliasSets, "Number of original alias sets");
STATISTIC(NInterestingSets, "Number of alias sets that were divided");
STATISTIC(NRangedSets, "Number of ranged alias sets found");
STATISTIC(NNewSets, "Number of alias sets found from the divided");
STATISTIC(NFinalSets, "Number of final alias sets");
//Global Variables Declarations
extern APInt Min;
extern APInt Max;
extern APInt Zero;
//////////

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
llvm::DenseMap<int, std::set<RangedAliasSets::MemRange*> > RangedAliasSets::getRangedAliasSets(){return RangedAliasSetsF;}
llvm::DenseMap<int, std::set<Value*> > RangedAliasSets::getAliasSets(){return NewAliasSetsF;}
/////////////////////////////////////////////////
/////////////////////////////////////////////////
/////////////////////////////////////////////////
/////////////////////////////////////////////////
/////////////////////////////////////////////////
/////////////////////////////////////////////////
/////////////////////////////////////////////////
bool RangedAliasSets::runOnModule(Module &M){
	//Does range analysis
	InterProceduralRA<Cousot> &ra = getAnalysis<InterProceduralRA<Cousot> >();
	unsigned MaxBitWidth = InterProceduralRA<Cousot>::getMaxBitWidth(M);
	errs() << "Cousot Inter Procedural analysis (Values -> Ranges):\n";
	for(Module::iterator F = M.begin(), Fe = M.end(); F != Fe; ++F)
	for(Function::iterator bb = F->begin(), bbEnd = F->end(); bb != bbEnd; ++bb){
		for(BasicBlock::iterator I = bb->begin(), IEnd = bb->end(); I != IEnd; ++I){
			const Value *v = &(*I);
			Range r = ra.getRange(v);
			if(!r.isUnknown()){
				r.print(errs());
				I->dump();
			}
		}
	}
	
	AliasSets &AS = getAnalysis<AliasSets>();
	llvm::DenseMap<int, std::set<Value*> > AliasSets = AS.getValueSets();
	NAliasSets = AliasSets.size();//statistics
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
        		else if(isa<CallInst>(**ii))
        			if( strcmp( ((CallInst*)*ii)->getCalledFunction()->getName().data(), "malloc") == 0 
        			or strcmp( ((CallInst*)*ii)->getCalledFunction()->getName().data(), "calloc") == 0
        			or strcmp( ((CallInst*)*ii)->getCalledFunction()->getName().data(), "realloc") == 0	)
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
	NInterestingSets = InterestingSets.size();//statistics
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
	int MemRangeSets_i = 1;
	for (llvm::DenseMap<int, std::vector<Instruction*> >::iterator i = InterestingVectors.begin(), e = InterestingVectors.end(); 
	i != e; ++i){
		std::set<MemRange*> memSet;
		for (std::vector<Instruction*>::iterator ii = i->second.begin(), ee = i->second.end(); 
      ii != ee; ++ii){
      	if(isa<AllocaInst>(**ii)){
      		//Mem Range [0,0]
      		memSet.insert(new MemRange(*ii, Zero, Zero));
         }
         else if(isa<CallInst>(**ii)){
        			if( strcmp( ((CallInst*)*ii)->getCalledFunction()->getName().data(), "malloc") == 0 
        			or strcmp( ((CallInst*)*ii)->getCalledFunction()->getName().data(), "calloc") == 0
        			or strcmp( ((CallInst*)*ii)->getCalledFunction()->getName().data(), "realloc") == 0	)
        				memSet.insert(new MemRange(*ii, Zero, Zero));
        	}
         else if(isa<GetElementPtrInst>(**ii)){
         	//Mem Range basePtrMemRange + indexes range
         	Value* base_ptr = ((GetElementPtrInst*)*ii)->getPointerOperand();
         	MemRange* base_range = MemRange::FindByValue(base_ptr, memSet);
         	//Summing up all indexes
         	APInt lower_range;
         	APInt higher_range;
         	lower_range = base_range->lower;
         	higher_range = base_range->higher;
         	for(User::op_iterator idx = ((GetElementPtrInst*)*ii)->idx_begin(), idxe = ((GetElementPtrInst*)*ii)->idx_end(); idx != idxe; idx++){
         		Value* indx = idx->get();
         		if(isa<ConstantInt>(*indx)){
         			lower_range = (lower_range == Min) ? Min : (lower_range + (((ConstantInt*)indx)->getValue()).sextOrTrunc(lower_range.getBitWidth()) );
         			higher_range = (higher_range == Max) ? Max : (higher_range + (((ConstantInt*)indx)->getValue()).sextOrTrunc(higher_range.getBitWidth()) );
         		}
         		else{
         			Range r = ra.getRange(indx);
               	if(!r.isUnknown()){
               		lower_range = (lower_range == Min || r.getLower() == Min) ? Min : (lower_range + r.getLower()); 
         				higher_range = (higher_range == Max || r.getUpper() == Max) ? Max : (higher_range + r.getUpper());
              		}
         		
         		}
         	}
         	
         	memSet.insert(new MemRange(*ii,lower_range,higher_range));
         }
         else if(isa<BitCastInst>(**ii)){
         	Value* base_ptr = (*ii)->getOperand(0);
         	MemRange* base_range = MemRange::FindByValue(base_ptr, memSet);
         	memSet.insert(new MemRange(*ii,base_range->lower,base_range->higher));
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
	}
	
	//Building Ranged Alias Sets    ---  MemRangeSets RangedAliasSets
	llvm::DenseMap<int, std::set<MemRange*> > RangedAliasSets;
	int RangedAliasSets_i = 1;
	for (llvm::DenseMap<int, std::set<MemRange*> >::iterator i = MemRangeSets.begin(), e = MemRangeSets.end(); 
	i != e; ++i){
		APInt MinusOne = Zero;
		MinusOne--;
		APInt One = Zero;
		One++;
		APInt lower_range = MinusOne;
		APInt higher_range = MinusOne;
		while(true){
			std::set<MemRange*> rangedMemSet;
			//Calculating new lower range
			APInt new_lower_range = Max;
			for (std::set<MemRange*>::iterator ii = i->second.begin(), ee = i->second.end(); 
	      	ii != ee; ++ii){
	      	if((*ii)->higher.sle(higher_range)){
	      		i->second.erase(*ii);
	      		continue;
	      	}
	      	else{
	      		if((*ii)->lower.sle(higher_range)) new_lower_range = higher_range + One;
	      		else if((*ii)->lower.slt(new_lower_range)) new_lower_range = (*ii)->lower;
	      	}
	      }
	      lower_range = new_lower_range;
	      //Calculating new higher range
	      APInt new_higher_range1 = Max;
			APInt new_higher_range2 = Max;
			for (std::set<MemRange*>::iterator ii = i->second.begin(), ee = i->second.end(); 
	      	ii != ee; ++ii){
	      	if((*ii)->higher.slt(new_higher_range1))new_higher_range1 = (*ii)->higher;
	      	
	      	if((*ii)->lower.sgt(lower_range) and (*ii)->lower.slt(new_higher_range2)) 
	      		new_higher_range2 = (*ii)->lower - One;
	      }
	      higher_range = new_higher_range1.slt(new_higher_range2) ? new_higher_range1 : new_higher_range2;
	      if(i->second.empty()) break;
	      //Add members
			for (std::set<MemRange*>::iterator ii = i->second.begin(), ee = i->second.end(); 
      	ii != ee; ++ii){
      		if((*ii)->lower.sle(higher_range)){
      			rangedMemSet.insert(new MemRange((*ii)->mem, lower_range, higher_range) );		
      		}
      	}
      	RangedAliasSets[RangedAliasSets_i] = rangedMemSet;
      	RangedAliasSets_i++;
      }
 	}
 	NRangedSets = RangedAliasSets.size();//statistics
 	//Printing Ranged Alias Sets
	errs() << "\n-------------------------\nRANGED ALIAS SETS:" << "\n";
	
	for(int i = 1; i <= RangedAliasSets.size(); i++){
		errs() << "Set " << i << ":\n";
		for (std::set<MemRange*>::iterator ii = RangedAliasSets[i].begin(), ee = RangedAliasSets[i].end(); 
      ii != ee; ++ii){
      	errs() << *((*ii)->mem) << "   ->   ";
      	errs() << " [" << (*ii)->lower << "," << (*ii)->higher << "]\n";
      }
	}
	
	errs() << "-------------------------\n\n";
	
	//Building new alias sets 
	////Merging appropriate table cells
	llvm::DenseMap<int, std::set<Value*> > NewAliasSets;
	int newi = 1;
	for (std::set<MemRange*>::iterator ii = RangedAliasSets[1].begin(), ee = RangedAliasSets[1].end(); 
      ii != ee; ++ii)
      NewAliasSets[newi].insert((*ii)->mem);
      
   for(int i = 2; i <= RangedAliasSets.size(); i++){
		bool merge = false;
		for (std::set<MemRange*>::iterator ii = RangedAliasSets[i].begin(), ee = RangedAliasSets[i].end(); 
      ii != ee; ++ii){
      	if( NewAliasSets[newi].count((*ii)->mem) ){
      		merge = true;
      		break;
      	}
      }
      
     if(!merge) newi++;
      
     for (std::set<MemRange*>::iterator ii = RangedAliasSets[i].begin(), ee = RangedAliasSets[i].end(); 
     ii != ee; ++ii)
     	NewAliasSets[newi].insert((*ii)->mem);
	}
	NNewSets = NewAliasSets.size();//statistics
	////adding undivided sets
	for(int i = 1; i <= InterestingSets.size(); i++){
		for(int j = 1; j <= AliasSets.size(); j++){
			if(InterestingSets[i] == AliasSets[j])
				AliasSets.erase(j);
		}
	}
	
	for(int i = 1; i <= AliasSets.size(); i++){
		if(!AliasSets[i].empty()){
			newi++;
			NewAliasSets[newi] = AliasSets[i];
		}
	}
	NFinalSets = NewAliasSets.size();//statistics
	
	//Printing New Alias Sets
	errs() << "\n-------------------------\nNEW ALIAS SETS:" << "\n";
	for(int i = 1; i <= NewAliasSets.size(); i++){
		errs() << "Set " << i << ":\n";
		for (std::set<Value*>::iterator ii = NewAliasSets[i].begin(), ee = NewAliasSets[i].end(); 
      ii != ee; ++ii){
      	errs() << **ii << "\n";
      }
	}
	errs() << "-------------------------\n\n";
	/**/
	//Done
	errs() << "\nDONE!!!" << "\n";
	MemRangeSetsF = MemRangeSets;
	RangedAliasSetsF = RangedAliasSets;
	NewAliasSetsF = NewAliasSets;
	
}
void RangedAliasSets::getAnalysisUsage(AnalysisUsage &AU) const
{
	AU.setPreservesAll();
	AU.addRequired<AliasSets>();
   AU.addRequired<InterProceduralRA<Cousot> >();
}
char RangedAliasSets::ID = 0;
static RegisterPass<RangedAliasSets> X("ranged-alias-sets",
"Get alias sets with memory range from pointer analysis pass", false, false);
