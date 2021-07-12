#ifndef SIMULATOR_HPP
#define SIMULATOR_HPP

#include <iostream>
#include <cstring>
using namespace std;
typedef unsigned uint;
const int QUEUE_SIZE = 32;
const int mem_size = 4194304;
unsigned char mem[200000];
enum Instructions
{
    LUI,
    AUIPC, // U - type instructions 0 - 1
    JAL,   // J - type instructions 2
    BEQ,
    BNE,
    BLT,
    BGE,
    BLTU,
    BGEU, // B - type instructions 3 - 8
    SB,
    SH,
    SW, // S - type instructions 9 - 11
    ADD,
    SUB,
    SLL,
    SLT,
    SLTU,
    XOR,
    SRL,
    SRA,
    OR,
    AND, // R - type instructions 12 - 21
    LB,
    LH,
    LW,
    LBU,
    LHU,
    JALR, // I-type instructions 22 - 27
    ADDI,
    SLTI,
    SLTIU,
    XORI,
    ORI,
    ANDI,
    SLLI,
    SRLI,
    SRAI, // I-type instructions 28 - 36
    NOP   // 37
};
class simulator
{
private:
    int cnt = 0;

    //pc_fetch
    uint pc;
    struct _cdb;
    template <class T>
    struct queue
    {
        uint head;
        uint tail;
        uint size;
        uint max_size;
        T data[32];
        queue()
        {
            head = 1;
            tail = 1;
            size = 0;
            max_size = 32;
        }

        bool is_empty()
        {
            return size == 0;
        }

        bool is_full()
        {
            return (tail == head - 1) || (head == 1 && tail == 32);
        }

        void push_back(T value)
        {
            if (size == max_size)
            {
                std::cout << "error, too much elements" << std::endl;
                return;
            }
            size++;
            if (tail == 32)
                tail = 1;
            data[tail++] = value;
        }

        T pop_front()
        {
            if (head == 32)
                head = 1;
            T val = data[head++];

            size--;
            return val;
        }

        T top()
        {
            if (head == 32)
                head = 1;
            return data[head];
        }
    };

    struct rs_unit
    {
        Instructions op;
        uint vj;
        uint vk;
        uint qj;
        uint qk;
        uint a;
        uint rd;
        uint rob_loc;
        uint _pc;
        uint ori_data;
        rs_unit()
        {
            op = NOP;
            vj = 0;
            vk = 0;
            qj = 0;
            qk = 0;
            a = 0;
            rob_loc = 0;
            rd = 0;
            _pc = 0;
            ori_data = 0;
        }
        void clear()
        {
            op = NOP;
            vj = 0;
            qj = 0;
            vk = 0;
            qk = 0;
            a = 0;
            rob_loc = 0;
            rd = 0;
            _pc = 0;
            ori_data = 0;
        }
    };

    struct inst_unit
    {
        uint data;
        uint pc;
    };

    struct rob_unit
    {
        Instructions op;
        uint rd;
        uint res;
        uint jump_address;
        uint data;
        uint num;
        bool is_ok;
        bool sl;
        rob_unit()
        {
            op = NOP;
            rd = 0;
            res = 0;
            is_ok = false;
            sl = false;
            data = 0;
            jump_address = 0;
            num = 0;
        }
        void clear()
        {
            op = NOP;
            rd = 0;
            res = 0;
            jump_address = 0;
            is_ok = false;
            sl = false;
            data = 0;
            num = 0;
        }
    };

    struct reg_file
    {
        uint q; // ROB number
        uint v; // current value
        bool busy;
        reg_file()
        {
            q = 0;
            v = 0;
            busy = false;
        }
    };
    reg_file reg_prev[32];
    reg_file reg_succ[32];

    struct _inst_que
    {
        uint _pc;
        queue<inst_unit> inst;
        _inst_que()
        {
            _pc = 0;
            inst.max_size = 32;
            for (int i = 0; i < 32; ++i)
            {
                inst.data[i].data = 0;
                inst.data[i].pc = 0;
            }
        }
    } inst_que;

    struct _inst_rs
    {
        rs_unit unit;
        void clear()
        {
            unit.clear();
        }
    } inst_rs;

    struct _rs
    {
        rs_unit unit[32];
        int head;
        int tail;
        int size;
        _rs()
        {
            head = 0;
            tail = 0;
            size = 0;
        }
        void add(rs_unit &tmp)
        {
            unit[tail++] = tmp;
            size++;
        }
        bool is_empty()
        {
            return head == tail;
        }
        bool is_full()
        {
            return size == 32;
        }
        rs_unit find()
        {
            rs_unit res;
            for (int i = 0; i < tail; ++i)
            {
                if (unit[i].qj == 0 && unit[i].qk == 0)
                {
                    res = unit[i];
                    size--;
                    for (int j = i; j < tail - 1; ++j)
                    {
                        unit[j] = unit[j + 1];
                    }
                    tail--;
                    break;
                }
            }
            return res;
        }
    } rs;

    struct _rs_ex
    {
        rs_unit unit;
        void clear()
        {
            unit.clear();
        }
    } rs_ex;

    struct _slbuffer
    {
        queue<rs_unit> sl_que;

    } slbuffer;

    struct my_rob
    {
        queue<rob_unit> rob_que;
    } rob_prev, rob_succ;

    struct _cdb
    {
        uint res;
        uint jump_address;
        uint rob_pos;
        uint rd;
        uint data;
        Instructions op;
        bool jump;
        bool sl;
        _cdb()
        {
            res = 0;
            data = 0;
            jump = false;
            jump_address = 0;
            rob_pos = 0;
            rd = 0;
            op = NOP;
            sl = false;
        }
        void clear()
        {
            data = 0;
            res = 0;
            jump_address = 0;
            rob_pos = 0;
            op = NOP;
            rd = 0;
            jump = false;
            sl = false;
        }
    } cdb_slb_prev, cdb_slb_next, cdb_ex;

    struct _transmit
    {
        bool rob_to_commit;
        bool commit_to_rob;
        bool commit_to_slb;
        bool slb_to_commit;
        bool commit_to_reg;
        rob_unit ctr_data;
        _transmit()
        {
            rob_to_commit = false;
            commit_to_rob = false;
            commit_to_slb = false;
            slb_to_commit = false;
            commit_to_reg = false;
        }
        void clear()
        {
            rob_to_commit = false;
            commit_to_rob = false;
            commit_to_slb = false;
            slb_to_commit = false;
            commit_to_reg = false;
        }
    } transmit;

public:
    simulator()
    {
        pc = 0;
    }

    void scan()
    {
        freopen("D:/Riscv_Simulator/testcases_for_riscv/testcases/queens.data", "r", stdin);
        char buffer[10];
        uint loc = 0;
        while (scanf("%s", buffer) != EOF)
        {
            if (buffer[0] == '@')
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

    unsigned int cycle = 0;
    bool init = false;
    void run()
    {
        while (true)
        {
            /*在这里使用了两阶段的循环部分：
              1. 实现时序电路部分，即在每个周期初同步更新的信息。
              2. 实现组合电路部分，即在每个周期中如ex、issue的部分
              已在下面给出代码
            */
            run_rob();

            if (transmit.ctr_data.data == 0x0ff00513)
            {
                std::cout << ((unsigned int)reg_prev[10].v & 255u);
                break;
            }

            run_slbuffer();

            run_reservation();

            run_regfile();

            run_inst_fetch_queue();

            update();
            //---------------------------------

            run_ex();

            run_issue();
            run_commit();
            if (pc == 4804 && cnt > 86500)
                int y = 0;
            if (cnt == 87280)
                int x = 0;
        }
    }

    void run_inst_fetch_queue()
    {
        /*
        在这一部分你需要完成的工作：
        1. 实现一个先进先出的指令队列
        2. 读取指令并存放到指令队列中
        3. 准备好下一条issue的指令
        tips: 考虑边界问题（满/空...）
        */
        if (init)
        {
            while (!inst_que.inst.is_empty())
            {
                inst_que.inst.pop_front();
            }
        }
        if (!inst_que.inst.is_full())
        {
            inst_unit data;
            std::memcpy(&data.data, mem + pc, 4);
            data.pc = pc;
            inst_que.inst.push_back(data);
            pc += 4;
        }
    }

    void run_regfile()
    {
        /*
        每个寄存器会记录Q和V，含义参考ppt。这一部分会进行写寄存器，内容包括：根据issue和commit的通知修改对应寄存器的Q和V。
        tip: 请注意issue和commit同一个寄存器时的情况
        */
        // 这个可能要晚一点再写，我现在还没想好后面怎么实现
        // 本来想的是把所有东西都准备好再去实现的，但是后面发现不现实，因为tomasulo内容太多了，我现在已经知道了大致框架，要开始先写一点了
        // commit first then issue
        if (transmit.commit_to_reg)
        {
            if (transmit.ctr_data.rd != 0)
            {
                if (reg_succ[transmit.ctr_data.rd].q == transmit.ctr_data.num)
                {
                    reg_succ[transmit.ctr_data.rd].q = 0;
                }
                reg_succ[transmit.ctr_data.rd].v = transmit.ctr_data.res;
            }
            transmit.commit_to_reg = false;
            transmit.ctr_data.clear();
        }
        if (inst_rs.unit.rd != 0)
            reg_succ[inst_rs.unit.rd].q = inst_rs.unit.rob_loc;
        inst_rs.unit.clear();
    }

    void run_issue()
    {
        /*
        在这一部分你需要完成的工作：
        1. 从run_inst_fetch_queue()中得到issue的指令
        2. 对于issue的所有类型的指令向ROB申请一个位置（或者也可以通过ROB预留位置），并通知regfile修改相应的值 // 这里怎么实现啊
        2. 对于 非 Load/Store的指令，将指令进行分解后发到Reservation Station
          tip: 1. 这里需要考虑怎么得到rs1、rs2的值，并考虑如当前rs1、rs2未被计算出的情况，参考书上内容进行处理
               2. 在本次作业中，我们认为相应寄存器的值已在ROB中存储但尚未commit的情况是可以直接获得的，即你需要实现这个功能
                  而对于rs1、rs2不ready的情况，只需要stall即可，有兴趣的同学可以考虑下怎么样直接从EX完的结果更快的得到计算结果
        3. 对于 Load/Store指令，将指令分解后发到SLBuffer(需注意SLBUFFER也该是个先进先出的队列实现)
        tips: 考虑边界问题（是否还有足够的空间存放下一条指令）
        */
        if (!inst_que.inst.is_empty() && !rs.is_full() && !rob_prev.rob_que.is_full())
        {
            if (rob_succ.rob_que.tail == 32)
                rob_succ.rob_que.tail = 1;
            uint loc = rob_succ.rob_que.tail++;
            rs_unit unit;
            uint data = inst_que.inst.top().data;
            int type;
            uint tmp = data & 0x7Fu;
            switch (tmp)
            {
            case 0x37:
            case 0x17:
                type = 0;
                break; // U - type, 0
            case 0x6F:
                type = 1;
                break; // J - type, 1
            case 0x63:
                type = 2;
                break; // B - type, 2
            case 0x23:
                type = 3;
                break; // S - type, 3
            case 0x33:
                type = 4;
                break; // R - type, 4
            case 0x67:
            case 0x03:
            case 0x13:
                type = 5;
                break; // I - type, 5
            default:
                type = 6;
                break;
            }
            uint funct3 = 0;
            uint funct7 = 0;
            uint rs1 = 0;
            uint rs2 = 0;
            uint rd = 0;
            uint imm = 0;
            switch (type)
            {
            case 0: // lui, quipc
            {
                switch (tmp)
                {
                case 0x37:
                    unit.op = LUI;
                    break; // U - type
                case 0x17:
                    unit.op = AUIPC;
                    break;
                default:
                    break;
                }
                rd = (data >> 7) & 0x1Fu;
                imm = (data >> 12) << 12;
                break;
            }
            case 1: // jal
            {
                unit.op = JAL; // J - type
                rd = (data >> 7) & 0x1Fu;
                imm = (((data >> 20) & 0x7FEu) + ((data >> 20) & 0x800u) + (((data)&0xFF000u)) + ((int)(data & 0x80000000) >> 11));
                break;
            }
            case 2: // B
            {
                funct3 = (data >> 12) & 0x7u;
                rs1 = (data >> 15) & 0x1Fu;
                rs2 = (data >> 20) & 0x1Fu;
                imm = (((data >> 7) & 0x1Eu) + ((data >> 20) & 0x7E0u) +
                       ((data & 0x80u) << 4) + ((int)(data & 0x80000000) >> 19));
                switch (funct3)
                {
                case 0:
                    unit.op = BEQ;
                    break;
                case 1:
                    unit.op = BNE;
                    break;
                case 4:
                    unit.op = BLT;
                    break;
                case 5:
                    unit.op = BGE;
                    break;
                case 6:
                    unit.op = BLTU;
                    break;
                case 7:
                    unit.op = BGEU;
                    break;
                default:
                    break;
                }
                break;
            }
            case 3: // S
            {
                funct3 = (data >> 12) & 0x7u;
                rs1 = (data >> 15) & 0x1Fu;
                rs2 = (data >> 20) & 0x1Fu;
                imm = (((int)data >> 20) & 0xFFFFFFE0) + ((data >> 7) & 0x1Fu);
                switch (funct3)
                {
                case 0:
                    unit.op = SB;
                    break;
                case 1:
                    unit.op = SH;
                    break;
                case 2:
                    unit.op = SW;
                    break;
                default:
                    break;
                }
                break;
            }
            case 4: // R
            {
                rd = (data >> 7) & 0x1Fu;
                funct3 = (data >> 12) & 0x7u;
                rs1 = (data >> 15) & 0x1Fu;
                rs2 = (data >> 20) & 0x1Fu;
                funct7 = (int)data >> 25;
                switch (funct3)
                {
                case 0:
                    if (funct7 == 0)
                    {
                        unit.op = ADD;
                        break;
                    }
                    else
                    {
                        unit.op = SUB;
                        break;
                    }
                case 1:
                    unit.op = SLL;
                    break;
                case 2:
                    unit.op = SLT;
                    break;
                case 3:
                    unit.op = SLTU;
                    break;
                case 4:
                    unit.op = XOR;
                    break;
                case 5:
                    if (funct7 == 0)
                    {
                        unit.op = SRL;
                        break;
                    }
                    else
                    {
                        unit.op = SRA;
                        break;
                    }
                case 6:
                    unit.op = OR;
                    break;
                case 7:
                    unit.op = AND;
                    break;
                default:
                    break;
                }
            }
            case 5: // I
            {
                rd = (data >> 7) & 0x1Fu;
                funct3 = (data >> 12) & 0x7u;
                rs1 = (data >> 15) & 0x1Fu;
                imm = (int)data >> 20;
                switch (tmp)
                {
                case 0x03:
                    switch (funct3)
                    {
                    case 0:
                        unit.op = LB;
                        break;
                    case 1:
                        unit.op = LH;
                        break;
                    case 2:
                        unit.op = LW;
                        break;
                    case 4:
                        unit.op = LBU;
                        break;
                    case 5:
                        unit.op = LHU;
                        break;
                    default:
                        break;
                    }
                    break;
                case 0x13:
                    switch (funct3)
                    {
                    case 0:
                        unit.op = ADDI;
                        break;
                    case 1:
                        unit.op = SLLI;
                        break;
                    case 2:
                        unit.op = SLTI;
                        break;
                    case 3:
                        unit.op = SLTIU;
                        break;
                    case 4:
                        unit.op = XORI;
                        break;
                    case 5:
                        if (funct7 == 0)
                        {
                            unit.op = SRLI;
                            break;
                        }
                        else
                        {
                            unit.op = SRAI;
                            break;
                        }
                    case 6:
                        unit.op = ORI;
                        break;
                    case 7:
                        unit.op = ANDI;
                        break;
                    default:
                        break;
                    }
                    break;
                case 0x67:
                    unit.op = JALR;
                    break;
                default:
                    break;
                }
            }
            default:
                break;
            }
            unit.rob_loc = loc;
            unit.a = imm; // rs1, rs2 equal zero

            if (reg_prev[rs1].q && rs1 != 0) // if q doesn't equal zero, it means reg[rs1] can be updated, so send the value to unit.qj
                unit.qj = reg_prev[rs1].q;
            else
                unit.vj = reg_prev[rs1].v;
            if (reg_prev[rs2].q && rs2 != 0)
                unit.qk = reg_prev[rs2].q;
            else
                unit.vk = reg_prev[rs2].v;
            unit.rd = rd;
            unit._pc = inst_que.inst.pop_front().pc;
            unit.ori_data = data;
            inst_rs.unit = unit;
        }
    }

    bool is_LS(Instructions inst)
    {
        return (8 < inst && inst < 12) || (21 < inst && inst < 27);
    }

    void update_rs(my_rob &rob, _rs &rs1)
    {
        for (int i = rs1.head; i != rs1.tail; i == 32 ? i = 1 : i++)
        {
            if (rs1.unit[i].qj != 0 && rob.rob_que.data[rs1.unit[i].qj].op != NOP)
            {
                rs1.unit[i].vj = rob.rob_que.data[rs1.unit[i].qj].res;
                rs1.unit[i].qj = 0;
            }
            if (rs1.unit[i].qk != 0 && rob.rob_que.data[rs1.unit[i].qk].op != NOP)
            {
                rs1.unit[i].vk = rob.rob_que.data[rs1.unit[i].qk].res;
                rs1.unit[i].qk = 0;
            }
        }
    }

    void run_reservation()
    {
        /*
        在这一部分你需要完成的工作：
        1. 设计一个Reservation Station，其中要存储的东西可以参考CAAQA或其余资料，至少需要有用到的寄存器信息等
        2. 如存在，从issue阶段收到要存储的信息，存进Reservation Station（如可以计算也可以直接进入计算）
        3. 从Reservation Station或者issue进来的指令中选择一条可计算的发送给EX进行计算
        4. 根据上个周期EX阶段或者SLBUFFER的计算得到的结果遍历Reservation Station，更新相应的值
        */
        // update
        if (init)
        {
            while (!rs.is_empty())
            {
                rs.head = rs.tail = rs.size = 0;
            }
            return;
        }
        if (inst_rs.unit.op != NOP && !is_LS(inst_rs.unit.op))
        {
            rs.add(inst_rs.unit);
        }
        update_rs(rob_prev, rs);

        rs_ex.unit = rs.find();
        rs_unit unit = rs_ex.unit;
        //        std::cout << unit.op << " Res: " << unit.a << " Rd: " << unit.rd << " Rs1: " << unit.vj << " Rs2: " << unit.vk << std::endl;
        cdb_ex.clear();
        // update later
    }

    void run_ex()
    {
        /*
        在这一部分你需要完成的工作：
        根据Reservation Station发出的信息进行相应的计算
        tips: 考虑如何处理跳转指令并存储相关信息
              Store/Load的指令并不在这一部分进行处理
        */
        rs_unit unit = rs_ex.unit;
        uint res = 0;
        if (is_LS(unit.op) || unit.op == NOP)
            return;
        switch (unit.op)
        {
        case LUI:
            res = unit.a;
            break;
        case AUIPC: // 实现可能有问题
            res = unit.a + unit._pc;
            break;
        case JAL:
        {
            res = unit._pc + 4;
            unit.a = unit._pc + unit.a;
            cdb_ex.jump_address = unit.a;
            cdb_ex.jump = true;
            break;
        }
        case JALR:
            res = unit._pc + 4;
            unit.a = (unit.vj + unit.a) & 0xFFFFFFFEu;
            cdb_ex.jump = true;
            cdb_ex.jump_address = unit.a;
            break;

        case BEQ:
            if (unit.vj == unit.vk)
            {
                unit.a = unit._pc + unit.a;
                cdb_ex.jump = true;
                cdb_ex.jump_address = unit.a;
            }
            break;
        case BNE:
            if (unit.vj != unit.vk)
            {
                unit.a = unit._pc + unit.a;
                cdb_ex.jump = true;
                cdb_ex.jump_address = unit.a;
            }
            break;
        case BLT:
            if ((int)unit.vj < (int)unit.vk)
            {
                unit.a = unit._pc + unit.a;
                cdb_ex.jump = true;
                cdb_ex.jump_address = unit.a;
            }
            break;
        case BGE:
            if ((int)unit.vj >= (int)unit.vk)
            {
                unit.a = unit._pc + unit.a;
                cdb_ex.jump = true;
                cdb_ex.jump_address = unit.a;
            }
            break;
        case BLTU:
            if (uint(unit.vj) < uint(unit.vk))
            {
                unit.a = unit._pc + unit.a;
                cdb_ex.jump = true;
                cdb_ex.jump_address = unit.a;
            }
            break;
        case BGEU:
            if (uint(unit.vj) >= uint(unit.vk))
            {
                unit.a = unit._pc + unit.a;
                cdb_ex.jump = true;
                cdb_ex.jump_address = unit.a;
            }
            break;

        case ADDI:
            res = unit.vj + unit.a;
            break;
        case SLTI:
            if ((int)unit.vj < (int)unit.a)
                res = 1;
            else
                res = 0;
            break;
        case SLTIU:
            if ((uint)unit.vj < (uint)unit.a)
                res = 1;
            else
                res = 0;
            break;
        case XORI:
            res = unit.vj ^ unit.a;
            break;
        case ORI:
            res = unit.vj | unit.a;
            break;
        case ANDI:
            res = (unit.vj & unit.a);
            break;
        case SLLI:
            res = (unit.vj << (unit.a & 0x1Fu));
            break;
        case SRLI:
            res = (uint(unit.vj) >> (unit.a & 0x1Fu));
            break;
        case SRAI:
            res = ((int)(unit.vj) >> (unit.a & 0x1Fu));
            break;
        case ADD:
            res = unit.vj + unit.vk;
            break;
        case SUB:
            res = unit.vj - unit.vk;
            break;
        case SLL:
            res = (unit.vj << (unit.vk & 0x1Fu));
            break;
        case SLT:
            if ((int)unit.vj < (int)unit.vk)
                res = 1;
            else
                res = 0;
            break;
        case SLTU:
            if ((uint)unit.vj < (uint)unit.vk)
                res = 1;
            else
                res = 0;
            break;
        case XOR:
            res = unit.vj ^ unit.vk;
            break;
        case SRL:
            res = ((uint)unit.vj >> (unit.vk & 0x1F));
            break;
        case SRA:
            res = ((int)unit.vj >> (unit.vk & 0x1F));
            break;
        case OR:
            res = unit.vj | unit.vk;
            break;
        case AND:
            res = unit.vj & unit.vk;
            break;
        default:
            return;
        }
        cdb_ex.data = unit.ori_data;
        cdb_ex.rob_pos = unit.rob_loc;
        cdb_ex.res = res;
        cdb_ex.op = unit.op;
        cdb_ex.rd = unit.rd;

        rs_ex.clear();
    }

    void update_slb(my_rob &rob, queue<rs_unit> &que)
    {
        for (int i = que.head; i != que.tail; i == 32 ? i = 1 : i++)
        {
            if (que.data[i].qj != 0 && rob.rob_que.data[que.data[i].qj].op != NOP)
            {
                que.data[i].vj = rob.rob_que.data[que.data[i].qj].res;
                que.data[i].qj = 0;
            }
            if (que.data[i].qk != 0 && rob.rob_que.data[que.data[i].qk].op != NOP)
            {
                que.data[i].vk = rob.rob_que.data[que.data[i].qk].res;
                que.data[i].qk = 0;
            }
        }
    }

    void run_slbuffer()
    {
        /*
        在这一部分中，由于SLBUFFER的设计较为多样，在这里给出两种助教的设计方案：
        1. 1）通过循环队列，设计一个先进先出的SLBUFFER，同时存储 head1、head2、tail三个变量。
           其中，head1是真正的队首，记录第一条未执行的内存操作的指令；tail是队尾，记录当前最后一条未执行的内存操作的指令。
           而head2负责确定处在head1位置的指令是否可以进行内存操作，其具体实现为在ROB中增加一个head_ensure的变量，每个周期head_ensure做取模意义下的加法，直到等于tail或遇到第一条跳转指令，
           这个时候我们就可以保证位于head_ensure及之前位置的指令，因中间没有跳转指令，一定会执行。因而，只要当head_ensure当前位置的指令是Store、Load指令，我们就可以向slbuffer发信息，增加head2。
           简单概括即对head2之前的Store/Load指令，我们根据判断出ROB中该条指令之前没有jump指令尚未执行，从而确定该条指令会被执行。
           2）同时SLBUFFER还需根据上个周期EX和SLBUFFER的计算结果遍历SLBUFFER进行数据的更新。
           3）此外，在我们的设计中，将SLBUFFER进行内存访问时计算需要访问的地址和对应的读取/存储内存的操作在SLBUFFER中一并实现，
           也可以考虑分成两个模块，该部分的实现只需判断队首的指令是否能执行并根据指令相应执行即可。
        2. 1）SLB每个周期会查看队头，若队头指令还未ready，则阻塞。

           2）当队头ready且是load指令时，SLB会直接执行load指令，包括计算地址和读内存，
           然后把结果通知ROB，同时将队头弹出。ROB commit到这条指令时通知Regfile写寄存器。

           3）当队头ready且是store指令时，SLB会等待ROB的commit，commit之后会SLB执行这
           条store指令，包括计算地址和写内存，写完后将队头弹出。
           4）同时SLBUFFER还需根据上个周期EX和SLBUFFER的计算结果遍历SLBUFFER进行数据的更新。
        */
        // I-type has three values
        // imm->unit.a
        // rd->unit.rd
        // rs1->unit.rs1
        // S-typeS
        // rd->rs2
        if (init)
        {
            while (!slbuffer.sl_que.is_empty())
            {
                slbuffer.sl_que.pop_front();
            }
            return;
        }
        rs_unit unit = inst_rs.unit;
        if (is_LS(unit.op))
        {
            /*            if(unit.qj != 0 && rob_prev.rob_que.data[unit.qj].op != NOP) // this instruction has entered rob_prev. In rob, we shall set the value to NOP once committed
            {
                unit.vj = rob_prev.rob_que.data[unit.qj].res;
                unit.qj = 0;
            }
            if(unit.qk != 0 && rob_prev.rob_que.data[unit.qk].op != NOP)
            {
                unit.vk = rob_prev.rob_que.data[unit.qk].res;
                unit.qk = 0;
            }*/
            slbuffer.sl_que.push_back(unit);
        }

        if (slbuffer.sl_que.is_empty())
            return;

        update_slb(rob_prev, slbuffer.sl_que);

        unit = slbuffer.sl_que.top();

        if (unit.qj == 0)
        {
            if (8 < unit.op && unit.op < 12 && !transmit.commit_to_slb)
            {
                cdb_slb_next.clear();
                cdb_slb_next.rob_pos = unit.rob_loc;
                cdb_slb_next.rd = unit.rd;
                cdb_slb_next.sl = true;
                return;
            }
            if (cycle != 2)
            {
                cycle++;
                return;
            }

            //            std::cout << unit.op << "Res: " << unit.a << "Rd: " << unit.rd << "Rs1: " << unit.vj << "Rs2: " << unit.vk << std::endl;
            cycle = 0;
            if (8 < unit.op && unit.op < 12) // 通过transmit在commit、sbuffer之间传递数据
            {
                cdb_slb_next.op = unit.op;
                unit = slbuffer.sl_que.pop_front();
                if (transmit.commit_to_slb)
                {
                    char tmp = 0;
                    short tmp1 = 0;
                    unit.a += unit.vj;
                    cdb_slb_next.res = unit.vk;
                    switch (unit.op)
                    {
                    case SB:
                        tmp = unit.vk;
                        memcpy(mem + unit.a, &tmp, 1);
                        break;
                    case SH:
                        tmp1 = unit.vk;
                        memcpy(mem + unit.a, &tmp1, 2);
                        break;
                    case SW:
                        memcpy(mem + unit.a, &unit.vk, 4);
                        break;
                    default:
                        break;
                    }
                    transmit.commit_to_slb = false;
                    transmit.slb_to_commit = true;
                }
            }
            else
            {
                cdb_slb_next.op = unit.op;
                cdb_slb_next.rob_pos = unit.rob_loc;
                cdb_slb_next.rd = unit.rd;
                unit = slbuffer.sl_que.pop_front();
                unit.a += unit.vj;
                if (unit.a != 0)
                {
                    char tmp = 0;
                    short tmp1 = 0;
                    unsigned char utmp = 0;
                    unsigned short utmp1 = 0;
                    switch (unit.op)
                    {
                    case LB:
                        memcpy(&tmp, mem + unit.a, 1);
                        cdb_slb_next.res = tmp;
                        break;
                    case LH:
                        memcpy(&tmp1, mem + unit.a, 2);
                        cdb_slb_next.res = tmp1;
                        break;
                    case LW:
                        memcpy(&cdb_slb_next.res, mem + unit.a, 4);
                        break;
                    case LBU:
                        memcpy(&utmp, mem + unit.a, 1);
                        cdb_slb_next.res = utmp;
                        break;
                    case LHU:
                        memcpy(&utmp1, mem + unit.a, 2);
                        cdb_slb_next.res = utmp1;
                        break;
                    default:
                        break;
                    }
                }
            }
        }
        else // stall
            return;
    }

    void run_rob()
    {
        /*
        在这一部分你需要完成的工作：
        1. 实现一个先进先出的ROB，存储所有指令
        1. 根据issue阶段发射的指令信息分配空间进行存储。
        2. 根据EX阶段和SLBUFFER的计算得到的结果，遍历ROB，更新ROB中的值
        3. 对于队首的指令，如果已经完成计算及更新，进行commit
        */

        if (transmit.commit_to_rob)
        {
            if (is_LS(rob_succ.rob_que.top().op))
            {
                cdb_slb_prev.clear();
                cdb_slb_next.clear();
            }
            cnt++;
            rob_succ.rob_que.data[rob_succ.rob_que.head].clear();
            rob_succ.rob_que.pop_front();
            transmit.commit_to_rob = false;
        }
        if (init)
        {
            for (int i = 0; i < 32; ++i)
                rob_succ.rob_que.data[i].clear();
            rob_succ.rob_que.head = rob_succ.rob_que.tail = 1;
            cdb_ex.clear();
            cdb_slb_next.clear();
            cdb_slb_prev.clear();
            return;
        }
        if (cdb_ex.op != NOP)
        {
            rob_succ.rob_que.data[cdb_ex.rob_pos].op = cdb_ex.op;
            rob_succ.rob_que.data[cdb_ex.rob_pos].res = cdb_ex.res;
            rob_succ.rob_que.data[cdb_ex.rob_pos].rd = cdb_ex.rd;
            rob_succ.rob_que.data[cdb_ex.rob_pos].data = cdb_ex.data;
            rob_succ.rob_que.data[cdb_ex.rob_pos].is_ok = !cdb_ex.jump;
            rob_succ.rob_que.data[cdb_ex.rob_pos].num = cdb_ex.rob_pos;
            rob_succ.rob_que.size++;
            if (!rob_succ.rob_que.data[cdb_ex.rob_pos].is_ok)
                rob_succ.rob_que.data[cdb_ex.rob_pos].jump_address = cdb_ex.jump_address;
        }
        if (cdb_slb_prev.op != NOP)
        {
            rob_succ.rob_que.data[cdb_slb_prev.rob_pos].op = cdb_slb_prev.op;
            rob_succ.rob_que.data[cdb_slb_prev.rob_pos].res = cdb_slb_prev.res;
            rob_succ.rob_que.data[cdb_slb_prev.rob_pos].rd = cdb_slb_prev.rd;
            rob_succ.rob_que.data[cdb_slb_prev.rob_pos].data = cdb_slb_prev.data;
            rob_succ.rob_que.data[cdb_slb_prev.rob_pos].is_ok = !cdb_slb_prev.jump;
            rob_succ.rob_que.data[cdb_slb_prev.rob_pos].num = cdb_slb_prev.rob_pos;
            rob_succ.rob_que.size++;
        }
        else if (cdb_slb_prev.sl)
        {
            rob_succ.rob_que.data[cdb_slb_prev.rob_pos].rd = cdb_slb_prev.rd;
            rob_succ.rob_que.data[cdb_slb_prev.rob_pos].sl = true;
            rob_succ.rob_que.data[cdb_slb_prev.rob_pos].num = cdb_slb_prev.rob_pos;
        }

        rob_unit unit = rob_succ.rob_que.top();
        if (unit.op != NOP)
        {
            transmit.rob_to_commit = true;
        }
        else
            return;
    }

    void run_commit()
    {
        /*
        在这一部分你需要完成的工作：
        1. 根据ROB发出的信息通知regfile修改相应的值，包括对应的ROB和是否被占用状态（注意考虑issue和commit同一个寄存器的情况）
        2. 遇到跳转指令更新pc值，并发出信号清空所有部分的信息存储（这条对于很多部分都有影响，需要慎重考虑）
        */
        rob_unit unit = rob_succ.rob_que.top();
        if (8 < unit.op && unit.op < 12)
        {
            if (!transmit.slb_to_commit) // 一定是顺序对应好的
            {
                transmit.commit_to_slb = true;
                return;
            }
            else
            {
                transmit.slb_to_commit = false;
            }
        }
        else if (unit.sl)
        {
            if (!transmit.slb_to_commit) // 一定是顺序对应好的
            {
                transmit.commit_to_slb = true;
                return;
            }
        }
        if (transmit.rob_to_commit)
        {
            transmit.rob_to_commit = false;
            if (!unit.is_ok)
            {
                if (unit.op == 2 || unit.op == 27)
                    transmit.commit_to_reg = true;
                pc = unit.jump_address;
                transmit.commit_to_rob = true;
                transmit.commit_to_reg = true;
                transmit.ctr_data = unit;
                init = true;
                // 清空所有部分的信息储存
            }
            else
            {
                transmit.commit_to_reg = true;
                transmit.commit_to_rob = true;
                transmit.ctr_data = unit;
            }
        }
    }

    void update()
    {
        /*
        在这一部分你需要完成的工作：
        对于模拟中未完成同步的变量（即同时需记下两个周期的新/旧信息的变量）,进行数据更新。
        */
        if (init)
        {
            for (int i = 0; i < 32; ++i)
                reg_succ[i].q = 0;
            init = false;
        }
        for (int i = 0; i < 32; ++i)
        {
            rob_prev.rob_que.data[i] = rob_succ.rob_que.data[i];
        }
        rob_prev.rob_que.tail = rob_succ.rob_que.tail;
        rob_prev.rob_que.head = rob_succ.rob_que.head;
        rob_prev.rob_que.size = rob_succ.rob_que.size;
        for (int i = 0; i < 32; ++i)
        {
            reg_prev[i] = reg_succ[i];
        }
        cdb_slb_prev = cdb_slb_next;
    }
};

#endif

int main()
{
    //    freopen("b.out","w",stdout);
    simulator RISC_v;
    RISC_v.scan();
    RISC_v.run();
    return 0;
}
