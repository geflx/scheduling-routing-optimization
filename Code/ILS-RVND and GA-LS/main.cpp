#include "Core.h"
#include "LocalSearch.h"
#include "Metaheuristics.h"

void execute(int N, int K, vector<double>& w, vector<int>& P,
    vector<vector<int> >& t, vector<int>& F, vector<int>& d,
    vector<int>& Q, vector<int>& s, string &fileName, ifstream &input, 
          int &metaheuristic, int &param1, int &param2, double &param3)
{
    // Read file directory, parameters, etc.
    getVariables(1, fileName, input, metaheuristic, param1, param2, param3);

    // Output file containing Time and Obj Function value (MIP Start model format).
    char strOf[50];
    sprintf(strOf, "MIP_Out.txt");
    ofstream mipOut(strOf);

    // Instance extra information and counter.
    double mi, delta;
    int countInst = 1, instCode;

    while (input >> instCode) {

        readInstance(input, mi, delta, N, K, P, d, s, w, Q, F, t); 

        Execution answer;

        time_t iniTime, endTime;
        time(&iniTime);

        pair<double, vector<data> > tmp;
        
        switch(metaheuristic){
            case 1:
                tmp = ILS_RVND_1(N, K, w, P, t, F, d, Q, s, param1, param2, 5);
                break;

            case 2:
                tmp = ILS_RVND_2(N, K, w, P, t, F, d, Q, s, param1, param2, 5);
                break;

            case 3:
                tmp = GA_LS(N, K, w, P, t, F, d, Q, s, param1);
                break;

            case 4: 
                tmp = ILS_RVND_IG(N, K, w, P, t, F, d, Q, s, param1, param2, 5, param3);
                break;
                        
            case 5:
                tmp = GA_LS_IG(N, K, w, P, t, F, d, Q, s, param1, param3);
                break;
                
            default:
                cout << "Invalid metaheuristic code.\n Aborting ... \n";
                exit(0);
        }

        time(&endTime);

        answer.value = tmp.first;
        answer.vec = tmp.second;
        answer.time = difftime(endTime, iniTime);

        // MIPStart Output.
        mipOut << setw(4) << answer.time << setw(15) << answer.value << setw(5);
        cout << "{\"Instance\": \"" << countInst++ << "\", \"Time\": \"" << answer.time  << "\", \"Obj_F\": \"" << answer.value << "\"},\n";

        for(data i : answer.vec){
            if(i.job)
                mipOut << "J";
            else
                mipOut << "V";
            mipOut << i.id << " ";
        }
        mipOut << "\n";
        
    }

    sprintf(strOf, "MIP_%djobs.txt", N);
    rename("MIP_Out.txt", strOf);
    mipOut.close();

    input.close();  
}

void Menu(int N, int K, vector<double>& w, vector<int>& P, vector<vector<int> >& t, vector<int>& F, 
          vector<int>& d, vector<int>& Q, vector<int>& s, string &fileName, ifstream &input, 
          int &metaheuristic, int &param1, int &param2, double &param3){

    cout << "Options:\n";
    cout << "1 - Execute\n";
    cout << "2 - Calibrate\n\n";
    cout << "Input here: ";

    int option;
    cin >> option;

    if(option == 1){

        execute(N, K, w, P, t, F, d, Q, s, fileName, input, metaheuristic, param1, param2, param3);

    }else if(option == 2){

        // calibrate(N, K, w, P, t, F, d, Q, s, fileName, input, metaheuristic, param1, param2, param3);

    }else{
        cout << "Undefined option.\n";
        exit(0);
    }
}

int main(int argc, char* argv[])
{
    srand(time(NULL));
    unsigned seed = std::chrono::system_clock::now().time_since_epoch().count(); // Shuffle seed

    // Setup information.
    int N, K;
    vector<double> w;
    vector<int> F, Q, s, d, P;
    vector<vector<int> > t;

    // Instance information.
    ifstream input;
    string fileName;

    // Running parameters.
    int metaheuristic, param1, param2;
    double param3;

    // List program options
    Menu (N, K, w, P, t, F, d, Q, s, fileName, input, metaheuristic, param1, param2, param3);
}

