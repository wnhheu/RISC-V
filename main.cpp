#include <iostream>
#include <cstring>
typedef uint32_t uint;
enum Instructions
{
    LUI, AUIPC,     // U - type instructions 0 - 1
    JAL,      // J - type instructions 2
    BEQ, BNE, BLT, BGE, BLTU, BGEU,     // B - type instructions 3 - 8
    SB, SH, SW,     // S - type instructions 9 - 11
    ADD, SUB, SLL, SLT, SLTU, XOR, SRL, SRA, OR, AND,    // R - type instructions 12 - 21
    LB, LH, LW, LBU, LHU, JALR,     // I-type instructions 22 - 27
    ADDI, SLTI, SLTIU, XORI, ORI, ANDI, SLLI, SRLI, SRAI,       // I-type instructions 28 - 36
    NOP // 37
}Instruction;

int ra[100000];
int rb[100000];
int cnt1 = 0;
class simulator
{
private:
    uint reg[32];
    uint pc;
    unsigned char mem[238868];
    int visited[38];
    struct _IF
    {
        uint data;
        uint pc;
        _IF()
        {
            data = 0;
            pc = 0;
        }
        void clear()
        {
            data = 0;
            pc = 0;
        }
    }my_IF;
    struct _ID
    {
        uint opcode;
        uint rd;
        uint funct3;
        uint funct7;
        uint rs1;
        uint rs2;
        uint imm;
        Instructions op;
        _ID()
        {
            opcode = 0;
            rd = 0;
            funct3 = 0;
            funct7 = 0;
            rs1 = 0;
            rs2 = 0;
            imm = 0;
            op = NOP;
        }
        void clear()
        {
            opcode = 0;
            rd = 0;
            funct3 = 0;
            funct7 = 0;
            rs1 = 0;
            rs2 = 0;
            imm = 0;
            op = NOP;
        }
    }my_ID;

    struct _EX
    {
        Instructions op;
        uint rd;
        uint res;
        _EX()
        {
            rd = 0;
            res = 0;
            op = NOP;
        }
        void clear()
        {
            rd = 0;
            res = 0;
            op = NOP;
        }
    }my_EX;

    struct _MEM
    {
        uint res;
        uint op;
        uint rd;
        _MEM()
        {
            res = 0;
            op = 0;
            rd = 0;
        }
        void clear()
        {
            res = 0;
            op = 0;
            rd = 0;
        }
    }my_MEM;
public:
    simulator()
    {
        std::memset(reg, 0, sizeof reg);
        std::memset(visited, 0, sizeof visited);
        pc = 0;
    }
    void init_mem()
    {
        // freopen("D:/Riscv_Simulator/testcases_for_riscv/testcases/magic.data","r",stdin);
        char buffer[10];
        uint loc = 0;
        while(scanf("%s", buffer) != EOF)
        {
            if(buffer[0] == '@')
            {
                sscanf(buffer + 1, "%X", &loc);
                continue;
            }
            sscanf(buffer, "%X", &mem[loc]);
            scanf("%X", &mem[loc + 1]);
            scanf("%X", &mem[loc + 2]);
            scanf("%X", &mem[loc + 3]);
            loc += 4;
        }
    }
    void IF()
    {
        memcpy(&my_IF.data, mem + pc, 4);
        pc += 4;
    }

    int check_type()
    {
        uint tmp = my_IF.data & 0x7Fu;
        my_ID.opcode = tmp;
        switch(my_ID.opcode)
        {
            case 0x37:
            case 0x17: return 0;    // U - type, 0
            case 0x6F: return 1;    // J - type, 1
            case 0x63: return 2;    // B - type, 2
            case 0x23: return 3;    // S - type, 3
            case 0x33: return 4;    // R - type, 4
            case 0x67:
            case 0x03:
            case 0x13: return 5;    // I - type, 5
        }
        throw "Illegal Operation " ;
    }
    
    void visualization()
    {
        switch (my_ID.opcode) {
            case 0x37: my_ID.op = LUI;          break;  // U - type
            case 0x17: my_ID.op = AUIPC;        break;
            case 0x6F: my_ID.op = JAL;          break; // J - type
            case 0x67: my_ID.op = JALR;         break;
            case 0x63:                              // B - type
                switch (my_ID.funct3) {
                    case 0: my_ID.op = BEQ;     break;
                    case 1: my_ID.op = BNE;     break;
                    case 4: my_ID.op = BLT;     break;
                    case 5: my_ID.op = BGE;     break;
                    case 6: my_ID.op = BLTU;    break;
                    case 7: my_ID.op = BGEU;    break;
                }   break;
            case 0x23:                              // S - type
                switch (my_ID.funct3) {
                    case 0: my_ID.op = SB;      break;
                    case 1: my_ID.op = SH;      break;
                    case 2: my_ID.op = SW;      break;
                }   break;
            case 0x33:                              // R - type
                switch (my_ID.funct3) {
                    case 0:
                        if(my_ID.funct7 == 0)
                        {
                            my_ID.op = ADD;     break;
                        }
                        else
                        {
                            my_ID.op = SUB;     break;
                        }
                    case 1: my_ID.op = SLL;     break;
                    case 2: my_ID.op = SLT;     break;
                    case 3: my_ID.op = SLTU;    break;
                    case 4: my_ID.op = XOR;     break;
                    case 5:
                        if(my_ID.funct7 == 0)
                        {
                            my_ID.op = SRL;     break;
                        }
                        else
                        {
                            my_ID.op = SRA;     break;
                        }
                    case 6: my_ID.op = OR;      break;
                    case 7: my_ID.op = AND;     break;
                }   break;
            case 0x03:                              // I - type
                switch (my_ID.funct3) {
                    case 0: my_ID.op = LB;      break;
                    case 1: my_ID.op = LH;      break;
                    case 2: my_ID.op = LW;      break;
                    case 4: my_ID.op = LBU;     break;
                    case 5: my_ID.op = LHU;     break;
                }   break;
            case 0x13:
                switch (my_ID.funct3) {
                    case 0: my_ID.op = ADDI;    break;
                    case 1: my_ID.op = SLLI;    break;
                    case 2: my_ID.op = SLTI;    break;
                    case 3: my_ID.op = SLTIU;   break;
                    case 4: my_ID.op = XORI;    break;
                    case 5:
                        if(my_ID.funct7 == 0)
                        {
                            my_ID.op = SRLI;
                            break;
                        }
                        else
                        {
                            my_ID.op = SRAI;
                            break;
                        }
                    case 6: my_ID.op = ORI;     break;
                    case 7: my_ID.op = ANDI;    break;
                }   break;
        }
    }

    void ID()
    {
        int type;
        try {
            type = check_type();
        }catch (const char * err){
            std::cout << err << my_ID.opcode << std::endl;
            abort();
        }
        switch (type) {
            case 0: // lui, quipc
            {
                my_ID.rd = (my_IF.data >> 7) & 0x1Fu;
                my_ID.imm = (my_IF.data >> 12) << 12;
                break;
            }
            case 1: // jal
            {
                my_ID.rd = (my_IF.data >> 7) & 0x1Fu;
                my_ID.imm = (((my_IF.data >> 20) & 0x7FEu) + ((my_IF.data >> 20) & 0x800u)
                        + (((my_IF.data) & 0xFF000u)) + ((int)(my_IF.data & 0x80000000) >> 11));
                break;
            }
            case 2: // B
            {
                my_ID.funct3 = (my_IF.data >> 12) & 0x7u;
                my_ID.rs1 = reg[(my_IF.data >> 15) & 0x1Fu];
                my_ID.rs2 = reg[(my_IF.data >> 20) & 0x1Fu];
                my_ID.imm = (((my_IF.data >> 7) & 0x1Eu) + ((my_IF.data >> 20) & 0x7E0u) +
                        ((my_IF.data & 0x80u) << 4) + ((int)(my_IF.data & 0x80000000) >> 19));
                break;
            }
            case 3: // S
            {
                my_ID.funct3 = (my_IF.data >> 12) & 0x7u;
                my_ID.rs1 = reg[(my_IF.data >> 15) & 0x1Fu];
                my_ID.rs2 = reg[(my_IF.data >> 20) & 0x1Fu];
                my_ID.imm = (((int)my_IF.data >> 20) & 0xFFFFFFE0) + ((my_IF.data >> 7) & 0x1Fu);
                break;
            }
            case 4: // R
            {
                my_ID.rd = (my_IF.data >> 7) & 0x1Fu;
                my_ID.funct3 = (my_IF.data >> 12) & 0x7u;
                my_ID.rs1 = reg[(my_IF.data >> 15) & 0x1Fu];
                my_ID.rs2 = reg[(my_IF.data >> 20) & 0x1Fu];
                my_ID.funct7 = (int)my_IF.data >> 25;
                break;
            }
            case 5: // I
            {
                my_ID.rd = (my_IF.data >> 7) & 0x1Fu;
                my_ID.funct3 = (my_IF.data >> 12) & 0x7u;
                my_ID.rs1 = reg[(my_IF.data >> 15) & 0x1Fu];
                my_ID.imm = (int)my_IF.data >> 20;
                break;
            }
            default: std::cout << "error" << std::endl;
        }
        visualization();
//        std::cout << my_ID.op << " Res: " << my_ID.imm << " Rd: " << my_ID.rd << " Rs1: " << my_ID.rs1 << " Rs2: " << my_ID.rs2 << std::endl;
        my_IF.clear();
    }


    void EX()
    {
        my_EX.rd = my_ID.rd;
        my_EX.op = my_ID.op;
        switch (my_EX.op) {
            case LUI:
                my_EX.res = my_ID.imm;
                break;
            case AUIPC:
                my_EX.res = my_ID.imm + pc;
                break;
            case JAL:
                my_EX.res = pc;
                pc = pc + my_ID.imm - 4;
                break;
            case JALR:
                my_EX.res = pc;
                pc = (my_ID.rs1 + my_ID.imm) & 0xFFFFFFFEu;
                break;

            case BEQ:
                if(my_ID.rs1 == my_ID.rs2)
                    pc = pc - 4 + my_ID.imm;
                break;
            case BNE:
                if(my_ID.rs1 != my_ID.rs2)
                    pc = pc - 4 + my_ID.imm;
                break;
            case BLT:
                if((int)my_ID.rs1 < (int)my_ID.rs2)
                    pc = pc - 4 + my_ID.imm;
                break;
            case BGE:
                if((int)my_ID.rs1 >= (int)my_ID.rs2)
                    pc = pc - 4 + my_ID.imm;
                break;
            case BLTU:
                if(uint(my_ID.rs1) < uint(my_ID.rs2))
                    pc = pc - 4 + my_ID.imm;
                break;
            case BGEU:
                if(uint(my_ID.rs1) >= uint(my_ID.rs2))
                    pc = pc - 4 + my_ID.imm;
                break;

            case ADDI:
                my_EX.res = my_ID.rs1 + my_ID.imm;
                break;
            case SLTI:
                if((int)my_ID.rs1 < (int)my_ID.imm)
                    my_EX.res = 1;
                else
                    my_EX.res = 0;
                break;
            case SLTIU:
                if((uint)my_ID.rs1 < (uint)my_ID.imm)
                    my_EX.res = 1;
                else
                    my_EX.res = 0;
                break;
            case XORI:
                my_EX.res = my_ID.rs1 ^ my_ID.imm;
                break;
            case ORI:
                my_EX.res = my_ID.rs1 | my_ID.imm;
                break;
            case ANDI:
                my_EX.res = (my_ID.rs1 & my_ID.imm);
                break;
            case SLLI:
                my_EX.res = (my_ID.rs1 << (my_ID.imm & 0x1Fu));
                break;
            case SRLI:
                my_EX.res = (uint(my_ID.rs1) >> (my_ID.imm & 0x1Fu));
                break;
            case SRAI:
                my_EX.res = ((int)(my_ID.rs1) >> (my_ID.imm & 0x1Fu));
                break;
            case ADD:
                my_EX.res = my_ID.rs1 + my_ID.rs2;
                break;
            case SUB:
                my_EX.res = my_ID.rs1 - my_ID.rs2;
                break;
            case SLL:
                my_EX.res = (my_ID.rs1 << (my_ID.rs2 & 0x1Fu));
                break;
            case SLT:
                if((int)my_ID.rs1 < (int)my_ID.rs2)
                    my_EX.res = 1;
                else
                    my_EX.res = 0;
                break;
            case SLTU:
                if((uint)my_ID.rs1 < (uint)my_ID.rs2)
                    my_EX.res = 1;
                else
                    my_EX.res = 0;
                break;
            case XOR:
                my_EX.res = my_ID.rs1 ^ my_ID.rs2;
                break;
            case SRL:
                my_EX.res = ((uint)my_ID.rs1 >> (my_ID.rs2 & 0x1F));
                break;
            case SRA:
                my_EX.res = ((int)my_ID.rs1 >> (my_ID.rs2 & 0x1F));
                break;
            case OR:
                my_EX.res = my_ID.rs1 | my_ID.rs2;
                break;
            case AND:
                my_EX.res = my_ID.rs1 & my_ID.rs2;
                break;
            case LB:
            case LH:
            case LW:
            case LBU:
            case LHU:
                my_EX.res = my_ID.rs1 + my_ID.imm;
                break;
            case SB:
            case SH:
            case SW:
                my_EX.res = my_ID.rs1 + my_ID.imm;
                my_EX.rd = my_ID.rs2;
                break;
            default: break;
        }
        my_ID.clear();
    }

    void MEM()
    {
        char tmp;
        unsigned char utmp;
        short tmp1;
        unsigned short utmp1;
//        std::cout << cnt1++ << " pc: " << pc << " " << "op: " << my_EX.op << " || ";
        cnt1++;
        my_MEM.op = my_EX.op;
        my_MEM.rd = my_EX.rd;
        my_MEM.res= my_EX.res;
        visited[my_EX.op]++;
        for(int i = 0; i < 3; ++i) {
            if (i == 2) {
                switch (my_EX.op) {
                    case LB:
                        memcpy(&tmp,mem + my_EX.res,1);
                        my_MEM.res = tmp;
                        break;
                    case LH:
                        memcpy(&tmp1,mem + my_EX.res,2);
                        my_MEM.res = tmp1;
                        break;
                    case LW:
                        memcpy(&my_MEM.res,mem + my_EX.res,4);
                        break;
                    case LBU:
                        memcpy(&utmp,mem + my_EX.res,1);
                        my_MEM.res = utmp;
                        break;
                    case LHU:
                        memcpy(&utmp1,mem + my_EX.res,2);
                        my_MEM.res = utmp1;
                        break;
                    case SB:
                        tmp = my_EX.rd;
                        memcpy(mem + my_EX.res, &tmp, 1);
                        break;
                    case SH:
                        tmp1 = my_EX.rd;
                        memcpy(mem + my_EX.res, &tmp1, 2);
                        break;
                    case SW:
                        memcpy(mem + my_EX.res, &my_EX.rd, 4);
                        break;
                    default: break;
                }
            }
        }
        my_EX.clear();
    }

    void WB()
    {
//        for(int i = 0; i < 16; ++i)
//            std::cout << reg[i] << " ";
//        std::cout << std::endl;
        if(2 < my_MEM.op && my_MEM.op < 12)
            return;
        else if(my_MEM.rd)
        {
            reg[my_MEM.rd] = my_MEM.res;
            if(cnt1 == 260)
                int x = 0;
        }
        my_MEM.clear();
    }

    void print()
    {
        printf("%d ", reg[10] & 0xFFu);
    }

    void run()
    {
        while(true)
        {
            IF();
            if(my_IF.data == 0 || my_IF.data == 0x0ff00513)
                break;
            ID();
            EX();
            MEM();
            WB();
            if(cnt1 == 87308)
                int x = 0;
        }
        print();
    }
};


int main() {
    freopen("a.out","w",stdout);
    simulator RISC_v;
    RISC_v.init_mem();
    RISC_v.run();
    return 0;
}
