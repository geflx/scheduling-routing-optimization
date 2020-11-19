#ifndef MIPSTART_H
#define MIPSTART_H


struct data {
    bool job; // True: Job, False: Vehicle.
    int id; // ID.

    data()
    {
        job = true; // Default: Is job.
        id = -1;
    }
};

struct vehicleLoaded {
    int id, initialPos, finalPos;
};

vector<vehicleLoaded> getVOrder(const vector<data>& config, int K)
{

    vector<vehicleLoaded> vehicleList;

    for (int i = 0; i < config.size(); i++) {

        if (!config[i].job) {

            vehicleLoaded temp;
            temp.id = config[i].id;

            int count = i + 1;
            temp.initialPos = count;

            while (count < config.size() && config[count].job)
                count++;

            temp.finalPos = count - 1;

            // Ignore if vehicle is empty.
            if (!(count == (i + 1)))
                vehicleList.push_back(temp);

            i = count - 1;
        }
    }

    return vehicleList;
}

vector<data> toSolution (const string &str)
{
    // String format is given as following example: "10728.6 V0 J0 J4 J5 V2 J1 J2".
	vector<data> S;

	stringstream input(str);

    int time;
    input >> time;

    double objValue;
    input >> objValue;

	char c;
	while(input >> c){

		int i;
		input >> i;

		data tmp;
		tmp.id = i;

		if(c == 'V')
			tmp.job = false;
		else
			tmp.job = true;

		S.push_back(tmp);
	}
	return S;
}

void getAijValues (int N, const vector<data> &S, vector<vector<int>> &mip_A)
{
	unordered_set<int> visited;

    int test = 0;
	for(data i: S){

		if(i.job){

			for(int j = 1; j <= N; j++)
				if((i.id + 1) != j && visited.find(j) == visited.end()){
					mip_A[i.id + 1][j] = 1;
                }

			visited.insert(i.id + 1);
		}

	}
    for(int i = 0; i <= N; i++)
        mip_A[0][i] = 1;
}

void loadMipStart (const string &str_S, int mip_N, int mip_K, vector<int> &mip_Y, vector<int> &mip_S,
					vector<int> &mip_T, vector<int> &mip_D, vector<vector<int>> &mip_A,
					vector<int> &mip_Ct, vector<vector<vector<int>>> &mip_X,
					const vector<int> &mip_P, const vector<vector<int>> &mip_t,
					const vector<int> &mip_Q, const vector<int> &mip_F, const vector<int> &mip_d, const vector<double> &mip_w)
{

	vector<data> S = toSolution(str_S);
    
	getAijValues(mip_N, S, mip_A);

    double travelC, useC, weightC, accPtime;
    travelC = useC = weightC = accPtime = 0.0;

    vector<vehicleLoaded> vehicleOrder = getVOrder(S, mip_K);

    for (int i = 0; i < vehicleOrder.size(); i++) {

        int iniJobPos = vehicleOrder[i].initialPos;
        int endJobPos = vehicleOrder[i].finalPos;

        for (int j = iniJobPos; j <= endJobPos; j++){

            accPtime += mip_P[S[j].id + 1];
        	mip_Ct[S[j].id + 1] = accPtime;
        }

        mip_S[vehicleOrder[i].id] = accPtime;

        int localTravelC = mip_t[0][S[iniJobPos].id + 1];

        mip_X[0][S[iniJobPos].id + 1][vehicleOrder[i].id] = 1;

        for (int j = iniJobPos; j < endJobPos; j++) {

            // Calculate Job Tardiness and following the Penalty weight cost.
            int T = (accPtime + localTravelC) - mip_d[S[j].id + 1];
            if (T > 0.001){

                weightC += T * mip_w[S[j].id + 1];
                mip_T[S[j].id + 1] = T;

            }else{

            	mip_T[S[j].id + 1] = 0;
            }

            mip_D[S[j].id + 1] = accPtime + localTravelC;

            localTravelC += mip_t[S[j].id + 1][S[j + 1].id + 1];
        	mip_X[S[j].id + 1][S[j + 1].id + 1][vehicleOrder[i].id] = 1;

        }

        double T = (accPtime + localTravelC) - mip_d[S[endJobPos].id + 1];

        if (T > 0.001){
            weightC += T * mip_w[S[endJobPos].id + 1];
            mip_T[S[endJobPos].id + 1] = T;
        }else{
            mip_T[S[endJobPos].id + 1] = 0;
        }

        mip_D[S[endJobPos].id + 1] = accPtime + localTravelC;

        // Last job to origin.
        localTravelC += mip_t[0][S[endJobPos].id + 1]; 
        mip_X[S[endJobPos].id + 1][0][vehicleOrder[i].id] = 1;

        useC += mip_F[vehicleOrder[i].id];
        mip_Y[vehicleOrder[i].id] = 1;

        travelC += localTravelC;

    }

    /*
    //return travelC + weightC + useC;

    ofstream here("out_test.txt");

    here << "F. Obj é: " << travelC + weightC + useC << "\n";

    here << "Aij" << "\n";
    for (int i = 0; i <= mip_N; i++) {
        for (int j = 0; j < mip_N + 1; j++) {
            if(i == j)
                cout << -1 << " ";
            else
                here << mip_A[i][j] << " ";
        }
        here << endl;
    }
    here << endl;

    here << "Yi " << "\n";
    for (int i = 0; i < mip_K; i++) {
        here << mip_Y[i] << " ";
    }
    here << endl << endl;

    here << "Si " << "\n";
    for (int i = 0; i < mip_K; i++) {
        here << mip_S[i] << " ";
    }
    here << endl << endl;

    here << "Di " << "\n";
    for (int i = 0; i < mip_N + 1; i++) {
        here << mip_D[i] << " ";
    }
    here << endl << endl;

    here << "Ti " << "\n";
    for (int i = 0; i < mip_N + 1; i++) {
        here << mip_T[i] << " ";
    }
    here << endl << endl;

    here << "Ct " << "\n";
    for (int i = 0; i < mip_N + 1; i++) {
        here << mip_Ct[i] << " ";
    }
    here << endl << endl;


    for (int k = 0; k < mip_K; k++) {
        here << "K is: " << k << "\n";
        for (int i = 0; i <= mip_N; i++) {

            for (int j = 0; j <= mip_N; j++) {

                here << mip_X[i][j][k] << " ";
                
            }
            here << "\n";
        }
        here << "\n";
    }
    here.close();
    */
}

#endif
