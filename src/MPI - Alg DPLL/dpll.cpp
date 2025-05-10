#include <iostream>
#include <fstream>
#include <sstream>
#include <cstdlib>
#include <chrono>
#include <iomanip>
#include <cstring>
#include <dirent.h>

using namespace std;

const int MAX_VARS = 300;
const int MAX_CLAUSES = 5000;
const int MAX_LITS_PER_CLAUSE = 20;
const int NUM_RUNS = 1;

int formula[MAX_CLAUSES][MAX_LITS_PER_CLAUSE];
int clauseLens[MAX_CLAUSES];
int numVars = 0;
int numClauses = 0;
int assignment[MAX_VARS + 1];

bool isClauseSatisfied(int idx) {
    for (int i = 0; i < clauseLens[idx]; ++i) {
        int lit = formula[idx][i];
        int var = abs(lit);
        if (assignment[var] == -1) continue;
        if ((lit > 0 && assignment[var] == 1) || (lit < 0 && assignment[var] == 0))
            return true;
    }
    return false;
}

bool hasEmptyClause() {
    for (int i = 0; i < numClauses; ++i) {
        bool sat = false;
        bool unassigned = false;
        for (int j = 0; j < clauseLens[i]; ++j) {
            int lit = formula[i][j];
            int var = abs(lit);
            if (assignment[var] == -1) {
                unassigned = true;
                continue;
            }
            if ((lit > 0 && assignment[var] == 1) || (lit < 0 && assignment[var] == 0)) {
                sat = true;
                break;
            }
        }
        if (!sat && !unassigned) return true;
    }
    return false;
}

bool DPLL(int varIdx) {
    if (varIdx > numVars) {
        for (int i = 0; i < numClauses; ++i)
            if (!isClauseSatisfied(i)) return false;
        return true;
    }

    for (int val = 0; val <= 1; ++val) {
        assignment[varIdx] = val;
        if (!hasEmptyClause() && DPLL(varIdx + 1))
            return true;
    }

    assignment[varIdx] = -1;
    return false;
}

bool loadDIMACS(const char* filename) {
    ifstream infile(filename);
    if (!infile) {
        cerr << "Eroare la deschiderea fisierului: " << filename << endl;
        return false;
    }

    string line;
    numClauses = 0;

    while (getline(infile, line)) {
        if (line.empty() || line[0] == 'c') continue;
        if (line[0] == 'p') {
            istringstream iss(line);
            string tmp;
            iss >> tmp >> tmp >> numVars >> tmp; // numar variabile
            continue;
        }

        istringstream iss(line);
        int lit;
        clauseLens[numClauses] = 0;
        while (iss >> lit && lit != 0) {
            if (clauseLens[numClauses] < MAX_LITS_PER_CLAUSE) {
                formula[numClauses][clauseLens[numClauses]++] = lit;
            }
        }
        numClauses++;
    }

    infile.close();
    return true;
}

string findFirstCNFFile() {
    DIR* dir;
    struct dirent* entry;
    dir = opendir(".");
    if (!dir) {
        cerr << "Eroare la deschiderea directorului curent.\n";
        return "";
    }

    while ((entry = readdir(dir)) != NULL) {
        const char* name = entry->d_name;
        size_t len = strlen(name);
        if (len > 4 && strcmp(name + len - 4, ".cnf") == 0) {
            closedir(dir);
            return string(name);
        }
    }

    closedir(dir);
    return "";
}

int main() {
    string filename = findFirstCNFFile();
    if (filename.empty()) {
        cout << "Nu s-a gasit niciun fisier .cnf in directorul curent." << endl;
        return 1;
    }

    if (!loadDIMACS(filename.c_str())) return 1;

    double total_time = 0.0;
    bool final_result = false;

    for (int run = 0; run < NUM_RUNS; ++run) {
        for (int i = 1; i <= numVars; ++i)
            assignment[i] = -1;

        auto start = chrono::high_resolution_clock::now();
        bool result = DPLL(1);
        auto end = chrono::high_resolution_clock::now();

        chrono::duration<double> elapsed = end - start;
        total_time += elapsed.count();

        if (run == 0) final_result = result;
    }

    cout << "Fisier testat: " << filename << endl;
    cout << "Variabile: " << numVars << ", Clauze: " << numClauses << endl;
    cout << "Rezultat: " << (final_result ? "SAT" : "UNSAT") << endl;
    cout << fixed << setprecision(8);
    cout << "Timp mediu executie (" << NUM_RUNS << " rulari): "
         << (total_time / NUM_RUNS) << " secunde" << endl;

    return 0;
}
