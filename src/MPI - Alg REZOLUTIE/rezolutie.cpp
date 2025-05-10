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
const int MAX_LITS_PER_CLAUSE = 10;
const int NUM_RUNS =1;

int formula[MAX_CLAUSES][MAX_LITS_PER_CLAUSE];
int clauseLens[MAX_CLAUSES];
int numClauses = 0;
int numVars = 0;

bool areComplementary(int a, int b) {
    return a == -b;
}

bool contains(int clause[], int len, int lit) {
    for (int i = 0; i < len; ++i)
        if (clause[i] == lit) return true;
    return false;
}

bool addClause(int newClause[], int len) {
    // Verificăm dacă clauza există deja
    for (int i = 0; i < numClauses; ++i) {
        if (clauseLens[i] != len) continue;
        bool identical = true;
        for (int j = 0; j < len; ++j)
            if (!contains(formula[i], clauseLens[i], newClause[j])) {
                identical = false;
                break;
            }
        if (identical) return false; // deja există
    }

    if (len == 0) return true; // clauză vidă

    // Adăugăm clauza nouă
    for (int i = 0; i < len; ++i)
        formula[numClauses][i] = newClause[i];
    clauseLens[numClauses++] = len;
    return false;
}

bool applyResolution() {
    bool changed = true;

    while (changed) {
        changed = false;
        int start = numClauses;

        for (int i = 0; i < start; ++i) {
            for (int j = i + 1; j < start; ++j) {
                for (int a = 0; a < clauseLens[i]; ++a) {
                    for (int b = 0; b < clauseLens[j]; ++b) {
                        if (!areComplementary(formula[i][a], formula[j][b])) continue;

                        int newClause[MAX_LITS_PER_CLAUSE] = {0};
                        int len = 0;

                        for (int x = 0; x < clauseLens[i]; ++x)
                            if (x != a && !contains(newClause, len, formula[i][x]))
                                newClause[len++] = formula[i][x];
                        for (int y = 0; y < clauseLens[j]; ++y)
                            if (y != b && !contains(newClause, len, formula[j][y]))
                                newClause[len++] = formula[j][y];

                        bool isEmpty = (len == 0);
                        bool alreadyExists = !addClause(newClause, len);

                        if (isEmpty) return false; // clauză vidă -> UNSAT
                        if (!alreadyExists) changed = true;
                    }
                }
            }
        }
    }

    return true; // nu s-a găsit contradicție => formula e SAT
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
            int dummy;
            iss >> tmp >> tmp >> numVars >> dummy;
            continue;
        }

        istringstream iss(line);
        int lit;
        int len = 0;
        while (iss >> lit && lit != 0 && len < MAX_LITS_PER_CLAUSE)
            formula[numClauses][len++] = lit;
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
    bool final_result = true;

    for (int run = 0; run < NUM_RUNS; ++run) {
        if (!loadDIMACS(filename.c_str())) return 1;

        auto start = chrono::high_resolution_clock::now();
        bool result = applyResolution();
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
