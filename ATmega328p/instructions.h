#ifndef __INSTRUCTIONS_
#define __INSTRUCTIONS_

#include <stdint.h>

//arithmetic and logic
static inline void ADD(uint32_t opcode);
static inline void ADC(uint32_t opcode);
static inline void ADIW(uint32_t opcode);
static inline void SUB(uint32_t opcode);
static inline void SUBI(uint32_t opcode);
static inline void SBC(uint32_t opcode);
static inline void SBCI(uint32_t opcode);
static inline void SBIW(uint32_t opcode);
static inline void AND(uint32_t opcode);
static inline void ANDI(uint32_t opcode);
static inline void OR(uint32_t opcode);
static inline void ORI(uint32_t opcode);
static inline void EOR(uint32_t opcode);
static inline void COM(uint32_t opcode);
static inline void NEG(uint32_t opcode);
static inline void INC(uint32_t opcode);
static inline void DEC(uint32_t opcode);
static inline void SER(uint32_t opcode);
static inline void MUL(uint32_t opcode);
static inline void MULS(uint32_t opcode);
static inline void MULSU(uint32_t opcode);
static inline void FMUL(uint32_t opcode);
static inline void FMULS(uint32_t opcode);
static inline void FMULSU(uint32_t opcode);

//branch instructions
static inline void RJMP(uint32_t opcode);
static inline void IJMP(uint32_t opcode);
static inline void JMP(uint32_t opcode);
static inline void RCALL(uint32_t opcode);
static inline void ICALL(uint32_t opcode);
static inline void CALL(uint32_t opcode);
static inline void RET(uint32_t opcode);
static inline void RETI(uint32_t opcode);
static inline void CPSE(uint32_t opcode);
static inline void CP(uint32_t opcode);
static inline void CPC(uint32_t opcode);
static inline void CPI(uint32_t opcode);
static inline void SBRC(uint32_t opcode);
static inline void SBRS(uint32_t opcode);
static inline void SBIC(uint32_t opcode);
static inline void SBIS(uint32_t opcode);
static inline void BRBS(uint32_t opcode);
static inline void BRBC(uint32_t opcode);

//bit and bit-test instructions
static inline void SBI(uint32_t opcode);
static inline void CBI(uint32_t opcode);
static inline void LSR(uint32_t opcode);
static inline void ROR(uint32_t opcode);
static inline void ASR(uint32_t opcode);
static inline void SWAP(uint32_t opcode);
static inline void BSET(uint32_t opcode);
static inline void BCLR(uint32_t opcode);
static inline void BST(uint32_t opcode);
static inline void BLD(uint32_t opcode);

//data transfer instructions
static inline void MOV(uint32_t opcode);
static inline void MOVW(uint32_t opcode);
static inline void LDI(uint32_t opcode);
static inline void LD_X(uint32_t opcode);
static inline void LD_Y(uint32_t opcode);
static inline void LD_Z(uint32_t opcode);
static inline void LDS(uint32_t opcode);
static inline void ST_X(uint32_t opcode);
static inline void ST_Y(uint32_t opcode);
static inline void ST_Z(uint32_t opcode);
static inline void STS(uint32_t opcode);
static inline void LPM(uint32_t opcode);
static inline void SPM(uint32_t opcode);
static inline void IN(uint32_t opcode);
static inline void OUT(uint32_t opcode);
static inline void PUSH(uint32_t opcode);
static inline void POP(uint32_t opcode);

//MCU control instructions
static inline void NOP(uint32_t opcode);
static inline void SLEEP(uint32_t opcode);
static inline void WDR(uint32_t opcode);
static inline void BREAK(uint32_t opcode);

//Unknown opcode
static inline void XXX(uint32_t opcode);

#endif // __INSTRUCTIONS_
