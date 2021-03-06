#include <iostream>
#include <cstdio>
#include "MIPSComputer.h"
#include <math.h>

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
    int op, rs, rt, rd, dat, adr, sft, func;
    unsigned int Ins;
    unsigned int oldPC;
    
    bool globalPredictionDecision;
    bool localPredictionDecision;
    bool branchResult;
    //int i =0;
    // while(i<8192){
    //     unsigned int test = int(Memory[i])+ int(Memory[i+1]<<8) + int(Memory[i+2]<<16) + int(Memory[i+3]<<24);
    //     cout<<test<<endl;
    //     i+=4;
    // }
    
    
    Ins = (Memory[PC+3]<<24)
    |(Memory[PC+2]<<16)
    |(Memory[PC+1]<<8)
    | Memory[PC+0]; //Little-Endian
    PC+=4;
    while(Ins){
        
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

        
                // if(op==5){
                //     cout<<"The follwoing is a BNE Instruction"<<endl;
                //     cout<<"Ins :"<<Ins<<endl;
                //     cout<<"rs: "<<rs<<endl;
                //     cout<<"rt: "<<rt<<endl;
                //     cout<<"dat: "<<dat<<endl;
                // }
        

        switch(op){
                //fill in all the missing instructions
            case 0:
                //cout<<Ins<<endl;
                //R type instructions
                //cout<<"(func,sft) - > ("<<func<<","<<sft<<")"<<rs<<","<<rt<<endl;
                if(sft == 0){ dealRTypes(rs,rt,rd,func);}
                else if(func==0) {
                    Reg[rd] = Reg[rt]<< (unsigned int) sft;
                    cout<<"sll"<<endl;
                }
                else if(func==2){  
                    Reg[rd] = Reg[rt]>>(unsigned int) sft;
                    cout<<"srl"<<endl;
                }
                break;
                
            case 8: //ADDI
                //cout<<rs<<" "<<rt<<" "<<dat<<endl;
                Reg[rt] = Reg[rs] + dat;
                //cout<<Reg[rt]<<endl;
                cout<<"addi"<<endl;
                break;
                
            case 35: //LW
                Reg[rt] = Memory[Reg[rs]+dat];
                cout<<"lw"<<endl;
                break;
                
            case 43: //SW
                Memory[Reg[rs]+dat] = Reg[rt];
                cout<<"sw"<<endl;
                break;
                
            case 4: //BEQ
                globalPredictionDecision = globalPredictor.branchPredictionDecision();
                localPredictionDecision = localPredictor.branchPredictionDecision(PC-4);
                oldPC = PC;
                if(Reg[rs] == Reg[rt])
                    PC+=(dat<<2);
                
                branchResult = (Reg[rs]==Reg[rt]);
                globalPredictor.updatePredictor(branchResult, globalPredictionDecision);
                localPredictor.updatePredictor(oldPC-4, branchResult, localPredictionDecision);
                cout<<"beq"<<endl;
                break;
                
            case 5: //BNE
               globalPredictionDecision = globalPredictor.branchPredictionDecision();
                localPredictionDecision = localPredictor.branchPredictionDecision(PC-4);
                oldPC = PC;
                //cout<<rs<<" "<<rt<<" "<<dat<<endl;
                //cout<<"oldPC: "<<oldPC<<endl;
                if(Reg[rs] != Reg[rt])
                    PC+=(dat<<2);
                //cout<<"newPC: "<<PC<<endl;
                branchResult = (Reg[rs]!=Reg[rt]);
                globalPredictor.updatePredictor(branchResult, globalPredictionDecision);
                localPredictor.updatePredictor(oldPC-4, branchResult, localPredictionDecision);
                cout<<"bne"<<endl;
                break;
                
            case 2:
                PC = (PC & 0xf0000000) | adr;
                //cout<<"newPC "<<PC<<endl;
                cout<<"j"<<endl;
                break;
                
            default:
                cout<<op<<"(I or J)Instruction Error!"<<endl;
                return -1;
        }
        Ins = (Memory[PC+3]<<24)
        |(Memory[PC+2]<<16)
        |(Memory[PC+1]<<8)
        | Memory[PC+0]; //Little-Endian
        PC+=4;
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

void MIPSComputer::dealRTypes(int rs,int rt,int rd,int func){
    switch (func) {
        case 0x20:
            Reg[rd] = Reg[rs]+Reg[rt];
            cout<<"add"<<endl;
            break;
            
        case 0x22:
            Reg[rd] = Reg[rs]- Reg[rt];
            cout<<"sub"<<endl;
            break;
            
        case 0x24:
            Reg[rd] = Reg[rs] & Reg[rt];
            cout<<"and"<<endl;
            break;
            
        case 0x25:
            Reg[rd] = Reg[rs] | Reg[rt];
            cout<<"or"<<endl;
            break;
            
        case 0x2A:
            Reg[rd] = Reg[rs] < Reg[rt];
            cout<<"slt"<<endl;
            break;
            
        default:
            break;
    }
    
}




