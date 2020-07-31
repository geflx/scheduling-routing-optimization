#include "core.h"
#include "ReuseRep.h"
#include "ag2.h"

int main(int argc, char* argv[]){

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
    int gaVersion, itNumber, popSize;

    getVariables(argc, argv, fileName, input, gaVersion, itNumber, popSize);
    
    int a, b, c;
    a = b = c = 0;

    // Mheuristic Output.
    char strOut[50];
    sprintf(strOut, "ANS_Output.txt");
    ofstream outFile(strOut);

    int instance = 0;

    while(input >> instNumber){

        readInstance(input, mi, delta, N, K, P, d, s, w, Q, F, t);

        ++instance;

        Solution answer;

        // Run meta-heuristic and computates time spent.
        time_t iniTime, endTime;
        time(&iniTime);

        if(gaVersion == 1) 
            answer = GA2_Repr1(N, K, itNumber, popSize, 0.5, P, d, s, w, Q, F, t);
        else if(gaVersion == 2)
            answer = GA2_Rep1_BL(N, K, itNumber, popSize, 0.5, P, d, s, w, Q, F, t);
 
        time(&endTime);
        double timeSpent = difftime(endTime, iniTime);


        // Calculating Objective Function.
        pair<double, double> answerOverlap = calculateObjWithOverlap(answer, N, K, P, d, s, w, Q, F, t);
        
        double multiplier, overlap, value;
        multiplier = getMultiplier(N);

        value = answerOverlap.first + multiplier * answerOverlap.second;
        overlap = answerOverlap.second;    

        // Printing on screen the completion percentage when Cont%10 == 1.
        if(instance % 20 == 1){

            double totalInstances = -1;
            if(N > 20) 
                totalInstances = 180;
            else
                totalInstances = 300;

            printf("%.1lf percent complete. \n",  (instance * 100 / totalInstances));
        }

        outFile << setw(3) << timeSpent << setw(15) << value << setw(3) << overlap << "\n";       
             
    }

    sprintf(strOut, "ANS_%djobs.txt", N);
    rename("ANS_Output.txt", strOut);
    outFile.close();

    input.close(); 
}
