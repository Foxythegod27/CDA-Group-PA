#include "spimcore.h"


/* ALU */
/* 10 Points */
void ALU(unsigned A,unsigned B,char ALUControl,unsigned *ALUresult,char *Zero)
{

}

int instruction_fetch(unsigned PC, unsigned *Mem, unsigned *instruction)
{
    if (PC % 4 != 0 || PC > 0xFFFF) // Check word alignment and memory bounds
        return 1;

    *instruction = Mem[PC >> 2]; // Fetch instruction
    return 0;
}

/* instruction partition */
/* 10 Points */
void instruction_partition(unsigned instruction, unsigned *op, unsigned *r1, unsigned *r2, unsigned *r3, unsigned *funct, unsigned *offset, unsigned *jsec)
{
    *op = (instruction >> 26) & 0x3F;
    *r1 = (instruction >> 21) & 0x1F;
    *r2 = (instruction >> 16) & 0x1F;
    *r3 = (instruction >> 11) & 0x1F;
    *funct = instruction & 0x3F;
    *offset = instruction & 0xFFFF;
    *jsec = instruction & 0x03FFFFFF;
}

/* instruction decode */
/* 15 Points */
int instruction_decode(unsigned op, struct_controls *controls)
{
    // Initialize control signals
    controls->RegDst = controls->ALUSrc = controls->MemtoReg = 0;
    controls->RegWrite = controls->MemRead = controls->MemWrite = controls->Branch = 0;
    controls->Jump = controls->ALUOp = 0;

    switch (op) {
        case 0x00: // R-type
            controls->RegDst = 1;
            controls->RegWrite = 1;
            controls->ALUOp = 7;
            break;
        case 0x23: // lw
            controls->ALUSrc = 1;
            controls->MemtoReg = 1;
            controls->RegWrite = 1;
            controls->MemRead = 1;
            break;
        case 0x2B: // sw
            controls->ALUSrc = 1;
            controls->MemWrite = 1;
            break;
        case 0x04: // beq
            controls->Branch = 1;
            controls->ALUOp = 1;
            break;
        case 0x02: // jump
            controls->Jump = 1;
            break;
        default:
            return 1; // Halt on illegal instruction
    }
    return 0;
}

/* Read Register */
/* 5 Points */
void read_register(unsigned r1, unsigned r2, unsigned *Reg, unsigned *data1, unsigned *data2)
{
    *data1 = Reg[r1];
    *data2 = Reg[r2];
}

/* Sign Extend */
/* 10 Points */
void sign_extend(unsigned offset, unsigned *extended_value)
{
    *extended_value = (offset & 0x8000) ? (offset | 0xFFFF0000) : offset;
}

/* ALU operations */
/* 10 Points */
int ALU_operations(unsigned data1, unsigned data2, unsigned extended_value, unsigned funct, char ALUOp, char ALUSrc, unsigned *ALUresult, char *Zero)
{
    unsigned operand2 = (ALUSrc) ? extended_value : data2;
    char control = ALUOp;

    if (ALUOp == 7) { // R-type
        switch (funct) {
            case 32: control = 0; break; // add
            case 34: control = 1; break; // sub
            case 42: control = 2; break; // slt
            case 43: control = 3; break; // sltu
            default: return 1; // Halt on illegal funct
        }
    }
    ALU(data1, operand2, control, ALUresult, Zero);
    return 0;
}

/* Read / Write Memory */
/* 10 Points */
int rw_memory(unsigned ALUresult, unsigned data2, char MemWrite, char MemRead, unsigned *memdata, unsigned *Mem)
{
    if (ALUresult % 4 != 0 || ALUresult > 0xFFFF)
        return 1;

    if (MemRead)
        *memdata = Mem[ALUresult >> 2];
    if (MemWrite)
        Mem[ALUresult >> 2] = data2;

    return 0;
}

/* Write Register */
/* 10 Points */
void write_register(unsigned r2, unsigned r3, unsigned memdata, unsigned ALUresult, char RegWrite, char RegDst, char MemtoReg, unsigned *Reg)
{
    if (RegWrite) {
        unsigned write_reg = (RegDst) ? r3 : r2;
        unsigned write_data = (MemtoReg) ? memdata : ALUresult;
        Reg[write_reg] = write_data;
    }
}

/* PC update */
/* 10 Points */
void PC_update(unsigned jsec, unsigned extended_value, char Branch, char Jump, char Zero, unsigned *PC)
{
    *PC += 4;

    if (Branch && Zero)
        *PC += (extended_value << 2);
    if (Jump)
        *PC = (jsec << 2) | (*PC & 0xF0000000);
}

