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


    // getVariables(fileName, input, gaVersion, runNb, itNumber, popSize);
    
    cout << "input filename: \n";
    getline(cin, fileName);
    input.open(fileName);

    vector<double> param_probMut  = {0.2, 0.3, 0.5};
    vector<int> param_psize = {40, 50, 70};
    vector<int> param_numIt = {500, 800, 1000};

    while(input >> instNumber){

        readInstance(input, mi, delta, N, K, P, d, s, w, Q, F, t);

        // for(int i=0; i<runNb; i++){
            
            Solution S, S2;
            // if( gaVersion == 1)  S = GA_Version_1(N, K, itNumber, popSize, P, d, s, w, Q, F, t);
            // else if( gaVersion == 2) S2 = GA_Version_2(N, K, itNumber, popSize, P, d, s, w, Q, F, t);

            vector<double> ans;
            vector<double> ans_over;
            vector<int> ans_time;

            for(int paramPSIZE : param_psize){
                for(double paramPROB : param_probMut){
                    for(int paramNUMIT : param_numIt){
                        time_t iniTime, endTime;
                        time(&iniTime);

                        Solution S3 = New_GA_Version_2(N, K, paramNUMIT, paramPSIZE, paramPROB, P, d, s, w, Q, F, t);
                        
                        time(&endTime);
                        double timeSpent = difftime(endTime, iniTime);

                        pair<double, double> Ans3 = tempObj(S3, N, K, P, d, s, w, Q, F, t);
                        
                        double multiplier = getMultiplier(N);

                        ans.push_back(Ans3.first + multiplier * Ans3.second);
                        ans_over.push_back(Ans3.second);
                        ans_time.push_back(timeSpent);
                    }
                }
            }
            int sz = ans.size();
            for(int i=0; i<sz; i++){
                cout << ans[i] << " ";
            }
            for(int i=0; i<sz; i++){
                cout << ans_over[i] << " ";
            }
            for(int i=0; i<sz; i++){
                cout << ans_time[i];
                if(i!= sz-1){
                    cout << " ";
                }else{
                    cout << "\n";
                }
            }
            
        // } 
    }

    input.close();

    
}
