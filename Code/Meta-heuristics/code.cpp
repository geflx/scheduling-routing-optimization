#include "core.h"
#include "localsearch.h"
#include "Heuristics.h"

int main(){

    srand(time(NULL));
    unsigned seed = std::chrono::system_clock::now().time_since_epoch().count(); // Shuffle seed

    int instNumber;
    double mi, delta;

    // Setup information.
    int N, K;
    vector<double> w;
    vector<int> F, Q, s, d, P;
    vector<vector<int>> t;

    // Main variables.
    ifstream input;
    string fileName;

    // Read input variables.
    int metaheuristic, parameter1, parameter2;
    getVariables(fileName, input, metaheuristic, parameter1, parameter2);
    
    while(input >> instNumber){
                
        readInstance(input, mi, delta, N, K, P, d, s, w, Q, F, t);  

        //Execute meta-heuristic and print solution.
        Execution answer = execute(N, K, w, P, t, F, d, Q, s, metaheuristic, parameter1, parameter2);
        printConfig(answer, Q, s, N, K, P, t, d, w, F, instNumber, mi, delta);
    }
    input.close();

}
