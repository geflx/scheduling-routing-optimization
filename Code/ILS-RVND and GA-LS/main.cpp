int ig_used = 0;

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
    string fileName;

    // Running parameters.
    int metaheuristic, parameter1, parameter2;

    if(argc != 5 && argc != 4){
    	getVariables(fileName, input, metaheuristic, parameter1, parameter2);
    }else{
    	fileName = argv[1];
    	input.open(fileName);

    	metaheuristic = atoi(argv[2]);
    	parameter1 = atoi(argv[3]);

    	if(argc == 5) 
    		parameter2 = atoi(argv[4]);
    	else 
    		parameter2 = -1;
    }

    // MIPStart Output.
    char strOf[50];
    sprintf(strOf, "MIPStart_Output.txt");
    ofstream mipOut(strOf);

    // Mheuristic Output.
    char strOut[50];
    sprintf(strOut, "ANS_Output.txt");
    ofstream outFile(strOut);

    int instance = 0;
    while (input >> instNumber) {

        readInstance(input, mi, delta, N, K, P, d, s, w, Q, F, t); 
        instance++;

        //Test(N, K, w, P, t, F, d, Q, s);
        ig_used = 0;
        //Execute meta-heuristic and print Mheuristic solution.
        Execution answer = execute(N, K, w, P, t, F, d, Q, s, metaheuristic, parameter1, parameter2);
        printConfig(outFile, answer, answer.time, Q, s, N, K, P, t, d, w, F, instNumber, mi, delta);
        cout << "IG_Used: " << ig_used << "\n";
        // Printing on screen the completion percentage when Cont%10 == 1.
        if(instance % 50 == 1){

            double totalInstances = -1;
            if(N > 20) 
                totalInstances = 180;
            else
                totalInstances = 300;

            printf("%.1lf percent complete. \n",  (instance * 100 / totalInstances));
        }

        // MIPStart Output.
        mipOut << setw(4) << answer.time << setw(15) << answer.value << setw(5);

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
    rename("MIPStart_Output.txt", strOf);
    mipOut.close();

    sprintf(strOut, "ANS_%djobs.txt", N);
    rename("ANS_Output.txt", strOut);
    outFile.close();

    input.close();
}
