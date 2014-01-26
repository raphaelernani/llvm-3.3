/*
 * InstCounterEx.h
 *
 *  Created on: Jan 26, 2014
 *      Author: raphael
 */

#ifndef INSTCOUNTEREX_H_
#define INSTCOUNTEREX_H_

#include "llvm/Pass.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/Instruction.h"
#include "llvm/ADT/Statistic.h"

namespace llvm {

class InstCounterEx: public ModulePass {
private:
	int NumInsts;
public:
	static char ID;

	InstCounterEx() : ModulePass(ID), NumInsts(0){};
	virtual ~InstCounterEx() {};

	bool runOnModule(Module &M);

	virtual void getAnalysisUsage(AnalysisUsage &AU) const{
		AU.setPreservesAll();
	}

	//Number of instructions of the module (of all kinds)
	int getNumInsts() const {
		return NumInsts;
	}
};


class InstCounterEx2: public ModulePass {
private:
	int NumInsts;
public:
	static char ID;

	InstCounterEx2() : ModulePass(ID), NumInsts(0){};
	virtual ~InstCounterEx2() {};

	bool runOnModule(Module &M);

	virtual void getAnalysisUsage(AnalysisUsage &AU) const{
		AU.setPreservesAll();
	}

	//Number of instructions of the module (of all kinds)
	int getNumInsts() const {
		return NumInsts;
	}
};


class InstCounterEx3: public ModulePass {
private:
	int NumInsts;
public:
	static char ID;

	InstCounterEx3() : ModulePass(ID), NumInsts(0){};
	virtual ~InstCounterEx3() {};

	bool runOnModule(Module &M);

	virtual void getAnalysisUsage(AnalysisUsage &AU) const{
		AU.setPreservesAll();
	}

	//Number of instructions of the module (of all kinds)
	int getNumInsts() const {
		return NumInsts;
	}
};


} /* namespace llvm */

#endif /* INSTCOUNTEREX_H_ */
