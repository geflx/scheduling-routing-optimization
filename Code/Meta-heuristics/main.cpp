#include "Core.h"
#include "LocalSearch.h"
#include "Metaheuristics.h"

Execution execute(int N, int K, const vector<double>& w, const vector<int>& P,
    const vector<vector<int> >& t, const vector<int>& F, const vector<int>& d,
    const vector<int>& Q, const vector<int>& s, int metaheuristic, int parameter1, int parameter2)
{
    Execution answer;

    time_t iniTime, endTime;
    time(&iniTime);

    pair<double, vector<data> > auxiliar;

    if (metaheuristic == 1)
        auxiliar = ils_rvnd(N, K, w, P, t, F, d, Q, s, parameter1, parameter2, 5);
    else if (metaheuristic == 2)
        auxiliar = ils_rvnd_custom(N, K, w, P, t, F, d, Q, s, parameter1, parameter2, 5);
    else
        auxiliar = genAlgo1(N, K, w, P, t, F, d, Q, s, parameter1);

    time(&endTime);

    answer.value = auxiliar.first;
    answer.vec = auxiliar.second;
    answer.time = difftime(endTime, iniTime);

    return answer;
}

int main()
{

    srand(time(NULL));
    unsigned seed = std::chrono::system_clock::now().time_since_epoch().count(); // Shuffle seed

    // Setup information.
    int N, K;
    vector<double> w;
    vector<int> F, Q, s, d, P;
    vector<vector<int> > t;

    // Instance information.
    int instNumber;
    double mi, delta;
    ifstream input;
    string fileName;

    // Running parameters.
    int metaheuristic, parameter1, parameter2;
    getVariables(fileName, input, metaheuristic, parameter1, parameter2);

    // MIPStart Output.
    char strOf[50];
    sprintf(strOf, "MIPStart_Output.txt");
    ofstream mipOut(strOf);

    while (input >> instNumber) {

        readInstance(input, mi, delta, N, K, P, d, s, w, Q, F, t);

        //Execute meta-heuristic and print solution.
        Execution answer = execute(N, K, w, P, t, F, d, Q, s, metaheuristic, parameter1, parameter2);
        printConfig(answer, answer.time, Q, s, N, K, P, t, d, w, F, instNumber, mi, delta);

        // MIPStart Output
        mipOut << setw(15) << answer.value << setw(5);
        for(data i : answer.vec){
            if(i.job)
                mipOut << "J";
            else
                mipOut << "V";
            mipOut << i.id << " ";
        }
        mipOut << "\n";
    }

    sprintf(strOf, "MIPStart_%djobs.txt", N);
    rename("MIPStart_Output.txt", strOf);
    mipOut.close();

    input.close();
}
