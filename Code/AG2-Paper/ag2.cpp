#include "core.h"
#include "ag2.h"

int main(){

    srand(time(NULL));
    unsigned seed = std::chrono::system_clock::now().time_since_epoch().count(); // Shuffle seed

    int instNumber;
    double mi, delta;

    // Setup information
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

        for(int i=0; i<runNb; i++){
            
            Solution S, S2;
            // if( gaVersion == 1)  S = GA_Version_1(N, K, itNumber, popSize, P, d, s, w, Q, F, t);
            // else if( gaVersion == 2) S2 = GA_Version_2(N, K, itNumber, popSize, P, d, s, w, Q, F, t);

          
    
            // time_t iniTime, endTime;
            // time(&iniTime);

            db_fea_after_inf = db_gen = db_inf_fixed = db_inf_random = 0;
            Solution S3 = New_GA_Version_2(N, K, itNumber, popSize, 0.5, P, d, s, w, Q, F, t);
            
            // time(&endTime);
            // double timeSpent = difftime(endTime, iniTime);

            pair<double, double> Ans3 = tempObj(S3, N, K, P, d, s, w, Q, F, t);
            
            double multiplier = getMultiplier(N);

        
            cout << Ans3.first + multiplier * Ans3.second << " " << Ans3.second << " " << db_gen << " " << db_inf_fixed << " " << db_inf_random << " " << db_fea_after_inf << " ";
        
            cout << "\n";
        } 
    }

    input.close();

    
}
