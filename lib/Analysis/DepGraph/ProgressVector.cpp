/*
 * ProgressVector.cpp
 *
 *  Created on: Jan 17, 2014
 *      Author: raphael
 */

#include "ProgressVector.h"

namespace llvm {

ProgressVector::ProgressVector(std::list<Value*> redefinition) {


	bool first = true;
	Value* PreviousValue = NULL;
	Expr PreviousExpr;
	Expr firstExpr;

	for(std::list<Value*>::iterator it = redefinition.begin();  it != redefinition.end(); it++){

		Value* CurrentValue = *it;

		errs() << "Value: " << *CurrentValue << "\n";


		Expr CurrentExpr;

		if (first) {
			firstExpr = Expr(CurrentValue);

			//errs() << "First Expr:" << firstExpr << "\n";

			vecExpr = firstExpr;
		} else {

			if (PHINode* PHI = dyn_cast<PHINode>(CurrentValue)) {
				//This will get the correct incoming value.
				CurrentValue = PreviousValue;
			}

			CurrentExpr = Expr(CurrentValue, 1);

			std::vector<std::pair<Expr, Expr> > vecSubs;
			vecSubs.push_back(std::pair<Expr, Expr>(PreviousExpr, vecExpr));
			CurrentExpr.subs(vecSubs);

			vecExpr = CurrentExpr;
		}

		PreviousExpr = Expr(CurrentValue);
		PreviousValue = CurrentValue;
		first = false;
	}

	//Here we compute the delta
	vecExpr = vecExpr - firstExpr;

	errs() << "Final expr: " << vecExpr << "\n\n";

}

Value* ProgressVector::getUniqueValue(Type* constType) {

	Value* result = NULL;

	if (vecExpr.isNumber()) {
		result = ConstantInt::get(constType, vecExpr.getNumber());
		return result;
	}

	result = vecExpr.getUniqueValue();


}

} /* namespace llvm */
