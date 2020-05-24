#include "ag2.h"
#include "core.h"

int main(){

    string fileName;
    int instNumber;
    double mi, delta;

    int N, K;
    vector<double> w;
    vector<int> F, Q, s, d, P;
    vector<vector<int>> t;

    cout << "Input instance name: ";
    getline(cin, fileName);
    cout << "string: " << fileName << "\n";
    ifstream input(fileName);

    int runNb; 
    cout << "Input running times: ";
    cin >> runNb;

    int itNumber, popSize;
    cout << "Input it. number and popSize: ";
    cin >> itNumber >> popSize;

    while(input >> instNumber){

        readInstance(input, mi, delta, N, K, P, d, s, w, Q, F, t);
        //debugRead(mi, delta, N, K, P, d, s, w, Q, F, t);

        for(int i=0; i<runNb; i++){

            Solution S = AG_Version_1(N, K, itNumber, popSize, P, d, s, w, Q, F, t);
            cout << S.Value << "\n";
            // for(int i=0; i<N; i++) cout<<"V"<<S.M[0][i]<<" "; cout << "\n";
            // for(int i=0; i<N; i++) cout<<"J"<<S.M[1][i]<<" "; cout << "\n";

        } 
    }

    input.close();
}
