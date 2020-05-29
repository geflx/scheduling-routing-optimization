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
            
            time_t iniTime, endTime;
            time(&iniTime);

            Solution S;
            if( gaVersion == 1)
                S = GA_Version_1(N, K, itNumber, popSize, P, d, s, w, Q, F, t);
            else if( gaVersion == 2)
                S = GA_Version_2(N, K, itNumber, popSize, P, d, s, w, Q, F, t);
                
            //printSolution(S, N);
            


            // Solution S = GA_Version_2(N, K, itNumber, popSize, P, d, s, w, Q, F, t);

            time(&endTime);
            double timeSpent = difftime(endTime, iniTime);

            pair<double, double> Ans = tempObj(S, N, K, P, d, s, w, Q, F, t);
            
            double multiplier = 0;

            if(N == 8 || N == 10)
                multiplier = 100;
            else if( N ==15 || N == 20)
                multiplier = 500;
            else if( N >= 50 && N <= 100)
                multiplier = 1000;
            
            cout << Ans.first + multiplier * Ans.second << " " << timeSpent << "\n";
            
        } 
    }

    input.close();
}
