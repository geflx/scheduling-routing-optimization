double global_probIG;
int global_sizePerturb;

#include "Core.h"
#include "LocalSearch.h"
#include "Metaheuristics.h"

Execution execute(int N, int K, const vector<double>& w, const vector<int>& P,
    const vector<vector<int> >& t, const vector<int>& F, const vector<int>& d,
    const vector<int>& Q, const vector<int>& s, int metaheuristic, int parameter1, int parameter2)
{
    Execution answer;

    time_t iniTime, endTime;
    time(&iniTime);

    pair<double, vector<data> > tmp;
    
    switch(metaheuristic){

    	case 1:
        	tmp = ILS_RVND_1(N, K, w, P, t, F, d, Q, s, parameter1, parameter2, 5);
			break;

    	case 2:
        	tmp = ILS_RVND_2(N, K, w, P, t, F, d, Q, s, parameter1, parameter2, 5);
 			break;
 			       	
    	case 3:
        	tmp = GA_LS(N, K, w, P, t, F, d, Q, s, parameter1);
			break;

    	case 4:
        	tmp = ILS_RVND_1_SBPO(N, K, w, P, t, F, d, Q, s, parameter1, parameter2, 5);
			break;

    	case 5:
        	tmp = ILS_RVND_2_SBPO(N, K, w, P, t, F, d, Q, s, parameter1, parameter2, 5);
			break;

    	case 6:
        	tmp = ILS_RVND_1_UPDATED(N, K, w, P, t, F, d, Q, s, parameter1, parameter2, 5);
			break;

    	case 7:
        	tmp = GA_LS_IG(N, K, w, P, t, F, d, Q, s, parameter1);
			break;

		case 8:
        	tmp = ILS_RVND_3(N, K, w, P, t, F, d, Q, s, parameter1, parameter2, global_sizePerturb);
			break;
			
    	default:
    		cout << "Invalid metaheuristic code.\n Aborting ... \n";
    		exit(0);
    }

    time(&endTime);

    answer.value = tmp.first;
    answer.vec = tmp.second;
    answer.time = difftime(endTime, iniTime);

    return answer;
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
    int instNumber;
    double mi, delta;
    ifstream input;

    // Running parameters.
    int metaheuristic, parameter1, parameter2;

   
    string fileName;
    cout << "Input instance file: ";
    cin >> fileName;

    for(int countFile = 1; countFile <= 5; countFile++){

        cout << "\nStarting file " << countFile << "/5 calibration ... \n";

        input.open(fileName);

        ofstream outputObj("calib_objf_" + to_string(countFile) + ".txt");
        ofstream outputTime("calib_time_" + to_string(countFile) + ".txt");
        ofstream outputResult("calib_result_" + to_string(countFile) + ".txt");

        while (input >> instNumber) {

            readInstance(input, mi, delta, N, K, P, d, s, w, Q, F, t); 

            vector<int> maxIter = {10};
            vector<int> maxIterILS = {100};
            vector<int> perturbSize = {5};
            vector<double> igPercentage = {0.2, 0.3, 0.4, 0.6};

            for(int _maxIter : maxIter){
                for(int _maxIterILS : maxIterILS){
                    for(int _perturbSize : perturbSize){
                        for(double _igPercentage : igPercentage){

                            global_sizePerturb = _perturbSize;
                            global_probIG = _igPercentage;
                            Execution answer = execute(N, K, w, P, t, F, d, Q, s, 8, _maxIter, _maxIterILS);


                            outputObj << answer.value << " ";
                            outputTime << answer.time << " ";
                            for(data i_job : answer.vec){
                                if(i_job.job)
                                    outputResult << "J";
                                else
                                    outputResult << "V";
                                outputResult << i_job.id << " ";
                            }
                            outputResult << "\n";
                        }
                    }
                }
            }
            outputObj << "\n";
            outputTime << "\n";
            outputResult << "\n\n";
            
        }
        
        cout << "Finishing file " << countFile <<"/5 of calibration ... \n";

        outputObj.close();
        outputTime.close();
        outputResult.close();

        input.close();
    }
   
}
