#ifndef MIPSCOMPUTER_H_
#define MIPSCOMPUTER_H_
#define MAXMEM 8192
#include "GlobalBranchPredictor.h"
#include "LocalBranchPredictor.h"

class MIPSComputer{
private:
    unsigned char Memory[MAXMEM];
    int Reg[32];
    unsigned int PC;
    int op, rs, rt, rd, dat, adr, sft, func;
    unsigned int Ins;
    int executeResult;
    GlobalBranchPredictor globalPredictor;
    LocalBranchPredictor localPredictor;
    bool globalPredictionDecision;
    bool localPredictionDecision;
    void dealRTypes();
    void fetch();
    void decode();
    void execute();
    void memoryAccess();
    void writeBack(int type);
public:
    MIPSComputer();
    void boot(char* file);
    int run();
    void printRegisters();
    void printBranchPredictionResult();
    ~MIPSComputer();
};

#endif  //MIPSCOMPUTER_H_