/*
 * ProgressVector.h
 *
 *  Created on: Jan 17, 2014
 *      Author: raphael
 */

#ifndef PROGRESSVECTOR_H_
#define PROGRESSVECTOR_H_

#include "llvm/IR/Instructions.h"
#include "../SymbolicRA/Range.h"
#include "../SymbolicRA/Expr.h"
#include <vector>

using namespace std;

namespace llvm {

class ProgressVector {
	Expr vecExpr;
public:
	ProgressVector(std::list<Value*> redefinition);
	virtual ~ProgressVector() {}
};

} /* namespace llvm */

#endif /* PROGRESSVECTOR_H_ */
