#ifndef __INSTRUCTIONS_
#define __INSTRUCTIONS_

#include <stdint.h>

//arithmetic and logic
void ADD(uint16_t opcode);
void ADC(uint16_t opcode);
void ADIW(uint16_t opcode);
void SUB(uint16_t opcode);
void SUBI(uint16_t opcode);
void SBC(uint16_t opcode);
void SBCI(uint16_t opcode);
void SBIW(uint16_t opcode);
void AND(uint16_t opcode);
void ANDI(uint16_t opcode);
void OR(uint16_t opcode);
void ORI(uint16_t opcode);
void EOR(uint16_t opcode);
void COM(uint16_t opcode);
void NEG(uint16_t opcode);
void SBR(uint16_t opcode);
void CBR(uint16_t opcode);
void INC(uint16_t opcode);
void DEC(uint16_t opcode);
void TST(uint16_t opcode);
void CLR(uint16_t opcode);
void SER(uint16_t opcode);
void MUL(uint16_t opcode);
void MULS(uint16_t opcode);
void MULSU(uint16_t opcode);
void FMUL(uint16_t opcode);
void FMULS(uint16_t opcode);
void FMULSU(uint16_t opcode);

//branch instructions
void RJMP(uint16_t opcode);
void IJMP(uint16_t opcode);
void JMP(uint16_t opcode);
void RCALL(uint16_t opcode);
void ICALL(uint16_t opcode);
void CALL(uint16_t opcode);
void RET(uint16_t opcode);
void RETI(uint16_t opcode);
void CPSE(uint16_t opcode);
void CP(uint16_t opcode);
void CPC(uint16_t opcode);
void CPI(uint16_t opcode);
void SBRC(uint16_t opcode);
void SBRS(uint16_t opcode);
void SBIC(uint16_t opcode);
void SBIS(uint16_t opcode);
void BRBS(uint16_t opcode);
void BRBC(uint16_t opcode);
void BREQ(uint16_t opcode);
void BRNE(uint16_t opcode);
void BRCS(uint16_t opcode);
void BRCC(uint16_t opcode);
void BRSH(uint16_t opcode);
void BRLO(uint16_t opcode);
void BRMI(uint16_t opcode);
void BRPL(uint16_t opcode);
void BRGE(uint16_t opcode);
void BRLT(uint16_t opcode);
void BRHS(uint16_t opcode);
void BRHC(uint16_t opcode);
void BRTS(uint16_t opcode);
void BRTC(uint16_t opcode);
void BRVS(uint16_t opcode);
void BRVC(uint16_t opcode);
void BRIE(uint16_t opcode);
void BRID(uint16_t opcode);

//bit and bit-test instructions
void SBI(uint16_t opcode);
void CBI(uint16_t opcode);
void LSL(uint16_t opcode);
void LSR(uint16_t opcode);
void ROL(uint16_t opcode);
void ROR(uint16_t opcode);
void ASR(uint16_t opcode);
void SWAP(uint16_t opcode);
void BSET(uint16_t opcode);
void BCLR(uint16_t opcode);
void BST(uint16_t opcode);
void BLD(uint16_t opcode);
void SEC(uint16_t opcode);
void CLC(uint16_t opcode);
void SEN(uint16_t opcode);
void CLN(uint16_t opcode);
void SEZ(uint16_t opcode);
void CLZ(uint16_t opcode);
void SEI(uint16_t opcode);
void CLI(uint16_t opcode);
void SES(uint16_t opcode);
void CLS(uint16_t opcode);
void SEV(uint16_t opcode);
void CLV(uint16_t opcode);
void SET(uint16_t opcode);
void CLT(uint16_t opcode);
void SEH(uint16_t opcode);
void CLH(uint16_t opcode);

//data transfer instructions
void MOV(uint16_t opcode);
void MOVW(uint16_t opcode);
void LDI(uint16_t opcode);
void LD(uint16_t opcode);
void LDD(uint16_t opcode);
void LDS(uint16_t opcode);
void ST(uint16_t opcode);
void STD(uint16_t opcode);
void STS(uint16_t opcode);
void LPM(uint16_t opcode);
void SPM(uint16_t opcode);
void IN(uint16_t opcode);
void OUT(uint16_t opcode);
void PUSH(uint16_t opcode);
void POP(uint16_t opcode);

//MCU control instructions
void NOP(uint16_t opcode);
void SLEEP(uint16_t opcode);
void WDR(uint16_t opcode);
void BREAK(uint16_t opcode);


#endif // __INSTRUCTIONS_
