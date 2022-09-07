#include <stdio.h>
#include <stdlib.h>
#include <math.h>
unsigned short BR = 0xF000, ADD = 0xE000, LD = 0xD000, ST = 0xC000, JSR = 0xB000, AND = 0xA000, LDR = 0x9000, STR = 0x8000, NOT = 0x6000, LDI = 0x5000, STI = 0x4000, JMP = 0x3000, LEA = 0x1000;
unsigned short mem[65024];
unsigned short reg[8];
unsigned short PC, PSR, IR;
unsigned short ORIG;
unsigned short BEN;
void boot();
unsigned short stob(char *p);
void readin(char *p, unsigned short ORIG);
void execute();
int main()
{
    char *p = (char *)malloc(sizeof(char) * 17);
    scanf("%s", p);
    ORIG = stob(p);
    boot();
    readin(p, ORIG);
    execute();
    for (unsigned short i = 0; i < 8; i++)
    {
        printf("R%u = x%04hX\n", i, reg[i]);
    }
}
void boot()
{
    for (unsigned short i = 0; i < 65024; i++)
    {
        mem[i] = 0x7777;
    }
    for (unsigned short i = 0; i < 8; i++)
    {
        reg[i] = 0x7777;
    }
    PC = ORIG;
    PSR = 0x8002;
    IR = 0x7777;
}
unsigned short stob(char *p)
{
    unsigned short sum = 0;
    for (unsigned short i = 0; i < 16; i++)
    {
        sum += (p[i] - '0') * pow(2, 15 - i);
    }
    return sum;
}
void readin(char *p, unsigned short ORIG)
{
    unsigned short i = ORIG;
    while (~scanf("%s", p))
    {
        mem[i++] = stob(p);
    }
}
unsigned short bit(unsigned short num, unsigned short id)
{
    return (num >> id) % 2;
}
unsigned short DR(unsigned short num)
{
    unsigned short mask = 0x0E00;
    num &= mask;
    num = num >> 9;
    return num;
}
unsigned short SR1(unsigned short num)
{
    unsigned short mask = 0x01C0;
    num &= mask;
    num = num >> 6;
    return num;
}
unsigned short SR2(unsigned short num)
{
    unsigned short mask = 0x0007;
    num &= mask;
    return num;
}
unsigned short imm5(unsigned short num)
{
    unsigned short mask = 0x001F;
    num &= mask;
    if (bit(num, 4))
    {
        num |= ~mask;
    }
    return num;
}
unsigned short offset9(unsigned short num)
{
    unsigned short mask = 0x01FF;
    num &= mask;
    if (bit(num, 8))
    {
        num |= ~mask;
    }
    return num;
}
unsigned short offset11(unsigned short num)
{
    unsigned short mask = 0x07FF;
    num &= mask;
    if (bit(num, 10))
    {
        num |= ~mask;
    }
    return num;
}
unsigned short offset6(unsigned short num)
{
    unsigned short mask = 0x003F;
    num &= mask;
    if (bit(num, 5))
    {
        num |= ~mask;
    }
    return num;
}
void setcc()
{
    int num = reg[DR(IR)];
    if (bit(num, 15))
    {
        PSR = 0x8004;
    }
    else if (num)
    {
        PSR = 0x8001;
    }
    else
    {
        PSR = 0x8002;
    }
}
void execute()
{
    IR = mem[PC++];
    BEN = (bit(IR, 11) & bit(PSR, 2)) + (bit(IR, 10) & bit(PSR, 1)) + (bit(IR, 9) & bit(PSR, 0));
    if ((IR & BR) == 0)
    {
        if (BEN)
        {
            PC += offset9(IR);
        }
    }
    else if ((IR & ADD) == 0)
    {
        if (bit(IR, 5))
        {
            reg[DR(IR)] = reg[SR1(IR)] + imm5(IR);
        }
        else
        {
            reg[DR(IR)] = reg[SR1(IR)] + reg[SR2(IR)];
        }
        setcc();
    }
    else if ((IR & LD) == 0)
    {
        reg[DR(IR)] = mem[(unsigned short)(PC + offset9(IR))];
        setcc();
    }
    else if ((IR & ST) == 0)
    {
        mem[(unsigned short)(PC + offset9(IR))] = reg[DR(IR)];
    }
    else if ((IR & JSR) == 0)
    {
        reg[7] = PC;
        if (bit(IR, 11))
        {
            PC += offset11(IR);
        }
        else
        {
            PC = reg[SR1(IR)];
        }
    }
    else if ((IR & AND) == 0)
    {
        if (bit(IR, 5))
        {
            reg[DR(IR)] = reg[SR1(IR)] & imm5(IR);
        }
        else
        {
            reg[DR(IR)] = reg[SR1(IR)] & reg[SR2(IR)];
        }
        setcc();
    }
    else if ((IR & LDR) == 0)
    {
        reg[DR(IR)] = mem[(reg[SR1(IR)] + offset6(IR))];
        setcc();
    }
    else if ((IR & STR) == 0)
    {
        mem[(reg[SR1(IR)] + offset6(IR))] = reg[DR(IR)];
    }
    else if ((IR & NOT) == 0)
    {
        reg[DR(IR)] = ~reg[SR1(IR)];
        setcc();
    }
    else if ((IR & LDI) == 0)
    {
        reg[DR(IR)] = mem[mem[(PC + offset9(IR))]];
        setcc();
    }
    else if ((IR & STI) == 0)
    {
        mem[mem[(PC + offset9(IR))]] = reg[DR(IR)];
    }
    else if ((IR & JMP) == 0)
    {
        PC = reg[SR1(IR)];
    }
    else if ((IR & LEA) == 0)
    {
        reg[DR(IR)] = PC + offset9(IR);
    }
    else
    {
        return;
    }
    execute();
}
