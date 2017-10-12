#include <iostream>
#include <cstdio>
#include "MIPSComputer.h"
#include <math.h>

#define RTYPE 1
#define ITYPE 2
#define BTYPE 3 
#define JTYPE 4

using namespace std;

MIPSComputer::MIPSComputer()
{
    Reg[0]=0;
    cout<<"Welcome!"<<endl;
}

void MIPSComputer::boot(char* file)
{
    FILE * f1;
    unsigned int m;
    int i=0;
    
    cout<<"MIPS computer is booting..."<<endl;
    if((f1=fopen(file,"rb"))==NULL){
        cout<<"Cannot open binary file!"<<endl;
        exit(1);
    }
    cout<<"Binary file is ready to load"<<endl;
    do{
        fread(&m,4,1,f1);
        Memory[i++]=(unsigned char)(m&0xFF);
        Memory[i++]=(unsigned char)((m>>8)&0xFF);
        Memory[i++]=(unsigned char)((m>>16)&0xFF);
        Memory[i++]=(unsigned char)((m>>24)&0xFF);
    }while(m);
    cout<<"Memory initialized!"<<endl;
    
    if(fclose(f1)){
        cout<<"Cannot close binary file!"<<endl;
        exit(1);
    }
    
    PC=0;
    cout<<"Boot Done!"<<endl<<endl;
}

int MIPSComputer::run()
{
    cout<<"MIPS computer starts execution..."<<endl;

    unsigned int oldPC;

    fetch();

    while(Ins){
        decode();
        execute();
        memoryAccess();
        
        if(op==0) writeBack(1);
        else if(op==8 || op==35) writeBack(2);
        else if(op==4 || op==5) writeBack(3);
        else if(op==2) writeBack(4);
        else writeBack(-1);
        
        fetch();
    }
    cout<<"All instructions have been executed!"<<endl<<endl;
    return 1;
}

void MIPSComputer::printRegisters()
{
    cout<<"Registers' values:"<<endl<<endl;
    
    for(int i=0;i<32;i++){
        cout<<"Reg"<<i<<": ";
        cout<<dec<<Reg[i];
        cout<<" || ";
        if((i+1)%4==0)
            cout<<endl;
    }
    cout<<endl;
}

void MIPSComputer::printBranchPredictionResult()
{
   cout<<"Hit ratio of global branch predictor: "<<globalPredictor.hitRatio()<<endl;
   cout<<"Hit ratio of local branch predictor: "<<localPredictor.hitRatio()<<endl;
}

MIPSComputer::~MIPSComputer()
{
    cout<<"MIPS computer shuts down!"<<endl;
}


void MIPSComputer::fetch(){
    Ins = (Memory[PC+3]<<24)
    |(Memory[PC+2]<<16)
    |(Memory[PC+1]<<8)
    | Memory[PC+0]; //Little-Endian
    PC+=4;
}

void MIPSComputer::decode(){
    
    //fill in the following values
    op =(Ins>>26)&0x3f; //IR{31..26}
    rs = (Ins>>21)&0x1f; //IR{25..21}
    rt = (Ins>>16)&0x1f; //IR{20..16}
    rd = (Ins>>11)&0x1f; //IR{15..11}
    sft= (Ins>>6)&0x1f; //IR{10..6}
    func= Ins&0x3f; //IR{5..0}
    dat= Ins&0xffff; //IR{15..0}
    adr= (Ins&0x3ffffff)<<2; //IR{25..0}<<2
        
    if(dat > 32767)
        dat -= pow(2,16);
    if(adr > 33554431)
        adr -= pow(2,26);
    if(op==4 || op==5){
        globalPredictionDecision = globalPredictor.branchPredictionDecision();
        localPredictionDecision = localPredictor.branchPredictionDecision(PC-4);
    }

}

void MIPSComputer::execute(){
    bool branchResult;
    switch(op){
        case 0:
            if(sft == 0){ dealRTypes();}
            else if(func==0) {
                executeResult= Reg[rt]<< (unsigned int) sft;
                cout<<"sll"<<endl;
            }
            else if(func==2){  
                executeResult = Reg[rt]>>(unsigned int) sft;
                cout<<"srl"<<endl;
            }
            break;
                
        case 8: //ADDI
            executeResult = Reg[rs] + dat;
            cout<<"addi"<<endl;
            break;
                
        case 35: //LW
            executeResult = Reg[rs]+dat;
            cout<<"lw"<<endl;
            break;
                
        case 43: //SW
            executeResult = Reg[rs]+dat;
            cout<<"sw"<<endl;
            break;
                
        case 4: //BEQ
            executeResult = 0;
            if(Reg[rs] == Reg[rt])
                executeResult = dat<<2;
            branchResult = (Reg[rs]==Reg[rt]);
            globalPredictor.updatePredictor(branchResult, globalPredictionDecision);
            localPredictor.updatePredictor(PC-4, branchResult, localPredictionDecision);
            cout<<"beq"<<endl;
            break;
                
        case 5: //BNE
            executeResult = 0;
            if(Reg[rs] != Reg[rt])
                executeResult = dat<<2;
                //PC+=(dat<<2);
            branchResult = (Reg[rs]!=Reg[rt]);
            globalPredictor.updatePredictor(branchResult, globalPredictionDecision);
            localPredictor.updatePredictor(PC-4, branchResult, localPredictionDecision);
            cout<<"bne"<<endl;
            break;
                
        case 2:
            executeResult = (PC & 0xf0000000) | adr; 
            cout<<"j"<<endl;
            break;
                
        default:
            cout<<op<<"(I or J)Instruction Error!"<<endl;
            break;
        }

}

void MIPSComputer::dealRTypes(){

    switch (func) {
        case 0x20:
            executeResult = Reg[rs]+Reg[rt];
            cout<<"add"<<endl;
            break;
            
        case 0x22:
            executeResult = Reg[rs]- Reg[rt];
            cout<<"sub"<<endl;
            break;
            
        case 0x24:
            executeResult = Reg[rs] & Reg[rt];
            cout<<"and"<<endl;
            break;
            
        case 0x25:
            executeResult = Reg[rs] | Reg[rt];
            cout<<"or"<<endl;
            break;
            
        case 0x2A:
            executeResult = Reg[rs] < Reg[rt];
            cout<<"slt"<<endl;
            break;
            
        default:
            break;
    }
    
}

void MIPSComputer::memoryAccess(){
    if(op==35)
        executeResult = Memory[Reg[rs]+dat];
    else if(op==43)
        Memory[executeResult] = Reg[rt];
}

void MIPSComputer::writeBack(int type){
    switch(type){
        case RTYPE:
            Reg[rd] = executeResult;
            break;
        case ITYPE: 
            Reg[rt] = executeResult;
            break;
        case BTYPE: 
            PC += executeResult;
            break;
        case JTYPE: 
            PC = executeResult;
            break;
        default:
            break;
    }
}


