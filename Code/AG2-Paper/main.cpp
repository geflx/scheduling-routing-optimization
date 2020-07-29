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

    // Input check-up: via ARGV or manually.
    if(argc != 5){
    	getVariables(fileName, input, gaVersion, itNumber, popSize);
    }else{

    	fileName = argv[1];
    	input.open(fileName);

    	gaVersion = atoi(argv[2]);
    	itNumber = atoi(argv[3]);
    	popSize = atoi(argv[4]);
    }
    
    int a, b, c;
    a = b = c = 0;

    while(input >> instNumber){

        readInstance(input, mi, delta, N, K, P, d, s, w, Q, F, t);

            
        time_t iniTime, endTime;
        time(&iniTime);

        // Solution S3 = GA2_Repr1(N, K, itNumber, popSize, 0.5, P, d, s, w, Q, F, t);
        
        time(&endTime);
        double timeSpent = difftime(endTime, iniTime);

        time_t iniTime2, endTime2;
        time(&iniTime2);

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

        cout << "GA2_Rep1_BL: " <<value << " " << overlap << " " << timeSpent2 << "\n";
        //cout << " GA2_Repr1: " << value2 << " " << overlap2 << " " << timeSpent << "\n";           
    

    }
    input.close();
}
