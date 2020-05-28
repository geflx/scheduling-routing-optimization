#include "ag2.h"
#include "core.h"

int main(){

    srand(time(NULL));
    unsigned seed = std::chrono::system_clock::now().time_since_epoch().count(); // Shuffle seed

    int instNumber;
    double mi, delta;

    int N, K;
    vector<double> w;
    vector<int> F, Q, s, d, P;
    vector<vector<int>> t;

    // Main variables
    ifstream input;
    string fileName;
    int gaVersion, runNb, itNumber, popSize;

    getVariables(fileName, input, gaVersion, runNb, itNumber, popSize);
   
    while(input >> instNumber){

        readInstance(input, mi, delta, N, K, P, d, s, w, Q, F, t);
        //debugRead(mi, delta, N, K, P, d, s, w, Q, F, t);

        for(int i=0; i<runNb; i++){
            
            /*
            Solution S;
            if( gaVersion == 1)
                S = GA_Version_1(N, K, itNumber, popSize, P, d, s, w, Q, F, t);
            else if( gaVersion == 2)
                S = GA_Version_2(N, K, itNumber, popSize, P, d, s, w, Q, F, t);
                
            printSolution(S, N);
            */
            Solution S;
            S = GA_Version_2(N, K, itNumber, popSize, P, d, s, w, Q, F, t);

            pair<double, double> Ans = tempObj(S, N, K, P, d, s, w, Q, F, t);
            cout << Ans.first << " " << Ans.first + Ans.second << " " ;
            if( Ans.second > 0.1){
                cout << Ans.first * Ans.second;
            }else{
                cout << -1 ;
            }
            cout << " " << Ans.second << "\n";
            
        } 
    }

    input.close();
}
