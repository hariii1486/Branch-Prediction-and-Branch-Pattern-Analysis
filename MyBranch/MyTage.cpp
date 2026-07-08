#include <iostream>
#include <vector>
#include <map>
#include <fstream>
#include <string>
#include "pin.H"
#include <iomanip>

// =========================
// CONFIG
// =========================
#define T1_SIZE 512
#define T2_SIZE 512
#define HIST1 8
#define HIST2 16
#define DEBUG 0

// cd ~/CS204_PROJECT/dashboard
// python3 -m http.server
// http://localhost:8000/

// $PIN_ROOT/pin -t obj-intel64/MyBranch.so -o test1 -- ./basic1
// $PIN_ROOT/pin -t obj-intel64/MyTage.so -o test1 -- ./basic1

// $PIN_ROOT/pin -t obj-intel64/MyBranch.so -o test2 -- ./bubble2
// $PIN_ROOT/pin -t obj-intel64/MyTage.so -o test2 -- ./bubble2

// $PIN_ROOT/pin -t obj-intel64/MyBranch.so -o test3 -- ./matrix3
// $PIN_ROOT/pin -t obj-intel64/MyTage.so -o test3 -- ./matrix3

// $PIN_ROOT/pin -t obj-intel64/MyBranch.so -o test4 -- ./bfs4
// $PIN_ROOT/pin -t obj-intel64/MyTage.so -o test4 -- ./bfs4

// $PIN_ROOT/pin -t obj-intel64/MyBranch.so -o test5 -- ./logs5
// $PIN_ROOT/pin -t obj-intel64/MyTage.so -o test5 -- ./logs5

// =========================
// GLOBAL STATE
// =========================
struct BranchTrace {
    char outcome;
    ADDRINT target;
};

UINT64 GHR = 0;
UINT64 GHR_MASK = (1ULL << HIST2) - 1;

// Maps branch PC to a list of (Outcome, Target)
std::map<ADDRINT, std::vector<BranchTrace>> history;
std::map<ADDRINT, UINT8> T0;
std::map<ADDRINT, int> mispredPerBranch;

struct TageEntry {
    UINT16 tag = 0;
    INT8 counter = 0;   // range: -2 to +3
    UINT8 u = 0;        // usefulness
};

TageEntry T1[T1_SIZE];
TageEntry T2[T2_SIZE];

UINT64 total = 0;
UINT64 taken = 0, nottaken = 0;
UINT64 tagePred = 0;
UINT64 tageCorrect = 0;
UINT64 clock_ = 0;

ADDRINT low = 0, high = 0;

// =========================
// HASH FUNCTIONS
// =========================
UINT32 get_index(ADDRINT ip, UINT64 history_val, int hist_len, int size) {
    UINT64 h = history_val & ((1ULL << hist_len) - 1);
    h ^= (h >> 3) ^ (h << 5);
    return (ip ^ h ^ (ip >> 2)) & (size - 1);
}

UINT16 get_tag(ADDRINT ip, UINT64 history_val, int hist_len) {
    UINT64 h = history_val & ((1ULL << hist_len) - 1);
    return (ip ^ (h >> 2) ^ (h << 1)) & 0xFFFF;
}

// =========================
// MAIN BRANCH HANDLER
// =========================
VOID RecordBranch(ADDRINT ip, ADDRINT target, BOOL isTaken)
{
    if (ip < low || ip > high) return;

    total++;
    bool actual = (isTaken != 0);

    UINT32 idx1 = get_index(ip, GHR, HIST1, T1_SIZE);
    UINT32 idx2 = get_index(ip, GHR, HIST2, T2_SIZE);

    UINT16 tag1 = get_tag(ip, GHR, HIST1);
    UINT16 tag2 = get_tag(ip, GHR, HIST2);

    bool hit1 = (T1[idx1].tag == tag1);
    bool hit2 = (T2[idx2].tag == tag2);

    if (T0.find(ip) == T0.end()) T0[ip] = 2;
    bool basePred = (T0[ip] >= 2);

    bool altPred = basePred;
    if (hit1) altPred = (T1[idx1].counter >= 0);

    TageEntry *provider = nullptr;
    if (hit2) provider = &T2[idx2];
    else if (hit1) provider = &T1[idx1];

    bool prediction;
    if (provider) {
        prediction = (provider->u == 0) ? altPred : (provider->counter >= 0);
    } else {
        prediction = basePred;
    }

    tagePred++;
    if (prediction == actual) tageCorrect++;
    else mispredPerBranch[ip]++;

    // Update Base
    UINT8 &base = T0[ip];
    if (actual) { if (base < 3) base++; }
    else { if (base > 0) base--; }

    // Update Provider
    if (provider) {
        if (actual) { if (provider->counter < 3) provider->counter++; }
        else { if (provider->counter > -2) provider->counter--; }

        if (prediction == actual) { if (provider->u < 3) provider->u++; }
        else { if (provider->u > 0) provider->u--; }
    }

    // Allocation
    if (prediction != actual) {
        if (!hit2 && T2[idx2].u == 0) {
            T2[idx2].tag = tag2;
            T2[idx2].counter = actual ? 0 : -1;
            T2[idx2].u = 0;
        }
        else if (!hit1 && T1[idx1].u == 0) {
            T1[idx1].tag = tag1;
            T1[idx1].counter = actual ? 0 : -1;
            T1[idx1].u = 0;
        }
    }

    // Update Global History
    GHR = ((GHR << 1) | (actual ? 1 : 0)) & GHR_MASK;

    // Record Trace with Target PC
    if (actual) taken++; else nottaken++;
    BranchTrace bt = { (actual ? 'T' : 'N'), target };
    history[ip].push_back(bt);

    // Aging
    clock_++;
    if ((clock_ & 1023) == 0) {
        for (int i = 0; i < T1_SIZE; i++) T1[i].u >>= 1;
        for (int i = 0; i < T2_SIZE; i++) T2[i].u >>= 1;
    }
}

// =========================
// PIN HOOKS
// =========================
VOID Instruction(INS ins, VOID *v) {
    if (INS_IsBranch(ins)) {
        INS_InsertPredicatedCall(
            ins, IPOINT_BEFORE, (AFUNPTR)RecordBranch,
            IARG_INST_PTR, IARG_BRANCH_TARGET_ADDR, IARG_BRANCH_TAKEN, IARG_END);
    }
}

VOID Image(IMG img, VOID *v) {
    if (IMG_IsMainExecutable(img)) {
        low = IMG_LowAddress(img);
        high = IMG_HighAddress(img);
    }
}

KNOB<std::string> KnobOutputFile(KNOB_MODE_WRITEONCE, "pintool", "o", "test1", "specify name");

VOID Fini(INT32 code, VOID *v)
{
    // --- 1. CONSOLE OUTPUT ---
    std::cout << "\n===============================================" << std::endl;
    std::cout << "           TAGE PREDICTOR STATISTICS           " << std::endl;
    std::cout << "===============================================" << std::endl;
    std::cout << "Total Branches: " << std::dec << total << std::endl;
    std::cout << "Taken:          " << taken << std::endl;
    std::cout << "Not Taken:      " << nottaken << std::endl;

    std::cout << "\n--- BRANCH TRACE (Source -> Target : History) ---" << std::endl;
    for (auto const& [pc, traces] : history) {
        // We print the PC and the most recent target address used by this branch
        std::cout << "0x" << std::hex << pc << " -> 0x" << traces.back().target << " : ";
        for (auto const& entry : traces) std::cout << entry.outcome;
        std::cout << std::dec << " (" << traces.size() << " calls)" << std::endl;
    }

    double acc = (tagePred > 0) ? (double)tageCorrect / tagePred * 100.0 : 0;
    std::cout << "\n--- ACCURACY REPORT ---" << std::endl;
    std::cout << "TAGE Accuracy: " << std::fixed << std::setprecision(2) << acc << "%" << std::endl;
    std::cout << "Mispredictions: " << (tagePred - tageCorrect) << std::endl;
    std::cout << "===============================================\n" << std::endl;

    // --- 2. JSON OUTPUT ---
    std::string baseName = KnobOutputFile.Value();
    std::string fullPath = "/home/hansika/CS204_PROJECT/dashboard/results/" + baseName + "_tage.json";
    std::ofstream out(fullPath.c_str());

    if (out.is_open()) {
        out << "{\n  \"summary\": {\n";
        out << "    \"total\": " << total << ",\n";
        out << "    \"best_accuracy\": " << acc << ",\n";
        out << "    \"winner\": \"TAGE\",\n";
        out << "    \"mispred\": " << (tagePred - tageCorrect) << "\n  },\n";
        out << "  \"predictors\": {\n    \"TAGE\": " << acc << "\n  }\n}";
        out.close();
    }
}

int main(int argc, char *argv[]) {
    if (PIN_Init(argc, argv)) return -1;
    IMG_AddInstrumentFunction(Image, 0);
    INS_AddInstrumentFunction(Instruction, 0);
    PIN_AddFiniFunction(Fini, 0);
    PIN_StartProgram();
    return 0;
}