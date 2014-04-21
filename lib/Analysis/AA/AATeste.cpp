/*
 * AATeste.cpp
 *
 *  Created on: Apr 15, 2014
 *      Author: raphael
 */

#include "AATeste.h"

namespace llvm {

	char AATeste::ID = 0;
	static RegisterPass<AATeste> X("aa-teste",
			"AATeste");

} /* namespace llvm */
