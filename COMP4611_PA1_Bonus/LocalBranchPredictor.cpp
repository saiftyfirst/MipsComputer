#include <iostream>
#include "LocalBranchPredictor.h"

using namespace std;

LocalBranchPredictor::LocalBranchPredictor()
{
    totalBranch = 1;
    totalHit = 0;
}

bool LocalBranchPredictor::branchPredictionDecision(unsigned int PC)
{	
	int registerNo = PC%10;
    if(PatternHistoryTables[registerNo][LocalHistoryRegisters[registerNo]]>1)
        return true;
    else return false;
}

void LocalBranchPredictor::updatePredictor(unsigned int PC, bool branchResult, bool branchPredictionDecision)
{	
	int registerNo = PC%10;
	char patternTableItem = PatternHistoryTables[registerNo][LocalHistoryRegisters[registerNo]];
    if(branchResult && (patternTableItem<3))
        patternTableItem +=1;
    else if(!branchResult && (patternTableItem>0))
        patternTableItem -=1;  

	LocalHistoryRegisters[registerNo] = (LocalHistoryRegisters[registerNo]<<1) | branchResult;
	totalBranch++;
	if(branchResult == branchPredictionDecision) totalHit++;

}

double LocalBranchPredictor::hitRatio()
{
    return 1.0*totalHit/totalBranch;
}

LocalBranchPredictor::~LocalBranchPredictor()
{
}