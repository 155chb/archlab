#include <stdio.h>
#include "shell.h"

/* 定义符号扩展 */
#define SIGN_EXTEND(V, W) ((V) | (((V) & (1 << ((W)-1))) ? 0xFFFFFFFF << (W) : 0))

/* 定义操作码 */
#define OP_REG          0x00
#define OP_BGEZ_BLTZ    0x01
#define OP_J            0x02
#define OP_JAL          0x03
#define OP_BEQ          0x04
#define OP_BNE          0x05
#define OP_BLEZ         0x06
#define OP_BGTZ         0x07
#define OP_ADDI         0x08
#define OP_ADDIU        0x09
#define OP_SLTI         0x0a
#define OP_SLTIU        0x0b
#define OP_ANDI         0x0c
#define OP_ORI          0x0d
#define OP_XORI         0x0e
#define OP_LUI          0x0f
#define OP_LB           0x20
#define OP_LH           0x21
#define OP_LW           0x23
#define OP_LBU          0x24
#define OP_LHU          0x25
#define OP_SB           0x28
#define OP_SH           0x29
#define OP_SW           0x2b

/* 定义功能码 */
#define FUNC_SLL        0x00
#define FUNC_SRL        0x02
#define FUNC_SRA        0x03
#define FUNC_SLLV       0x04
#define FUNC_SRLV       0x06
#define FUNC_SRAV       0x07
#define FUNC_JR         0x08
#define FUNC_JALR       0x09
#define FUNC_SYSCALL    0x0c
#define FUNC_MFHI       0x10
#define FUNC_MTHI       0x11
#define FUNC_MFLO       0x12
#define FUNC_MTLO       0x13
#define FUNC_MULT       0x18
#define FUNC_MULTU      0x19
#define FUNC_DIV        0x1a
#define FUNC_DIVU       0x1b
#define FUNC_ADD        0x20
#define FUNC_ADDU       0x21
#define FUNC_SUB        0x22
#define FUNC_SUBU       0x23
#define FUNC_AND        0x24
#define FUNC_OR         0x25
#define FUNC_XOR        0x26
#define FUNC_NOR        0x27
#define FUNC_SLT        0x2a
#define FUNC_SLTU       0x2b

void process_instruction()
{
    /* execute one instruction here. You should use CURRENT_STATE and modify
     * values in NEXT_STATE. You can call mem_read_32() and mem_write_32() to
     * access memory. */
    NEXT_STATE.PC = CURRENT_STATE.PC + 4;
    uint32_t INST = mem_read_32(CURRENT_STATE.PC);
    uint8_t
        OP = INST >> 26,
        RS = (INST >> 21) & 0x1F,
        RT = (INST >> 16) & 0x1F,
        RD = (INST >> 11) & 0x1F,
        SHAMT = (INST >> 6) & 0x1F,
        FUNC = INST & 0x3F;
    uint16_t IMM = INST & 0xFFFF;
    uint32_t TARGET = INST & 0x03FFFFFF;
    uint64_t TEMP = 0x0000000000000000;
    uint32_t ADDR = 0x00000000;
    if (OP == OP_REG)
    {
        switch (FUNC)
        {
        case FUNC_SLL:
            NEXT_STATE.REGS[RD] = (NEXT_STATE.REGS[RT] << SHAMT) & 0xFFFFFFFF;
            break;
        case FUNC_SRL:
            NEXT_STATE.REGS[RD] = NEXT_STATE.REGS[RT] >> SHAMT;
            break;
        case FUNC_SRA:
            NEXT_STATE.REGS[RD] = SIGN_EXTEND(NEXT_STATE.REGS[RT] >> SHAMT, 32 - SHAMT);
            break;
        case FUNC_SLLV:
            NEXT_STATE.REGS[RD] = (NEXT_STATE.REGS[RT] << NEXT_STATE.REGS[RS]) & 0xFFFFFFFF;
            break;
        case FUNC_SRLV:
            NEXT_STATE.REGS[RD] = NEXT_STATE.REGS[RT] >> NEXT_STATE.REGS[RS];
            break;
        case FUNC_SRAV:
            NEXT_STATE.REGS[RD] = SIGN_EXTEND(NEXT_STATE.REGS[RT] >> NEXT_STATE.REGS[RS], 32 - NEXT_STATE.REGS[RS]);
            break;
        case FUNC_JR:
            NEXT_STATE.PC = CURRENT_STATE.REGS[RS];
            break;
        case FUNC_JALR: /* jalr指令有两种形式，但机器码不需要区分，在编译时就会将rd的数值默认赋值为31 */
            NEXT_STATE.PC = CURRENT_STATE.REGS[RS];
            NEXT_STATE.REGS[RD] = CURRENT_STATE.PC + 4;
            break;
        case FUNC_MFHI:
            NEXT_STATE.REGS[RD] = CURRENT_STATE.HI;
            break;
        case FUNC_MTHI:
            NEXT_STATE.HI = CURRENT_STATE.REGS[RS];
            break;
        case FUNC_MFLO:
            NEXT_STATE.REGS[RD] = CURRENT_STATE.LO;
            break;
        case FUNC_MTLO:
            NEXT_STATE.LO = CURRENT_STATE.REGS[RS];
            break;
        case FUNC_MULT: /* 这里不知道是什么原因，计算结果是当作int32_t进行存储,所以先扩展一下再计算 */
            TEMP = (int64_t)CURRENT_STATE.REGS[RS] * (int64_t)CURRENT_STATE.REGS[RT];
            NEXT_STATE.LO = TEMP & 0xFFFFFFFF;
            NEXT_STATE.HI = TEMP >> 32;
            break;
        case FUNC_MULTU:    /* 同mult */
            TEMP = (uint64_t)CURRENT_STATE.REGS[RS] * (uint64_t)CURRENT_STATE.REGS[RT];
            NEXT_STATE.LO = TEMP & 0xFFFFFFFF;
            NEXT_STATE.HI = TEMP >> 32;
            break;
        case FUNC_DIV:
            NEXT_STATE.LO = (int32_t)CURRENT_STATE.REGS[RS] / (int32_t)CURRENT_STATE.REGS[RT];
            NEXT_STATE.HI = (int32_t)CURRENT_STATE.REGS[RS] % (int32_t)CURRENT_STATE.REGS[RT];
            break;
        case FUNC_DIVU:
            NEXT_STATE.LO = CURRENT_STATE.REGS[RS] / CURRENT_STATE.REGS[RT];
            NEXT_STATE.HI = CURRENT_STATE.REGS[RS] % CURRENT_STATE.REGS[RT];
            break;
        case FUNC_ADD:
            NEXT_STATE.REGS[RD] = CURRENT_STATE.REGS[RS] + CURRENT_STATE.REGS[RT];
            break;
        case FUNC_ADDU:
            NEXT_STATE.REGS[RD] = CURRENT_STATE.REGS[RS] + CURRENT_STATE.REGS[RT];
            break;
        case FUNC_SUB:
            NEXT_STATE.REGS[RD] = CURRENT_STATE.REGS[RS] - CURRENT_STATE.REGS[RT];
            break;
        case FUNC_SUBU:
            NEXT_STATE.REGS[RD] = CURRENT_STATE.REGS[RS] - CURRENT_STATE.REGS[RT];
            break;
        case FUNC_AND:
            NEXT_STATE.REGS[RD] = CURRENT_STATE.REGS[RS] & CURRENT_STATE.REGS[RT];
            break;
        case FUNC_OR:
            NEXT_STATE.REGS[RD] = CURRENT_STATE.REGS[RS] | CURRENT_STATE.REGS[RT];
            break;
        case FUNC_XOR:
            NEXT_STATE.REGS[RD] = CURRENT_STATE.REGS[RS] ^ CURRENT_STATE.REGS[RT];
            break;
        case FUNC_NOR:
            NEXT_STATE.REGS[RD] = ~(CURRENT_STATE.REGS[RS] | CURRENT_STATE.REGS[RT]);
            break;
        case FUNC_SLT:
            NEXT_STATE.REGS[RD] = (int32_t)CURRENT_STATE.REGS[RS] < (int32_t)CURRENT_STATE.REGS[RT] ? 1 : 0;
            break;
        case FUNC_SLTU:
            NEXT_STATE.REGS[RD] = CURRENT_STATE.REGS[RS] < CURRENT_STATE.REGS[RT] ? 1 : 0;
            break;
        case FUNC_SYSCALL:
            if (CURRENT_STATE.REGS[2] == 0x0000000A)
                RUN_BIT = FALSE;
            else ;
            break;
        default:
            printf("Unknown instruction: 0x%08x\n", INST);
            RUN_BIT = FALSE;
            break;
        }
    }
    else
    {
        switch (OP)
        {
        case OP_BGEZ_BLTZ:
            switch (RT)
            {
            case 0x00:  /* BLTZ */
                TARGET = CURRENT_STATE.PC + SIGN_EXTEND(IMM << 2, 18);
                if (CURRENT_STATE.REGS[RS] >> 31 == 1)
                    NEXT_STATE.PC = TARGET;
                else ;
                break;
            case 0x01:  /* BGEZ */
                TARGET = CURRENT_STATE.PC + SIGN_EXTEND(IMM << 2, 18);
                if (CURRENT_STATE.REGS[RS] >> 31 == 0)
                    NEXT_STATE.PC = TARGET;
                else ;
                break;
            case 0x10:  /* BLTZAL */
                TARGET = CURRENT_STATE.PC + SIGN_EXTEND(IMM << 2, 18);
                if (CURRENT_STATE.REGS[RS] >> 31 == 1)
                    NEXT_STATE.PC = TARGET;
                else ;
                NEXT_STATE.REGS[31] = CURRENT_STATE.PC + 4;
                break;
            case 0x11:  /* BGEZAL */
                TARGET = CURRENT_STATE.PC + SIGN_EXTEND(IMM << 2, 18);
                if (CURRENT_STATE.REGS[RS] >> 31 == 0)
                    NEXT_STATE.PC = TARGET;
                else ;
                NEXT_STATE.REGS[31] = CURRENT_STATE.PC + 4;
                break;
            default:
                printf("Unknown instruction: 0x%08x\n", INST);
                RUN_BIT = FALSE;
                break;
            }
            break;
        case OP_J:
            NEXT_STATE.PC = (CURRENT_STATE.PC & 0xF0000000) | (TARGET << 2);
            break;
        case OP_JAL:
            NEXT_STATE.PC = (CURRENT_STATE.PC & 0xF0000000) | (TARGET << 2);
            NEXT_STATE.REGS[31] = CURRENT_STATE.PC + 4;
            break;
        case OP_BEQ:
            TARGET = CURRENT_STATE.PC + SIGN_EXTEND(IMM << 2, 18);
            if (NEXT_STATE.REGS[RS] == NEXT_STATE.REGS[RT])
                NEXT_STATE.PC = TARGET;
            else ;
            break;
        case OP_BNE:
            TARGET = CURRENT_STATE.PC + SIGN_EXTEND(IMM << 2, 18);
            if (NEXT_STATE.REGS[RS] != NEXT_STATE.REGS[RT])
                NEXT_STATE.PC = TARGET;
            else ;
            break;
        case OP_BLEZ:
            TARGET = CURRENT_STATE.PC + SIGN_EXTEND(IMM << 2, 18);
            if (CURRENT_STATE.REGS[RS] >> 31 == 1 ||
                NEXT_STATE.REGS[RS] == 0)
                NEXT_STATE.PC = TARGET;
            else ;
            break;
        case OP_BGTZ:
            TARGET = CURRENT_STATE.PC + SIGN_EXTEND(IMM << 2, 18);
            if (CURRENT_STATE.REGS[RS] >> 31 == 0 &&
                NEXT_STATE.REGS[RS] != 0)
                NEXT_STATE.PC = TARGET;
            else ;
            break;
        case OP_ADDI:
            NEXT_STATE.REGS[RT] = CURRENT_STATE.REGS[RS] + SIGN_EXTEND(IMM, 16);
            break;
        case OP_ADDIU:
            NEXT_STATE.REGS[RT] = CURRENT_STATE.REGS[RS] + SIGN_EXTEND(IMM, 16);
            break;
        case OP_SLTI:
            NEXT_STATE.REGS[RT] = (int32_t)CURRENT_STATE.REGS[RS] < (int32_t)SIGN_EXTEND(IMM, 16) ? 1 : 0;
            break;
        case OP_SLTIU:
            NEXT_STATE.REGS[RT] = CURRENT_STATE.REGS[RS] < SIGN_EXTEND(IMM, 16) ? 1 : 0;
            break;
        case OP_ANDI:   /* 注意：andi的立即数要求是非负整数 */
            NEXT_STATE.REGS[RT] = CURRENT_STATE.REGS[RS] & IMM;
            break;
        case OP_ORI:    /* 同andi */
            NEXT_STATE.REGS[RT] = CURRENT_STATE.REGS[RS] | IMM;
            break;
        case OP_XORI:   /* 同addi */
            NEXT_STATE.REGS[RT] = CURRENT_STATE.REGS[RS] ^ IMM;
            break;
        case OP_LUI:
            NEXT_STATE.REGS[RT] = IMM << 16;
            break;
        case OP_LB:
            TARGET = CURRENT_STATE.REGS[RS] + SIGN_EXTEND(IMM, 16);
            NEXT_STATE.REGS[RT] = SIGN_EXTEND(mem_read_32(TARGET) & 0x000000FF, 8);
            break;
        case OP_LH:
            TARGET = CURRENT_STATE.REGS[RS] + SIGN_EXTEND(IMM, 16);
            NEXT_STATE.REGS[RT] = SIGN_EXTEND(mem_read_32(TARGET) & 0x0000FFFF, 16);
            break;
        case OP_LW:
            TARGET = CURRENT_STATE.REGS[RS] + SIGN_EXTEND(IMM, 16);
            NEXT_STATE.REGS[RT] = mem_read_32(TARGET);
            break;
        case OP_LBU:
            TARGET = CURRENT_STATE.REGS[RS] + SIGN_EXTEND(IMM, 16);
            NEXT_STATE.REGS[RT] = mem_read_32(TARGET) & 0x000000FF;
            break;
        case OP_LHU:
            TARGET = CURRENT_STATE.REGS[RS] + SIGN_EXTEND(IMM, 16);
            NEXT_STATE.REGS[RT] = mem_read_32(TARGET) & 0x0000FFFF;
            break;
        case OP_SB:
            ADDR = CURRENT_STATE.REGS[RS] + SIGN_EXTEND(IMM, 16);
            TARGET = mem_read_32(ADDR);
            TARGET = (TARGET & 0xFFFFFF00) | (NEXT_STATE.REGS[RT] & 0x000000FF);
            mem_write_32(ADDR, TARGET);
            break;
        case OP_SH:
            ADDR = CURRENT_STATE.REGS[RS] + SIGN_EXTEND(IMM, 16);
            TARGET = mem_read_32(ADDR);
            TARGET = (TARGET & 0xFFFF0000) | (NEXT_STATE.REGS[RT] & 0x0000FFFF);
            mem_write_32(ADDR, TARGET);
            break;
        case OP_SW:
            ADDR = CURRENT_STATE.REGS[RS] + SIGN_EXTEND(IMM, 16);
            mem_write_32(ADDR, NEXT_STATE.REGS[RT]);
            break;
        default:
            printf("Unknown instruction: 0x%08x\n", INST);
            RUN_BIT = FALSE;
            break;
        }
    }
}
