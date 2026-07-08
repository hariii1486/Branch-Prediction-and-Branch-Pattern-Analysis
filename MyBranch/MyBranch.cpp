#include <iostream>
#include <map>
#include <vector>
#include "pin.H"
#include <fstream>
#include <string>
#include <iomanip>

// Structure to hold individual branch trace info
struct BranchTrace {
    char outcome;
    ADDRINT target;
};

std::map<ADDRINT, int> mispredBranch;
std::map<ADDRINT, std::vector<BranchTrace>> history; 

// Predictor States
std::map<ADDRINT, bool> BHT;     
std::map<ADDRINT, UINT8> BHT2;     
std::map<ADDRINT, UINT8> selector; 

// Global Counters
UINT64 predictions = 0, correct = 0;   
UINT64 pred2 = 0, correct2 = 0;       
UINT64 predH = 0, correctH = 0;       
UINT64 total = 0, taken = 0, nottaken = 0;

ADDRINT low = 0, high = 0;

KNOB<std::string> KnobOutputFile(KNOB_MODE_WRITEONCE, "pintool",
    "o", "test1", "specify base name for the output JSON");

VOID RecordBranch(ADDRINT ip, ADDRINT target, BOOL isTaken)
{
    if (ip < low || ip > high) return;

    total++;
    bool actual = (isTaken != 0);

    //  1-bit prediction logic 
    bool pred1_valid = (BHT.find(ip) != BHT.end());
    bool pred1 = pred1_valid ? BHT[ip] : actual;
    predictions++;
    if (pred1 == actual) correct++;

    //  2-bit prediction logic 
    bool pred2_valid = (BHT2.find(ip) != BHT2.end());
    UINT8 state2 = pred2_valid ? BHT2[ip] : 2; 
    bool pred2_val = (state2 >= 2);
    pred2++;
    if (pred2_val == actual) correct2++;

    //  Hybrid prediction logic 
    if (selector.find(ip) == selector.end()) selector[ip] = 2;
    UINT8 sel = selector[ip];
    bool final_pred = (sel <= 1) ? pred1 : pred2_val;
    predH++;
    if (final_pred == actual) correctH++;

    //  Update Selectors and Predictors 
    if (pred1_valid && pred2_valid && (pred1 != pred2_val)) {
        if (pred2_val == actual && selector[ip] < 3) selector[ip]++;
        else if (pred1 == actual && selector[ip] > 0) selector[ip]--;
    }

    BHT[ip] = actual; // Update 1-bit

    if (!pred2_valid) BHT2[ip] = 2;
    if (actual) { if (BHT2[ip] < 3) BHT2[ip]++; }
    else { if (BHT2[ip] > 0) BHT2[ip]--; }

    //  Store Stats and Trace 
    if (isTaken) taken++; else nottaken++;
    
    BranchTrace bt = { (isTaken ? 'T' : 'N'), target };
    history[ip].push_back(bt);

    if (final_pred != actual) mispredBranch[ip]++;
}

VOID Instruction(INS ins, VOID *v)
{
    if (INS_IsBranch(ins))
    {
        INS_InsertPredicatedCall(
            ins, IPOINT_BEFORE, (AFUNPTR)RecordBranch,
            IARG_INST_PTR,
            IARG_BRANCH_TARGET_ADDR,
            IARG_BRANCH_TAKEN,
            IARG_END);
    }
}

VOID Image(IMG img, VOID *v)
{
    if (IMG_IsMainExecutable(img)) {
        low = IMG_LowAddress(img);
        high = IMG_HighAddress(img);
    }
}

VOID Fini(INT32 code, VOID *v)
{
    //std::cout << "\n===============================================" << std::endl;
    std::cout << "           BRANCH PREDICTION STATISTICS          " << std::endl;
    //std::cout << "===============================================" << std::endl;
    std::cout << "Total Branches: " << std::dec << total << std::endl;
    std::cout << "Taken:          " << taken << std::endl;
    std::cout << "Not Taken:      " << nottaken << std::endl;

    std::cout << "\n--- DETAILED BRANCH TRACE (Source -> Target : History) ---" << std::endl;
    for (auto const& [pc, traces] : history)
    {
        // Print Source and the target of the last occurrence for context
        std::cout << "0x" << std::hex << pc << " -> 0x" << traces.back().target << " : ";
        for (auto const& entry : traces) std::cout << entry.outcome;
        std::cout << std::dec << " (" << traces.size() << " calls)" << std::endl;
    }

    auto printStat = [](std::string name, UINT64 p, UINT64 c) {
        double acc = (p > 0) ? (double)c / p * 100.0 : 0;
        std::cout << "\n--- " << name << " ---" << std::endl;
        std::cout << "Accuracy: " << std::fixed << std::setprecision(2) << acc << "%" << std::endl;
        std::cout << "Mispredictions: " << (p - c) << std::endl;
    };

    printStat("1-BIT PREDICTOR", predictions, correct);
    printStat("2-BIT PREDICTOR", pred2, correct2);
    printStat("HYBRID PREDICTOR", predH, correctH);

    // --- 2. JSON FILE OUTPUT ---
    std::string baseName = KnobOutputFile.Value();
    std::string fullPath = "/home/hansika/CS204_PROJECT/dashboard/results/" + baseName + ".json";
    std::ofstream out(fullPath.c_str());

    if (out.is_open()) {
        double acc1 = (predictions > 0) ? (double)correct / predictions * 100.0 : 0;
        double acc2 = (pred2 > 0) ? (double)correct2 / pred2 * 100.0 : 0;
        double accH = (predH > 0) ? (double)correctH / predH * 100.0 : 0;

        out << "{\n  \"summary\": {\n";
        out << "    \"total\": " << total << ",\n";
        out << "    \"best_accuracy\": " << accH << ",\n";
        out << "    \"winner\": \"Hybrid\",\n";
        out << "    \"mispred\": " << (predH - correctH) << "\n  },\n";
        
        out << "  \"predictors\": {\n";
        out << "    \"1-bit\": " << acc1 << ",\n";
        out << "    \"2-bit\": " << acc2 << ",\n";
        out << "    \"Hybrid\": " << accH << "\n  },\n";

        out << "  \"branches\": [\n";
        for (auto it = mispredBranch.begin(); it != mispredBranch.end(); ++it) {
            out << "    {\"pc\": \"0x" << std::hex << it->first << "\", \"miss\": " << std::dec << it->second << "}";
            if (std::next(it) != mispredBranch.end()) out << ",";
            out << "\n";
        }
        out << "  ]\n}";
        out.close();
    }
}

int main(int argc, char *argv[])
{
    if (PIN_Init(argc, argv)) return -1;
    IMG_AddInstrumentFunction(Image, 0);
    INS_AddInstrumentFunction(Instruction, 0);
    PIN_AddFiniFunction(Fini, 0);
    PIN_StartProgram();
    return 0;
}