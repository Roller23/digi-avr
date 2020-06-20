#ifndef __INSTRUCTIONS_
#define __INSTRUCTIONS_

#include <stdint.h>

//arithmetic and logic
static inline void ADD(const uint32_t opcode);
static inline void ADC(const uint32_t opcode);
static inline void ADIW(const uint32_t opcode);
static inline void SUB(const uint32_t opcode);
static inline void SUBI(const uint32_t opcode);
static inline void SBC(const uint32_t opcode);
static inline void SBCI(const uint32_t opcode);
static inline void SBIW(const uint32_t opcode);
static inline void AND(const uint32_t opcode);
static inline void ANDI(const uint32_t opcode);
static inline void OR(const uint32_t opcode);
static inline void ORI(const uint32_t opcode);
static inline void EOR(const uint32_t opcode);
static inline void COM(const uint32_t opcode);
static inline void NEG(const uint32_t opcode);
static inline void INC(const uint32_t opcode);
static inline void DEC(const uint32_t opcode);
static inline void SER(const uint32_t opcode);
static inline void MUL(const uint32_t opcode);
static inline void MULS(const uint32_t opcode);
static inline void MULSU(const uint32_t opcode);
static inline void FMUL(const uint32_t opcode);
static inline void FMULS(const uint32_t opcode);
static inline void FMULSU(const uint32_t opcode);

//branch instructions
static inline void RJMP(const uint32_t opcode);
static inline void IJMP(const uint32_t opcode);
static inline void JMP(const uint32_t opcode);
static inline void RCALL(const uint32_t opcode);
static inline void ICALL(const uint32_t opcode);
static inline void CALL(const uint32_t opcode);
static inline void RET(const uint32_t opcode);
static inline void RETI(const uint32_t opcode);
static inline void CPSE(const uint32_t opcode);
static inline void CP(const uint32_t opcode);
static inline void CPC(const uint32_t opcode);
static inline void CPI(const uint32_t opcode);
static inline void SBRC(const uint32_t opcode);
static inline void SBRS(const uint32_t opcode);
static inline void SBIC(const uint32_t opcode);
static inline void SBIS(const uint32_t opcode);
static inline void BRBS(const uint32_t opcode);
static inline void BRBC(const uint32_t opcode);

//bit and bit-test instructions
static inline void SBI(const uint32_t opcode);
static inline void CBI(const uint32_t opcode);
static inline void LSR(const uint32_t opcode);
static inline void ROR(const uint32_t opcode);
static inline void ASR(const uint32_t opcode);
static inline void SWAP(const uint32_t opcode);
static inline void BSET(const uint32_t opcode);
static inline void BCLR(const uint32_t opcode);
static inline void BST(const uint32_t opcode);
static inline void BLD(const uint32_t opcode);

//data transfer instructions
static inline void MOV(const uint32_t opcode);
static inline void MOVW(const uint32_t opcode);
static inline void LDI(const uint32_t opcode);
static inline void LD_X(const uint32_t opcode);
static inline void LD_Y(const uint32_t opcode);
static inline void LD_Z(const uint32_t opcode);
static inline void LDS(const uint32_t opcode);
static inline void ST_X(const uint32_t opcode);
static inline void ST_Y(const uint32_t opcode);
static inline void ST_Z(const uint32_t opcode);
static inline void STS(const uint32_t opcode);
static inline void LPM(const uint32_t opcode);
static inline void SPM(const uint32_t opcode);
static inline void IN(const uint32_t opcode);
static inline void OUT(const uint32_t opcode);
static inline void PUSH(const uint32_t opcode);
static inline void POP(const uint32_t opcode);

//MCU control instructions
static inline void NOP(const uint32_t opcode);
static inline void SLEEP(const uint32_t opcode);
static inline void WDR(const uint32_t opcode);
static inline void BREAK(const uint32_t opcode);

//Unknown opcode
static inline void XXX(const uint32_t opcode);

#endif // __INSTRUCTIONS_
