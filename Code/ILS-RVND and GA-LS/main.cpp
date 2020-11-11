#include "Core.h"
#include "LocalSearch.h"
#include "Metaheuristics.h"

void execute(int N, int K, vector<double>& w, vector<int>& P,
    vector<vector<int> >& t, vector<int>& F, vector<int>& d,
    vector<int>& Q, vector<int>& s, string &fileName, 
          int &metaheuristic, int &param1, int &param2, double &param3)
{
    // Read file directory, parameters, etc.
    getVariables(1, fileName, metaheuristic, param1, param2, param3);
    ifstream input(fileName);

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
        int countSolutions = 0;

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
                tmp = GA_LS_IG(N, K, w, P, t, F, d, Q, s, param1, param3, countSolutions);
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

// REVISAR, ESTÁ SEM REVISAO!!

void calibrate(int N, int K, vector<double>& w, vector<int>& P,
    vector<vector<int> >& t, vector<int>& F, vector<int>& d,
    vector<int>& Q, vector<int>& s, string &fileName, 
          int &metaheuristic, int &param1, int &param2, double &param3)
{
    // Read file directory, parameters, etc.
    getVariables(2, fileName, metaheuristic, param1, param2, param3);

    if(metaheuristic == 1){

        // Calibrate ILS. Reading parameters to calibrate.

        vector<int> _pert;
        vector<double> _percentIG;

        cout << "\nInput \"pert\" values to calibrate(-1 ends input): ";
        int tmpPert;
        while(cin >> tmpPert && tmpPert != -1)
            _pert.push_back(tmpPert);

        cout << "\nInput IG percentage values to calibrate(e.g. 0.3, note -1 ends input): ";
        double tmpPercentIG;
        while(cin >> tmpPercentIG && tmpPercentIG >= 0.0f)
            _percentIG.push_back(tmpPercentIG);

        cout << "\nInput how many times to execute (e.g. 5): ";
        int contExec; 
        cin >> contExec;

        // Instance extra information and counter.
        double mi, delta;
        int countInst = 1, instCode;

        cout << "\n";
   
        for(int i = 0; i < contExec; i++){

            cout << "Calibrating, step " << (i+1) << "/" << contExec << "...\n";

            // Prepairing output files to each calibration execution.

            ofstream timeOutput("C" + to_string(i+1) + "_ILS_" + "Time.csv");
            ofstream objOutput("C" + to_string(i+1) + "_ILS_" + "Obj.csv");
            ofstream descOutput("C" + to_string(i+1) + "_ILS_" + "Description.csv");
            ofstream itOutput("C" + to_string(i+1) + "_ILS_" + "Iterations.csv");

            // Print combinations and format the output files as .csv files.
            for(int a = 0; a < _pert.size(); a++){
                for(int b = 0; b < _percentIG.size(); b++){
                    timeOutput << _pert[a] << "-" << _percentIG[b];
                    objOutput << _pert[a] << "-" << _percentIG[b];
                    descOutput << _pert[a] << "-" << _percentIG[b];
                    itOutput << _pert[a] << "-" << _percentIG[b];

                    if(a == (_pert.size() - 1) && b == (_percentIG.size() -1)){
                        timeOutput << "\n";
                        objOutput << "\n";
                        descOutput << "\n";
                        itOutput << "\n";
                    }else{
                        timeOutput << ",";
                        objOutput << ",";
                        descOutput << ",";
                        itOutput << ",";
                    }
                }
            }

            // Reading instances again at each step.
            ifstream input(fileName);

            while (input >> instCode) {
                readInstance(input, mi, delta, N, K, P, d, s, w, Q, F, t); 

                for(int pertParam: _pert){
                    for(double percentIGParam: _percentIG){

                        time_t iniTime, endTime; 
                        time(&iniTime);

                        int contRestart = 0, contIt = 0;
                        pair<double, vector<data>> tmp = ILS_RVND_IG_Time_Based(N, K, w, P, t, F, d, Q, s, pertParam, percentIGParam, contRestart, contIt);
                        time(&endTime);

                        // Prints time, objF and iterations in output files.

                        timeOutput << difftime(endTime, iniTime);
                        objOutput << tmp.first;
                        itOutput << contRestart << "-" << contIt;

                        // Print job and delivery sequence.

                        int countItems = 0;
                        for(data itJob : tmp.second){
                            if(itJob.job)
                                descOutput << "J";
                            else
                                descOutput << "V";
                            descOutput << itJob.id;
                            if(countItems != (N + K - 1))
                                descOutput << "-";
                            countItems++;
                        }

                        // Print common to separate content
                        if(!(pertParam == _pert.back() && percentIGParam == _percentIG.back())){
                            timeOutput << ",";
                            objOutput << ",";
                            descOutput << ",";
                            itOutput << ",";
                        }
                    }
                }
                timeOutput << "\n";
                objOutput << "\n";
                descOutput << "\n";
                itOutput << "\n";
                
            }

            input.close();
            timeOutput.close();
            objOutput.close();
            descOutput.close();
            itOutput.close();
        }
    }else if(metaheuristic == 2){

        // Calibrate Genetic Algorithm.

        vector<int> _psize;
        vector<double> _percentIG;

        cout << "\nInput \"psize\" values to calibrate(-1 ends input): ";
        int tmpPsize;
        while(cin >> tmpPsize && tmpPsize != -1)
            _psize.push_back(tmpPsize);

        cout << "\nInput IG percentage values to calibrate(e.g. 0.3, note -1 ends input): ";
        double tmpPercentIG;
        while(cin >> tmpPercentIG && tmpPercentIG >= 0.0f)
            _percentIG.push_back(tmpPercentIG);

        cout << "\nInput how many times to execute (e.g. 5): ";
        int contExec; 
        cin >> contExec;

        // Instance extra information and counter.
        double mi, delta;
        int countInst = 1, instCode;

        cout << "\n";
   
        for(int i = 0; i < contExec; i++){

            cout << "Calibrating, step " << (i+1) << "/" << contExec << "...\n";

            // Prepairing output files to each calibration execution.

            ofstream timeOutput("C" + to_string(i+1) + "_GA_" + "Time.csv");
            ofstream objOutput("C" + to_string(i+1) + "_GA_" + "Obj.csv");
            ofstream descOutput("C" + to_string(i+1) + "_GA_" + "Description.csv");
            ofstream itOutput("C" + to_string(i+1) + "_GA_" + "CountSolutions.csv");

            // Print combinations and format the output files as .csv files.
            for(int a = 0; a < _psize.size(); a++){
                for(int b = 0; b < _percentIG.size(); b++){
                    timeOutput << _psize[a] << "-" << _percentIG[b];
                    objOutput << _psize[a] << "-" << _percentIG[b];
                    descOutput << _psize[a] << "-" << _percentIG[b];
                    itOutput << _psize[a] << "-" << _percentIG[b];

                    if(a == (_psize.size() - 1) && b == (_percentIG.size() -1)){
                        timeOutput << "\n";
                        objOutput << "\n";
                        descOutput << "\n";
                        itOutput << "\n";
                    }else{
                        timeOutput << ",";
                        objOutput << ",";
                        descOutput << ",";
                        itOutput << ",";
                    }
                }
            }

            // Reading instances again at each step.
            ifstream input(fileName);

            while (input >> instCode) {
                readInstance(input, mi, delta, N, K, P, d, s, w, Q, F, t); 

                for(int psizeParam: _psize){
                    for(double percentIGParam: _percentIG){

                        time_t iniTime, endTime; 
                        time(&iniTime);

                        int countSolutions = 0;
                        pair<double, vector<data>> tmp = GA_LS_IG(N, K, w, P, t, F, d, Q, s, psizeParam, percentIGParam, countSolutions);
                        time(&endTime);

                        // Prints time, objF and iterations in output files.

                        timeOutput << difftime(endTime, iniTime);
                        objOutput << tmp.first;
                        itOutput << countSolutions;

                        // Print job and delivery sequence.

                        int countItems = 0;
                        for(data itJob : tmp.second){
                            if(itJob.job)
                                descOutput << "J";
                            else
                                descOutput << "V";
                            descOutput << itJob.id;
                            if(countItems != (N + K - 1))
                                descOutput << "-";
                            countItems++;
                        }

                        // Print common to separate content
                        if(!(psizeParam == _psize.back() && percentIGParam == _percentIG.back())){
                            timeOutput << ",";
                            objOutput << ",";
                            descOutput << ",";
                            itOutput << ",";
                        }
                    }
                }
                timeOutput << "\n";
                objOutput << "\n";
                descOutput << "\n";
                itOutput << "\n";
                
            }

            input.close();
            timeOutput.close();
            objOutput.close();
            descOutput.close();
            itOutput.close();
        }
    }
}

void Menu(int N, int K, vector<double>& w, vector<int>& P, vector<vector<int> >& t, vector<int>& F, 
          vector<int>& d, vector<int>& Q, vector<int>& s, string &fileName, 
          int &metaheuristic, int &param1, int &param2, double &param3){

    cout << "Options:\n";
    cout << "1 - Execute\n";
    cout << "2 - Calibrate\n\n";
    cout << "Input option: ";

    int option;
    cin >> option;

    if(option == 1){

        execute(N, K, w, P, t, F, d, Q, s, fileName, metaheuristic, param1, param2, param3);

    }else if(option == 2){

        calibrate(N, K, w, P, t, F, d, Q, s, fileName, metaheuristic, param1, param2, param3);

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
    string fileName;

    // Running parameters.
    int metaheuristic, param1, param2;
    double param3;

    // List program options
    Menu (N, K, w, P, t, F, d, Q, s, fileName, metaheuristic, param1, param2, param3);
}

