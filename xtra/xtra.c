#include <assert.h>
#include <stdio.h>
#include <string.h>

#include "xis.h"
#include "xtra.h"

#define MEM_IDX(x) ((x) % XIS_MEM_SIZE)

const static char *reg_map[16][3] = {
        {"%rax", "%ax", "%al"},  // r00
        {"%rbx", "%bx", "%bl"},  // r01
        {"%rcx", "%cx", "%cl"},  // r02
        {"%rdx", "%dx", "%dl"},  // r03
        {"%rsi", "%si", "%sil"},  // r04
        {"%rdi", "%di", "%dil"},  // r05
        {"%r8", "%r8w", "%r8b"},  // r06
        {"%r9", "%r9w", "%r9b"},  // r07
        {"%r10", "%r10w", "%r10b"},  // r08
        {"%r11", "%r11w", "%r11b"},  // r09
        {"%r12", "%r12w", "%r12b"},  // r10
        {"%r13", "%r13w", "%r13b"},  // r11
        {"%r14", "%r14w", "%r14b"},  // r12
        {"%r15", "%r15w", "%r15b"},  // r13
        {"%rbp", "%bp", "%bpl"},  // r14  frame pointer
        {"%rsp", "%sp", "%spl"}   // r15  stack pointer
};

#define ZERO_OP  "    %s\n"
#define ONE_OP   "    %s %s\n"
#define ABS_OP   "    %s *%s\n"
#define TWO_OP   "    %s %s, %s\n"
#define LOAD_OP  "    %s (%s), %s\n"
#define STOR_OP  "    %s %s, (%s)\n"
#define LOADB_OP "    %s (%s), %s\n"
#define STORB_OP "    %s %s, (%s)\n"
#define TEST_OP  "    %s %s, %s\n    setnz %%r15b\n"
#define EQU_OP   "    %s %s, %s\n    setz %%r15b\n"
#define CMP_OP   "    %s %s, %s\n    setg %%r15b\n"
#define BR_OP    "    test %%r15b, %%r15b\n    %s %s\n"
#define OUT_OP   "    push %%rdi\n    %s %s, %%di\n    call outchar\n    pop %%rdi\n"

#define F_OP1_MASK 0x00000007
#define F_OP1_64   0x00000000
#define F_OP1_16   0x00000001
#define F_OP1_8    0x00000002
#define F_OP1_DEF   F_OP1_64
#define F_OP1_IMM  0x00000003
#define F_OP1_LABEL 0x00000004
#define F_OP1_REL  0x00000005
#define F_OP2_MASK 0x00000018
#define F_OP2_64   0x00000000
#define F_OP2_16   0x00000008
#define F_OP2_8    0x00000010
#define F_OP2_DEF   F_OP2_64
#define F_OP1(x)   ((x) & F_OP1_MASK)
#define F_OP2(x)   (((x) & F_OP2_MASK) >> 3)

typedef struct op_out {
    unsigned char op_code;
    char *fmt;
    char *operation;
    int flags;
} op_out_t;

const op_out_t op_table[] = {
        /* OPS  OPNUM    Operations with no operands */
        {I_RET,   ZERO_OP, "ret", 0},
        {I_STD,   NULL, NULL, 0},
        {I_CLD,   NULL, NULL, 0},

        /* OPS  I OPNUM   Operations with one register operand */
        {I_NEG,   ONE_OP,  "neg", F_OP1_DEF},
        {I_NOT,   ONE_OP,  "not", F_OP1_DEF},
        {I_PUSH,  ONE_OP,  "push", F_OP1_64},
        {I_POP,   ONE_OP,  "pop", F_OP1_64},
        {I_JMPR,  ONE_OP,  "jmp *", F_OP1_64},
        {I_CALLR, ABS_OP, "call", F_OP1_64},
        {I_OUT,   OUT_OP, "mov", F_OP1_16},
        {I_INC,   ONE_OP, "inc", F_OP1_DEF},
        {I_DEC,   ONE_OP, "dec", F_OP1_DEF},

        /* OPS  I OPNUM   Operations with one immediate operand */
        {I_BR,    BR_OP, "jnz", F_OP1_REL},
        {I_JR,    ONE_OP, "jmp", F_OP1_REL},

        /* OPS  OPNUM   Operations with two register operands */
        {I_ADD,   TWO_OP, "add", F_OP1_DEF | F_OP2_DEF},
        {I_SUB,   TWO_OP, "sub", F_OP1_DEF | F_OP2_DEF},
        {I_MUL,   TWO_OP, "imul", F_OP1_DEF | F_OP2_DEF},
        {I_AND,   TWO_OP, "and", F_OP1_DEF | F_OP2_DEF},
        {I_OR,    TWO_OP, "or", F_OP1_DEF | F_OP2_DEF},
        {I_XOR,   TWO_OP, "xor", F_OP1_DEF | F_OP2_DEF},
        {I_SHR,   TWO_OP, "shr", F_OP1_8 | F_OP2_16},
        {I_SHL,   TWO_OP, "shl", F_OP1_8 | F_OP2_16},
        {I_TEST,  TEST_OP, "test", F_OP1_DEF | F_OP2_DEF},
        {I_CMP,   CMP_OP, "cmp", F_OP1_DEF | F_OP2_DEF},
        {I_EQU,   EQU_OP, "cmp", F_OP1_DEF | F_OP2_DEF},
        {I_MOV,   TWO_OP, "mov", F_OP1_DEF | F_OP2_DEF},
        {I_LOAD,  LOAD_OP, "mov", F_OP1_64 | F_OP2_16},
        {I_STOR,  STOR_OP, "mov", F_OP1_16 | F_OP2_64},
        {I_LOADB, LOADB_OP, "movb", F_OP1_64 | F_OP2_8},
        {I_STORB, STORB_OP, "movb", F_OP1_8 | F_OP2_64},

        /* OPS  R OPNUM   X Operations with one immediate operand */
        {I_JMP,   ONE_OP, "jmp", F_OP1_LABEL},
        {I_CALL,  ONE_OP, "call", F_OP1_LABEL},

        /* OPS  R OPNUM   X Ops with 1 imm. and 1 reg. operand */
        {I_LOADI, TWO_OP, "mov", F_OP1_IMM | F_OP2_DEF},

        {I_BAD,   NULL,   NULL, 0}
};

static void read_word(FILE *fpin, unsigned char *data) {
    assert(fpin);
    assert(data);
    unsigned long rc = fread(data, 1, 2, fpin);
    assert(rc == 2);
}

extern int xtra(char *name, FILE *fpin, long size) {
    unsigned int pc = 0;
    int debug = 0;

    printf(".globl %s\n", name);
    printf("%s:\n", name);
    printf("    push %%rbp\n");
    printf("    mov %%rsp, %%rbp\n");

    while (size > 0) {
        unsigned char data[2];
        read_word(fpin, data);

        printf(".L%4.4x:\n", pc);

        if (debug) {
            printf("    call debug\n");
        }

        pc = MEM_IDX(pc + 2);
        unsigned char op = data[0];
        if (!op) {
            break;
        }

        unsigned char op2 = data[1];
        int r1 = (op2 >> 4) & 0xf;
        int r2 = op2 & 0xf;

        unsigned short val;
        if (XIS_IS_EXT_OP(op)) {
            read_word(fpin, data);
            val = data[0] << 8 | data[1];
            pc = MEM_IDX(pc + 2);
        }

        int i;
        for (i = 0; op_table[i].op_code; i++) {
            if (op_table[i].op_code == op) {
                break;
            }
        }
        assert(op_table[i].op_code);

        if (op_table[i].op_code == I_STD || op_table[i].op_code == I_CLD) {
            debug = op_table[i].op_code == I_STD;
            continue;
        }
        assert(op_table[i].operation);

        if (op_table[i].operation) {
            const char *val1 = NULL;
            const char *val2 = NULL;
            char buffer[16];
            int typ1 = F_OP1(op_table[i].flags);
            int typ2 = F_OP2(op_table[i].flags & F_OP2_MASK);

            if (typ1 == F_OP1_IMM) {
                sprintf(buffer, "$%d", val);
                val1 = buffer;
                r2 = r1;
            } else if (typ1 == F_OP1_LABEL || typ1 == F_OP1_REL) {
                if (typ1 == F_OP1_REL) {
                    val = op2 + pc - 2;
                }
                sprintf(buffer, ".L%4.4x", val);
                val1 = buffer;
                r2 = r1;
            } else {
                val1 = reg_map[r1][typ1];
            }
            val2 = reg_map[r2][typ2];
            printf(op_table[i].fmt, op_table[i].operation, val1, val2);
        }
    }
    printf("    pop %%rbp\n");
    printf("    ret\n");
    return 1;
}

/*
 * shl a, b
 *   push c
 *   mov a, c
 *   shl c, b
 *   pop c
 * shl a, c
 *   push b
 *   mov c, b
 *   mov a, c
 *   shl c, b
 *   mov b, c
 *   pop b
 * shl c, b
 * shl c, c
 */