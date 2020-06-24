#include "core.h"
#include "ag2.h"

#define RUN_X_TIMES 5

int main(){
    // Main variables
    ifstream input;
    string fileName;
    int jobsNumber = 1;

    cout << "1) Insert file name: ";
    getline(cin, fileName);

    cout << "2) Insert jobs number: ";
    cin >> jobsNumber;
    
    cout<<"\n\n";

    // Shuffle seed
    srand(time(NULL));
    unsigned seed = std::chrono::system_clock::now().time_since_epoch().count(); 
    
    for(int run=1; run<=RUN_X_TIMES; run++){

        cout <<"Running " << run <<"/5 times. \n\n";
        string outFile = "calib_" + to_string(run) + "_" + to_string(jobsNumber) + "jobs.txt";
        
        //Ifstream and ofstream
        ofstream out(outFile); 
        input.open(fileName);

        int instNumber;
        double mi, delta;

        // Setup information
        int N, K;
        vector<double> w;
        vector<int> F, Q, s, d, P;
        vector<vector<int>> t;

        int gaVersion, runNb, itNumber, popSize;

        // Combinations
        vector<double> param_probMut  = {0.2, 0.3, 0.5};
        vector<int> param_psize = {40, 50, 70};
        vector<int> param_numIt = {500, 800, 1000};

        //Print combinations
        for(int paramPSIZE : param_psize){
            for(double paramPROB : param_probMut){
                for(int paramNUMIT : param_numIt){
                    out << paramPSIZE << "_" << paramNUMIT << "_" << paramPROB << " ";
                }
            }
        }
        //Print Overlap and Time Columns (header)
        for(int i=1; i<= param_probMut.size() * param_psize.size() * param_numIt.size(); i++)
            out << "OVER_" << i << " ";
        for(int i=1; i<= param_probMut.size() * param_psize.size() * param_numIt.size(); i++)
            out << "TIME_" << i << " ";
        
        out << endl;

        while(input >> instNumber){

            readInstance(input, mi, delta, N, K, P, d, s, w, Q, F, t);

            // Answer arrays (obj, overlap, time)
            vector<double> ans;
            vector<double> ans_over;
            vector<int> ans_time;

            for(int paramPSIZE : param_psize){
                for(double paramPROB : param_probMut){
                    for(int paramNUMIT : param_numIt){

                        // Initialize time
                        time_t iniTime, endTime;
                        time(&iniTime);

                        // Call heuristic
                        Solution S3 = New_GA_Version_2(N, K, paramNUMIT, paramPSIZE, paramPROB, P, d, s, w, Q, F, t);
                        
                        //Calculate time spent
                        time(&endTime);
                        double timeSpent = difftime(endTime, iniTime);

                        pair<double, double> Ans3 = tempObj(S3, N, K, P, d, s, w, Q, F, t);
                        
                        // Get constant number (penalty calculation)
                        double multiplier = getMultiplier(N);

                        //Add solution to "answer" arrays
                        ans.push_back(Ans3.first + multiplier * Ans3.second);
                        ans_over.push_back(Ans3.second);
                        ans_time.push_back(timeSpent);
                    }
                }
            }
            //Print solutions
            int sz = ans.size();
            for(int i=0; i<sz; i++)
                out << ans[i] << " ";
            
            for(int i=0; i<sz; i++)
                out << ans_over[i] << " ";

            for(int i=0; i<sz; i++){
                out << ans_time[i];
                if(i!= sz-1)
                    out << " ";
                else
                    out << "\n";
            }
        }
        out.close();
        input.close();
    }
}