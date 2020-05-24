#include "ag2.h"
#include "core.h"

int main(){

    string fileName;
    int instNumber;
    double mi, delta;

    int N, K;
    vector<double> w(N);
    vector<int> F(K), Q(K);
    vector<int> s(N), d(N), P(N);
    vector<vector<int>> t (N+1, vector<int> (N+1, 0));

    getline(cin, fileName);
    ifstream input(fileName);

    while(input >> instNumber){

        readInstance(input, mi, delta, N, K, P, d, s, w, Q, F, t);

        int runNb; 
        cout << "Input running times: ";
        cin >> runNb;

        int itNumber, popSize;
        cout << "Input it. number and popSize: ";
        cin >> itNumber >> popSize;

        for(int i=0; i<runNb; i++){

            //Solution Ans = ag2(N, K, itNumber, popSize, P, d, s, w, Q, F, t);
            //cout << Ans.Value << "\n";

        }
    }

}
