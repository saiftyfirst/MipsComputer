#include <iostream>
#include "GlobalBranchPredictor.h"

using namespace std;

GlobalBranchPredictor::GlobalBranchPredictor()
{
    GlobalHistoryRegister = 0;
    totalBranch = 1;
    totalHit = 0;
}

bool GlobalBranchPredictor::branchPredictionDecision()
{
    if(PatternHistoryTable[GlobalHistoryRegister]>1)
        return true;
    else return false;
}

void GlobalBranchPredictor::updatePredictor(bool branchResult, bool branchPredictionDecision)
{
    if(branchResult && (PatternHistoryTable[GlobalHistoryRegister]<3))
        PatternHistoryTable[GlobalHistoryRegister] +=1;
    else if(!branchResult && PatternHistoryTable[GlobalHistoryRegister]>0)
        PatternHistoryTable[GlobalHistoryRegister] -=1;  
    
    GlobalHistoryRegister = (GlobalHistoryRegister<<1) | branchResult;
    totalBranch++;
    if(branchResult == branchPredictionDecision) totalHit++;

}

double GlobalBranchPredictor::hitRatio()
{   
    return 1.0*totalHit/totalBranch;
}

GlobalBranchPredictor::~GlobalBranchPredictor()
{
}