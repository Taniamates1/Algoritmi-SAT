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
const int NUM_RUNS =1;

int formula[MAX_CLAUSES][MAX_LITS_PER_CLAUSE];
int clauseLens[MAX_CLAUSES];
int numVars = 0;
int numClauses = 0;

bool isEmptyClause(int idx) {
    return clauseLens[idx] == 0;
}

bool containsLiteral(int clause[], int len, int lit) {
    for (int i = 0; i < len; ++i)
        if (clause[i] == lit) return true;
    return false;
}

int addClause(int newClause[]) {
    if (numClauses >= MAX_CLAUSES) return -1;
    int len = 0;
    for (int i = 0; i < MAX_LITS_PER_CLAUSE && newClause[i] != 0; ++i)
        formula[numClauses][len++] = newClause[i];
    clauseLens[numClauses] = len;
    return numClauses++;
}

bool eliminateVariable(int var) {
    int pos[MAX_CLAUSES], neg[MAX_CLAUSES];
    int posCount = 0, negCount = 0;

    for (int i = 0; i < numClauses; ++i) {
        for (int j = 0; j < clauseLens[i]; ++j) {
            if (formula[i][j] == var) pos[posCount++] = i;
            if (formula[i][j] == -var) neg[negCount++] = i;
        }
    }

    // Generăm rezolvenți
    int startClause = numClauses;
    for (int i = 0; i < posCount; ++i) {
        for (int j = 0; j < negCount; ++j) {
            int newClause[MAX_LITS_PER_CLAUSE] = {0};
            int len = 0;

            // Adaugă literații din clauza pozitivă
            for (int a = 0; a < clauseLens[pos[i]]; ++a) {
                int lit = formula[pos[i]][a];
                if (lit != var) newClause[len++] = lit;
            }

            // Adaugă literații din clauza negativă
            for (int b = 0; b < clauseLens[neg[j]]; ++b) {
                int lit = formula[neg[j]][b];
                if (lit != -var && !containsLiteral(newClause, len, lit))
                    newClause[len++] = lit;
            }

            newClause[len] = 0;
            if (len == 0) return false; // Clauza vidă
            addClause(newClause);
        }
    }

    // Eliminăm clauze care conțin var sau -var
    int writeIndex = 0;
    for (int i = 0; i < numClauses; ++i) {
        bool skip = false;
        for (int j = 0; j < clauseLens[i]; ++j)
            if (abs(formula[i][j]) == var) {
                skip = true;
                break;
            }
        if (!skip) {
            if (writeIndex != i) {
                memcpy(formula[writeIndex], formula[i], sizeof(int) * MAX_LITS_PER_CLAUSE);
                clauseLens[writeIndex] = clauseLens[i];
            }
            ++writeIndex;
        }
    }

    numClauses = writeIndex;
    return true;
}

bool DavisPutnam() {
    for (int var = 1; var <= numVars; ++var) {
        if (!eliminateVariable(var)) return false;
    }
    return numClauses == 0;
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
            int ignored;
            iss >> tmp >> tmp >> numVars >> ignored;
            continue;
        }

        istringstream iss(line);
        int lit;
        int len = 0;
        while (iss >> lit && lit != 0) {
            if (len < MAX_LITS_PER_CLAUSE)
                formula[numClauses][len++] = lit;
        }
        clauseLens[numClauses++] = len;
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

    double total_time = 0.0;
    bool final_result = false;

    for (int run = 0; run < NUM_RUNS; ++run) {
        if (!loadDIMACS(filename.c_str())) return 1;

        auto start = chrono::high_resolution_clock::now();
        bool result = DavisPutnam();
        auto end = chrono::high_resolution_clock::now();

        chrono::duration<double> elapsed = end - start;
        total_time += elapsed.count();

        if (run == 0) final_result = result;
    }

    cout << "Fisier testat: " << filename << endl;
    cout << "Variabile: " << numVars << ", Clauze initiale: " << numClauses << endl;
    cout << "Rezultat: " << (final_result ? "SAT" : "UNSAT") << endl;
    cout << fixed << setprecision(8);
    cout << "Timp mediu executie (" << NUM_RUNS << " rulari): "
         << (total_time / NUM_RUNS) << " secunde" << endl;

    return 0;
}
