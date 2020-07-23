#include "core.h"

#include "coreLS.h"
#include "nbhoods.h"

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
    
    int a, b, c;
    a = b = c = 0;

    while(input >> instNumber){

        readInstance(input, mi, delta, N, K, P, d, s, w, Q, F, t);

        for(int i=0; i<runNb; i++){
            
            time_t iniTime, endTime;
            time(&iniTime);

            // Solution S3 = GA2_Repr1(N, K, itNumber, popSize, 0.5, P, d, s, w, Q, F, t);
            
            time(&endTime);
            double timeSpent = difftime(endTime, iniTime);

            time_t iniTime2, endTime2;
            time(&iniTime2);

            cout << "here\n";
            Solution S4 = GA2_Rep1_BL(N, K, itNumber, popSize, 0.5, P, d, s, w, Q, F, t);

            time(&endTime2);
            double timeSpent2 = difftime(endTime2, iniTime2);


            //pair<double, double> Ans3 = tempObj(S3, N, K, P, d, s, w, Q, F, t);
            pair<double, double> Ans4 = tempObj(S4, N, K, P, d, s, w, Q, F, t);
            
            double multiplier = getMultiplier(N);

            double value, overlap, value2, overlap2;
            value = (Ans4.first + multiplier * Ans4.second);
            overlap = (Ans4.second);    

            // value2 = (Ans3.first + multiplier * Ans3.second);
            // overlap2 = (Ans3.second);   

            cout << "GA2_Rep1_BL: " <<value << " " << overlap << " " << timeSpent2;
            //cout << " GA2_Repr1: " << value2 << " " << overlap2 << " " << timeSpent << "\n";           
        }

    }
    input.close();
}
