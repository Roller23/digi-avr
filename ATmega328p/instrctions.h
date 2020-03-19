#ifndef __INSTRUCTIONS_
#define __INSTRUCTIONS_

#include <stdint.h>

//arithmetic and logic
void ADD(uint32_t opcode);
void ADC(uint32_t opcode);
void ADIW(uint32_t opcode);
void SUB(uint32_t opcode);
void SUBI(uint32_t opcode);
void SBC(uint32_t opcode);
void SBCI(uint32_t opcode);
void SBIW(uint32_t opcode);
void AND(uint32_t opcode);
void ANDI(uint32_t opcode);
void OR(uint32_t opcode);
void ORI(uint32_t opcode);
void EOR(uint32_t opcode);
void COM(uint32_t opcode);
void NEG(uint32_t opcode);
void SBR(uint32_t opcode);
void CBR(uint32_t opcode);
void INC(uint32_t opcode);
void DEC(uint32_t opcode);
void TST(uint32_t opcode);
void CLR(uint32_t opcode);
void SER(uint32_t opcode);
void MUL(uint32_t opcode);
void MULS(uint32_t opcode);
void MULSU(uint32_t opcode);
void FMUL(uint32_t opcode);
void FMULS(uint32_t opcode);
void FMULSU(uint32_t opcode);

//branch instructions
void RJMP(uint32_t opcode);
void IJMP(uint32_t opcode);
void JMP(uint32_t opcode);
void RCALL(uint32_t opcode);
void ICALL(uint32_t opcode);
void CALL(uint32_t opcode);
void RET(uint32_t opcode);
void RETI(uint32_t opcode);
void CPSE(uint32_t opcode);
void CP(uint32_t opcode);
void CPC(uint32_t opcode);
void CPI(uint32_t opcode);
void SBRC(uint32_t opcode);
void SBRS(uint32_t opcode);
void SBIC(uint32_t opcode);
void SBIS(uint32_t opcode);
void BRBS(uint32_t opcode);
void BRBC(uint32_t opcode);
void BREQ(uint32_t opcode);
void BRNE(uint32_t opcode);
void BRCS(uint32_t opcode);
void BRCC(uint32_t opcode);
void BRSH(uint32_t opcode);
void BRLO(uint32_t opcode);
void BRMI(uint32_t opcode);
void BRPL(uint32_t opcode);
void BRGE(uint32_t opcode);
void BRLT(uint32_t opcode);
void BRHS(uint32_t opcode);
void BRHC(uint32_t opcode);
void BRTS(uint32_t opcode);
void BRTC(uint32_t opcode);
void BRVS(uint32_t opcode);
void BRVC(uint32_t opcode);
void BRIE(uint32_t opcode);
void BRID(uint32_t opcode);

//bit and bit-test instructions
void SBI(uint32_t opcode);
void CBI(uint32_t opcode);
void LSL(uint32_t opcode);
void LSR(uint32_t opcode);
// void ROL(uint32_t opcode); pseudo instruction for ADC
void ROR(uint32_t opcode);
void ASR(uint32_t opcode);
void SWAP(uint32_t opcode);
void BSET(uint32_t opcode);
void BCLR(uint32_t opcode);
void BST(uint32_t opcode);
void BLD(uint32_t opcode);
void SEC(uint32_t opcode);
void CLC(uint32_t opcode);
void SEN(uint32_t opcode);
void CLN(uint32_t opcode);
void SEZ(uint32_t opcode);
void CLZ(uint32_t opcode);
void SEI(uint32_t opcode);
void CLI(uint32_t opcode);
void SES(uint32_t opcode);
void CLS(uint32_t opcode);
void SEV(uint32_t opcode);
void CLV(uint32_t opcode);
void SET(uint32_t opcode);
void CLT(uint32_t opcode);
void SEH(uint32_t opcode);
void CLH(uint32_t opcode);

//data transfer instructions
void MOV(uint32_t opcode);
void MOVW(uint32_t opcode);
void LDI(uint32_t opcode);
void LD(uint32_t opcode);
void LDD(uint32_t opcode);
void LDS(uint32_t opcode);
void ST(uint32_t opcode);
void STD(uint32_t opcode);
void STS(uint32_t opcode);
void LPM(uint32_t opcode);
void SPM(uint32_t opcode);
void IN(uint32_t opcode);
void OUT(uint32_t opcode);
void PUSH(uint32_t opcode);
void POP(uint32_t opcode);

//MCU control instructions
void NOP(uint32_t opcode);
void SLEEP(uint32_t opcode);
void WDR(uint32_t opcode);
void BREAK(uint32_t opcode);

#endif // __INSTRUCTIONS_