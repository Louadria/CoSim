// =========================================================================
//
//  File Name:         rv32i_cpu.h
//  Design Unit Name:
//  Revision:          OSVVM MODELS STANDARD VERSION
//
//  Maintainer:        Simon Southwell email:  simon.southwell@gmail.com
//  Contributor(s):
//    Simon Southwell      simon.southwell@gmail.com
//
//  Description:
//    Contains the header for the rv32i_cpu class
//    
//    This file is part of the base RISC-V instruction set simulator
//    (rv32i_cpu).
//
//  Revision History:
//    Date      Version    Description
//    01/2023   2023.01    Released with OSVVM CoSim
//    28th June 2021       Earlier version
//
//  This file is part of OSVVM.
//
//  Copyright (c) 2021 Simon Southwell. 
//
//  Licensed under the Apache License, Version 2.0 (the "License");
//  you may not use this file except in compliance with the License.
//  You may obtain a copy of the License at
//
//      https://www.apache.org/licenses/LICENSE-2.0
//
//  Unless required by applicable law or agreed to in writing, software
//  distributed under the License is distributed on an "AS IS" BASIS,
//  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
//  See the License for the specific language governing permissions and
//  limitations under the License.
//
// =========================================================================


#ifndef _RV32I_CPU_H_
#define _RV32I_CPU_H_

// -------------------------------------------------------------------------
// INCLUDES
// -------------------------------------------------------------------------

#include <chrono>
#include <cstdio>
#include <cstdint>
#include <cstring>

#include "rv32i_cpu_hdr.h"

// -------------------------------------------------------------------------
// DEFINES
// -------------------------------------------------------------------------

// -------------------------------------------------------------------------
// Class definition for RISC-V RV32I instruction set simulator model
// -------------------------------------------------------------------------

class rv32i_cpu
{
public:

    // ------------------------------------------------
    // Public class definitions
    // ------------------------------------------------

    // Define a class to hold all of the CPU registers. This makes it easier
    // to access all of the state as a single unit for debug purposes, and
    // save & restore features.
    class rv32i_hart_state
    {
    public:

        // Register types set at 64 bit to allow for RV64 extensions

        // General purpose registers
        uint64_t x[RV32I_NUM_OF_REGISTERS] = { 0 };

        // Floating point registers (for RV32F/RV32D)
        uint64_t f[RV32I_NUM_OF_REGISTERS] = { 0 };

        // CSR registers
        uint64_t csr[RV32I_CSR_SPACE_SIZE] = { 0 };

        // Program counter
        uint64_t pc;

    };

    // Define a class to hold the registers (times the number of
    // supported harts) and other internal state
    class rv32i_state
    {
    public:

        rv32i_hart_state  hart[RV32I_NUM_OF_HARTS];

        // Current privilege level
        uint32_t          priv_lvl = RV32_PRIV_MACHINE;

    };

    // ------------------------------------------------
    // Constructors/destructors
    // ------------------------------------------------

    // Constructor, with default configuration values
            LIBRISCV32_API     rv32i_cpu                     (FILE* dbgfp = stdout);

    // Virtual destructor for polymorphic class
    virtual LIBRISCV32_API     ~rv32i_cpu                    (){/* No dynamic allocation yet */}

    // ------------------------------------------------
    // Public methods (user interface)
    // ------------------------------------------------

    LIBRISCV32_API int         run                            (rv32i_cfg_s &cfg);

    // Read executable
    LIBRISCV32_API int         read_elf                       (const char* const filename);

    // External direct memory access
    LIBRISCV32_API uint32_t    read_mem                       (const uint32_t byte_addr, const int type, bool &fault);
    LIBRISCV32_API void        write_mem                      (const uint32_t byte_addr, const uint32_t data, const int type, bool &fault);

    // Callback function registration
    LIBRISCV32_API void        register_ext_mem_callback      (p_rv32i_memcallback_t callback_func) { p_mem_callback = callback_func; };

    // Reset the cpu (i.e. generate a reset pin assertion event)
    LIBRISCV32_API void        reset_cpu                      (void)                                { reset(); };

    // Return value of indexed integer register
    LIBRISCV32_API uint32_t    regi_val                       (uint32_t reg_idx)
    {
        return (uint32_t)state.hart[curr_hart].x[reg_idx % RV32I_NUM_OF_REGISTERS];
    };

    // Return value of indexed integer register
    LIBRISCV32_API uint32_t    pc_val                         ()
    {
        return (uint32_t)state.hart[curr_hart].pc;
    };

    LIBRISCV32_API rv32i_hart_state rv32_get_cpu_state(int hart_num = 0)                            { return state.hart[hart_num]; }
    LIBRISCV32_API void             rv32_set_cpu_state(rv32i_hart_state &s, int hart_num = 0)       { state.hart[hart_num] = s; }

    // Dummy method so compilation works when rv32csr_cpu class not included in inheritance.
    // Will be overloaded by rv32csr_cpu.
    LIBRISCV32_API void          register_int_callback(p_rv32i_intcallback_t callback_func) { };

    // ------------------------------------------------
    // Public member variables
    // ------------------------------------------------
public:

    // Mappings of register indexes to register name strings
    const char* rmap_str[32] = { "zero", "ra", "sp",  "gp",  "tp", "t0", "t1", "t2",
                                 "s0",   "s1", "a0",  "a1",  "a2", "a3", "a4", "a5",
                                 "a6",   "a7", "s2",  "s3",  "s4", "s5", "s6", "s7",
                                 "s8",   "s9", "s10", "s11", "t3", "t4", "t5", "t6"};
    // Mappings of register indexes to register name strings
    const char* xmap_str[32] = { "x0",  "x1",  "x2",  "x3",  "x4",  "x5",  "x6",  "x7",
                                 "x8",  "x9",  "x10", "x11", "x12", "x13", "x14", "x15",
                                 "x16", "x17", "x18", "x19", "x20", "x21", "x22", "x23",
                                 "x24", "x25", "x26", "x27", "x28", "x29", "x30", "x31"};

    // Mappings of register indexes to register name strings
    const char* fmap_str[32] = { "ft0", "ft1", "ft2",  "ft3",  "ft4", "ft5", "ft6",  "ft7",
                                 "fs0", "fs1", "fa0",  "fa1",  "fa2", "fa3", "fa4",  "fa5",
                                 "fa6", "fa7", "fs2",  "fs3",  "fs4", "fs5", "fs6",  "fs7",
                                 "fs8", "fs9", "fs10", "fs11", "ft8", "ft9", "ft10", "ft11"};

    // Mappings of register indexes to register name strings
    const char* fxmap_str[32] = { "f0",  "f1",  "f2",  "f3",  "f4",  "f5",  "f6",  "f7",
                                  "f8",  "f9",  "f10", "f11", "f12", "f13", "f14", "f15",
                                  "f16", "f17", "f18", "f19", "f20", "f21", "f22", "f23",
                                  "f24", "f25", "f26", "f27", "f28", "f29", "f30", "f31"};

    // ------------------------------------------------
    // Protected member variables
    // ------------------------------------------------
protected:
    // Clock cycle count. Protected status so that rvcsr_cpu overriding of process_trap()
    // method can update it on exceptions.
    rv32i_time_t          cycle_count;

    // ------------------------------------------------
    // Internal constant definitions
    // ------------------------------------------------


    // String constants for instruction disassembly
    static const char reserved_str[DISASSEM_STR_SIZE] ;
    static const char lb_str      [DISASSEM_STR_SIZE] ;
    static const char lh_str      [DISASSEM_STR_SIZE] ;
    static const char lw_str      [DISASSEM_STR_SIZE] ;
    static const char lbu_str     [DISASSEM_STR_SIZE] ;
    static const char lhu_str     [DISASSEM_STR_SIZE] ;
    static const char sb_str      [DISASSEM_STR_SIZE] ;
    static const char sh_str      [DISASSEM_STR_SIZE] ;
    static const char sw_str      [DISASSEM_STR_SIZE] ;
    static const char beq_str     [DISASSEM_STR_SIZE] ;
    static const char bne_str     [DISASSEM_STR_SIZE] ;
    static const char blt_str     [DISASSEM_STR_SIZE] ;
    static const char bge_str     [DISASSEM_STR_SIZE] ;
    static const char bltu_str    [DISASSEM_STR_SIZE] ;
    static const char bgeu_str    [DISASSEM_STR_SIZE] ;
    static const char jalr_str    [DISASSEM_STR_SIZE] ;
    static const char jal_str     [DISASSEM_STR_SIZE] ;
    static const char fence_str   [DISASSEM_STR_SIZE] ;
    static const char addi_str    [DISASSEM_STR_SIZE] ;
    static const char slti_str    [DISASSEM_STR_SIZE] ;
    static const char sltiu_str   [DISASSEM_STR_SIZE] ;
    static const char xori_str    [DISASSEM_STR_SIZE] ;
    static const char ori_str     [DISASSEM_STR_SIZE] ;
    static const char andi_str    [DISASSEM_STR_SIZE] ;
    static const char slli_str    [DISASSEM_STR_SIZE] ;
    static const char srli_str    [DISASSEM_STR_SIZE] ;
    static const char srai_str    [DISASSEM_STR_SIZE] ;
    static const char add_str     [DISASSEM_STR_SIZE] ;
    static const char sub_str     [DISASSEM_STR_SIZE] ;
    static const char sll_str     [DISASSEM_STR_SIZE] ;
    static const char slt_str     [DISASSEM_STR_SIZE] ;
    static const char sltu_str    [DISASSEM_STR_SIZE] ;
    static const char xor_str     [DISASSEM_STR_SIZE] ;
    static const char srl_str     [DISASSEM_STR_SIZE] ;
    static const char sra_str     [DISASSEM_STR_SIZE] ;
    static const char or_str      [DISASSEM_STR_SIZE] ;
    static const char and_str     [DISASSEM_STR_SIZE] ;
    static const char ecall_str   [DISASSEM_STR_SIZE] ;
    static const char ebrk_str    [DISASSEM_STR_SIZE] ;
    static const char auipc_str   [DISASSEM_STR_SIZE] ;
    static const char lui_str     [DISASSEM_STR_SIZE] ;

    // ------------------------------------------------
    // Internal Type definitions
    // ------------------------------------------------

    typedef uint32_t opcode_t;

    // Disassembly enable flags
    bool                  disassemble;               // Dissassemble mode
    bool                  rt_disassem;               // Disassemble during runtime

    // Flag to halt on a reserved instruction
    bool                  halt_rsvd_instr;

    // Flag to halt on ecall
    bool                  halt_ecall;

    // Debug ABI register names enable flag
    bool                  abi_en;

    // Holds CSR and HART (pc and regs) state
    rv32i_state           state;

    // Current active hart
    uint32_t              curr_hart;

    // File pointer for disassembler (and other debug) output
    FILE*                 dasm_fp;

    // Load/store or jump target address (for trap handling)
    uint32_t              access_addr;

    // Flag to say a compressed instruction being processed (for use by rv32c_cpu)
    bool                  cmp_instr;
    uint32_t              cmp_instr_code;
    uint32_t              RV32_IADDR_ALIGN_MASK;

    // RV32I Decode tables
    rv32i_decode_table_t  primary_tbl    [RV32I_NUM_PRIMARY_OPCODES];
    rv32i_decode_table_t  load_tbl       [RV32I_NUM_SECONDARY_OPCODES];
    rv32i_decode_table_t  store_tbl      [RV32I_NUM_SECONDARY_OPCODES];
    rv32i_decode_table_t  branch_tbl     [RV32I_NUM_SECONDARY_OPCODES];
    rv32i_decode_table_t  op_imm_tbl     [RV32I_NUM_SECONDARY_OPCODES];
    rv32i_decode_table_t  op_tbl         [RV32I_NUM_SECONDARY_OPCODES];
    rv32i_decode_table_t  arith_tbl      [RV32I_NUM_TERTIARY_OPCODES];
    rv32i_decode_table_t  sri_tbl        [RV32I_NUM_TERTIARY_OPCODES];
    rv32i_decode_table_t  srr_tbl        [RV32I_NUM_TERTIARY_OPCODES];
    rv32i_decode_table_t  sll_tbl        [RV32I_NUM_TERTIARY_OPCODES];
    rv32i_decode_table_t  slt_tbl        [RV32I_NUM_TERTIARY_OPCODES];
    rv32i_decode_table_t  sltu_tbl       [RV32I_NUM_TERTIARY_OPCODES];
    rv32i_decode_table_t  xor_tbl        [RV32I_NUM_TERTIARY_OPCODES];
    rv32i_decode_table_t  or_tbl         [RV32I_NUM_TERTIARY_OPCODES];
    rv32i_decode_table_t  and_tbl        [RV32I_NUM_TERTIARY_OPCODES];

    // Decode table for SYSTEM instructions
    rv32i_decode_table_t  sys_tbl        [RV32I_NUM_SECONDARY_OPCODES];
    rv32i_decode_table_t  e_tbl          [RV32I_NUM_SYSTEM_OPCODES];

    // ------------------------------------------------
    // Private member variables
    // ------------------------------------------------
 private:

    // Trap status
    int32_t               trap;

    // Internal memory
    uint8_t               internal_mem   [4*RV32I_INT_MEM_WORDS+4];

    // Instructions retired count
    rv32i_time_t          instret_count;

    // Real time counter comparator value
    rv32i_time_t          mtimecmp;

    // String forming scratch space
    char                  str            [NUM_DISASSEM_BUFS][DISASSEM_STR_SIZE];
    int                   str_idx;

    // Pointer to external memory callback function
    p_rv32i_memcallback_t p_mem_callback;

    // Current instruction
    uint32_t              curr_instr;

    // Reset vector
    uint32_t              reset_vector;

    // ------------------------------------------------
    // Virtual methods
    // ------------------------------------------------
public:
    // Instruction for illegal/unimplemented instructions. Public
    // so derived classes can use same function, and virtual so
    // can be overloaded.
    virtual void     reserved             (const p_rv32i_decode_t);

protected:
    // State reset
    virtual void     reset                ();

    // Increment PC. For RV32I always 4, but can be overridden
    // to support compressed instructions (RV32C)
    virtual void increment_pc()
    {
        state.hart[curr_hart].pc = (uint32_t)(state.hart[curr_hart].pc + 4);
    }

    // Place holder virtual methods for overloading with CSR access functionality
    virtual uint32_t access_csr(const unsigned funct3, const uint32_t addr, const uint32_t rd, const uint32_t value) { return 1;}
    virtual uint32_t csr_wr_mask(const uint32_t addr, bool& unimp) { unimp = true; return 0;}

    // Fetch next instruction. For RV32I, always a simple 32 bit read.
    // Can be overridden to support compressed instructions (RV32C),
    // expanding to 32 bits, and managing half word PC increments.
    virtual uint32_t fetch_instruction()
    {
        bool fault;
        return read_mem((uint32_t)state.hart[curr_hart].pc, MEM_RD_ACCESS_INSTR, fault);
    }

private:
    // RV32I trap processing. Since CSR registers not implemented,
    // the PC is redirected to a fixed trap location. Can be overridden
    // to implement full trap support and updating of CSR registers.
    virtual void process_trap(int trap_type = 0)
    {
        state.hart[curr_hart].pc = RV32I_FIXED_MTVEC_ADDR;
        cycle_count += RV32I_TRAP_EXTRA_CYCLES;
    }

    // Virtual place holder for adding interrupt features
    // (external time and software. Called once per execute() cycle.
    virtual int process_interrupts() { return 0; };

    virtual void decode_exception(rv32i_decode_table_t*& pEntry, rv32i_decode_t& d)
    {
        pEntry = NULL;
    }

    // ------------------------------------------------
    // Protected methods
    // ------------------------------------------------

protected:

    // Disassembly register name decode to a fixed width string
    // (Uses [and clobbers] scratch member variable "str and its
    // index, str_idx")
    inline char* rmap                    (uint32_t r, int slen = 5)
    {
        // Move to next string buffer
        str_idx = (str_idx + 1) % NUM_DISASSEM_BUFS;

        strcpy(str[str_idx], "         ");                            // Initialise with spaces

        // Select approriate mapping table on whethjer ABI or normal register naming
        const char* map_str = abi_en ? rmap_str[r] : xmap_str[r];

        size_t len         = strlen(map_str);                         // Get length of indexed abi string

        strncpy(str[str_idx], map_str, DISASSEM_STR_SIZE);            // Lookup and copy abi string from reg index

        str[str_idx][len]  = ',';                                     // Add a comma
        str[str_idx][slen] = 0;                                       // Terminate string at fixed width

        return str[str_idx];
    }

    // Disassembly register name decode to a fixed width string
    // (Uses [and clobbers] scratch member variable "str and its
    // index, str_idx")
    inline char* fmap                    (uint32_t r, int slen = 5)
    {
        // Move to next string buffer
        str_idx = (str_idx + 1) % NUM_DISASSEM_BUFS;

        strcpy(str[str_idx], "         ");                            // Initialise with spaces

        // Select approriate mapping table on whethjer ABI or normal register naming
        const char* map_str = abi_en ? fmap_str[r] : fxmap_str[r];

        size_t len = strlen(map_str);                                 // Get length of indexed abi string

        strncpy(str[str_idx], map_str, DISASSEM_STR_SIZE);            // Lookup and copy abi string from reg index

        str[str_idx][len]  = ',';                                     // Add a comma
        str[str_idx][slen] = 0;                                       // Terminate string at fixed width

        return str[str_idx];
    }

    // Return real time as the number of microseconds
    inline uint64_t real_time_us() {
        using namespace std::chrono;
        return time_point_cast<microseconds>(system_clock::now()).time_since_epoch().count();
    };

    inline uint64_t clk_cycles() {
        return cycle_count;
    }

    inline uint64_t inst_retired() {
        return instret_count;
    }

    inline uint32_t get_curr_instruction()
    {
        return curr_instr;
    }

    inline uint32_t get_last_access_addr()
    {
        return access_addr;
    }

    // ------------------------------------------------
    // Private methods
    // ------------------------------------------------
private:

    // Execution of instruction method
    int  execute                         (rv32i_decode_t &decode, rv32i_decode_table_t*);

    // Primary instruction decode method
    rv32i_decode_table_t* primary_decode (const opcode_t instr, rv32i_decode_t& decoded_data);

    // ------------------------------------------------
    // Instruction methods
    // ------------------------------------------------

    // RV32I instruction methods
    void lui                             (const p_rv32i_decode_t);
    void auipc                           (const p_rv32i_decode_t);

    void jal                             (const p_rv32i_decode_t);
    void jalr                            (const p_rv32i_decode_t);

    void beq                             (const p_rv32i_decode_t);
    void bne                             (const p_rv32i_decode_t);
    void blt                             (const p_rv32i_decode_t);
    void bge                             (const p_rv32i_decode_t);
    void bltu                            (const p_rv32i_decode_t);
    void bgeu                            (const p_rv32i_decode_t);

    void lb                              (const p_rv32i_decode_t);
    void lh                              (const p_rv32i_decode_t);
    void lw                              (const p_rv32i_decode_t);
    void lbu                             (const p_rv32i_decode_t);
    void lhu                             (const p_rv32i_decode_t);
    void sb                              (const p_rv32i_decode_t);
    void sh                              (const p_rv32i_decode_t);
    void sw                              (const p_rv32i_decode_t);

    void addi                            (const p_rv32i_decode_t);
    void slti                            (const p_rv32i_decode_t);
    void sltiu                           (const p_rv32i_decode_t);
    void xori                            (const p_rv32i_decode_t);
    void ori                             (const p_rv32i_decode_t);
    void andi                            (const p_rv32i_decode_t);
    void slli                            (const p_rv32i_decode_t);
    void srli                            (const p_rv32i_decode_t);
    void srai                            (const p_rv32i_decode_t);

    void addr                            (const p_rv32i_decode_t);
    void subr                            (const p_rv32i_decode_t);
    void sllr                            (const p_rv32i_decode_t);
    void sltr                            (const p_rv32i_decode_t);
    void sltur                           (const p_rv32i_decode_t);
    void xorr                            (const p_rv32i_decode_t);
    void srlr                            (const p_rv32i_decode_t);
    void srar                            (const p_rv32i_decode_t);
    void orr                             (const p_rv32i_decode_t);
    void andr                            (const p_rv32i_decode_t);

    void fence                           (const p_rv32i_decode_t);

    // RV32I virtual system instructions
    void ecall                           (const p_rv32i_decode_t);
    void ebreak                          (const p_rv32i_decode_t);
};

#endif
