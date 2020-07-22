#include <ilcplex/ilocplex.h>
#include <iostream>
#include <fstream>
#include <algorithm>
#include <vector>
#include <unordered_set>
#include <sstream>
#include <string>

using namespace std;

#include "MipStart.h"

typedef IloArray<IloIntArray> IntMatriz;
typedef IloArray<IloNumArray> FloatMatriz;
typedef IloArray<IloArray<IloNumVarArray> > Matriz3D;
typedef IloArray<IloNumVarArray> NumVarMatrix;
typedef IloArray<NumVarMatrix> NumVar3Matrix;

int main()
{

	// MIPStart Checkup.

	cout << "Use MipStart in Executions (Y/N)? ";
	char charMip;
	cin >> charMip;

	ifstream mipIn;

	bool useMip = false;
	if(charMip == 'Y'){
		useMip = true;

		string strMip;
		cout << "Insert MipFile: ";
		cin >> strMip;

		mipIn.open(strMip);
	}

	// Input and Output files declaration.

    float VarValue;
    char name[50];

    string fileName;
    cout << "Insert Instance File: ";
    cin >> fileName;

    ifstream in(fileName);

    ofstream myfile("CPLEX_Output.txt");

    float mi, delta;
    int instance_number;

    myfile << setw(15) << "Instance";
    myfile << setw(12) << "Obj_Value";
    myfile << setw(10) << "Obj_1";
    myfile << setw(10) << "Obj_2";
    myfile << setw(11) << "OBj_3";
    myfile << setw(14) << "RelativeGap%";
    myfile << setw(12) << "Status";
    myfile << setw(12) << "CPUtime" << "\t";
    myfile << "Processing_Order" << "    ";
    myfile << "Vehicle_Routes" << endl;

    while (in >> instance_number) {

    	IloEnv cplex_env;
        IloModel modeloP(cplex_env);
        IloExpr funcaoO(cplex_env);
        IloRangeArray Constraints(cplex_env);

        IloInt N, K;

        in >> mi >> delta;
        in >> N >> K;

        // Constant variables declaration.

        IloNumArray F(cplex_env, K);
        IloNumArray Q(cplex_env, K);

        FloatMatriz C(cplex_env, N + 1);
        IntMatriz t(cplex_env, N + 1);
        IloNumArray P(cplex_env, N + 1);
        IloNumArray w(cplex_env, N + 1);
        IloNumArray d(cplex_env, N + 1);
        IloNumArray s(cplex_env, N + 1);

        //MipStart constant variables declaration.

        int mip_N, mip_K;

        mip_N = N;
        mip_K = K;

        vector<int> mip_F(mip_K, 0);
        vector<int> mip_Q(mip_K, 0);

        vector<vector<int>> mip_t(mip_N+1, vector<int> (mip_N+1, 0));

        vector<int> mip_P(mip_N+1, 0), mip_d(mip_N+1, 0),
                    mip_s(mip_N+1, 0);

        vector<double> mip_w(mip_N+1, 0.0);

        // MipStart decision variables declaration.

        vector<int> mip_Y(mip_K, 0), mip_S(mip_K, 0), mip_Ct(mip_N+1, 0),
                    mip_D(mip_N+1, 0), mip_T(mip_N+1, 0);
        vector<vector<int>> mip_A (mip_N+1, vector<int> (mip_N+1, 0));
        vector<vector<vector<int>>> mip_X (mip_N+1, vector<vector<int>> (mip_N+1, vector<int> (mip_K, 0)));

        // Initializing Origin(job_id: 0) variables.

        P[0] = d[0] = w[0] = s[0] = 0;
        mip_P[0] = mip_d[0] = mip_w[0] = mip_s[0] = 0;

        for (int i = 1; i <= N; i++){
            in >> P[i];
            mip_P[i] = P[i];
        }

        for (int i = 1; i <= N; i++){
            in >> d[i];
            mip_d[i] = d[i];
        }

        for (int i = 1; i <= N; i++){
            in >> s[i];
            mip_s[i] = s[i];
        }

        for (int i = 1; i <= N; i++){
            in >> w[i];
             mip_w[i] = w[i];
        }

        for (int i = 0; i < K; i++){
            in >> Q[i];
            mip_Q[i] = Q[i];
        }

        for (int i = 0; i < K; i++){
            in >> F[i];
            mip_F[i] = F[i];
        }


        for (int i = 0; i <= N; i++)
            t[i] = IloIntArray(cplex_env, N + 1);

        for (int i = 0; i <= N; i++){
            for (int j = 0; j <= N; j++){
                in >> t[i][j];
                mip_t[i][j] = t[i][j];
            }
        }

      // Initializing decision variables (array, 2D matrix and 3D matrix).

        NumVarMatrix A(cplex_env, N + 1);
        for (int i = 0; i <= N; i++) {
            A[i] = IloNumVarArray(cplex_env, N + 1, 0, 1, ILOINT);

            for (int j = 0; j < N + 1; j++) {
                sprintf(name, "A[%d][%d]", i, j);
                A[i][j].setName(name);
            }
        }

        IloNumVarArray Y(cplex_env, K, 0, 1, ILOINT);
        for (int i = 0; i < K; i++) {
            sprintf(name, "Y[%d]", i);
            Y[i].setName(name);
        }

        IloNumVarArray S(cplex_env, K, 0, IloInfinity, ILOINT);
        for (int i = 0; i < K; i++) {
            sprintf(name, "S[%d]", i);
            S[i].setName(name);
        }

        IloNumVarArray T(cplex_env, N + 1, 0, IloInfinity, ILOINT);
        for (int i = 0; i < N + 1; i++) {
            sprintf(name, "T[%d]", i);
            T[i].setName(name);
        }

        IloNumVarArray D(cplex_env, N + 1, 0, IloInfinity, ILOINT);
        for (int i = 0; i < N + 1; i++) {
            sprintf(name, "D[%d]", i);
            D[i].setName(name);
        }

        IloNumVarArray Ct(cplex_env, N + 1, 0, IloInfinity, ILOINT);
        for (int i = 0; i < N + 1; i++) {
            sprintf(name, "Ct[%d]", i);
            Ct[i].setName(name);
        }

        NumVar3Matrix X(cplex_env, N + 1);
        for (int i = 0; i <= N; i++) {

            X[i] = NumVarMatrix(cplex_env, N + 1);

            for (int j = 0; j <= N; j++) {

                X[i][j] = IloNumVarArray(cplex_env, K, 0, 1, ILOINT);

                for (int k = 0; k < K; k++) {
                    sprintf(name, "x[%d][%d][%d]", i, j, k);
                    X[i][j][k].setName(name);
                }
            }
        }

        // Declaring Objective Function.

        // First therm: travelling costs.
        for (IloInt i = 0; i <= N; i++)
            for (IloInt j = 0; j <= N; j++)
                for (IloInt k = 0; k < K; k++)
                    funcaoO += t[i][j] * X[i][j][k];

        // Second therm: Vehicles use costs.
        for (IloInt k = 0; k < K; k++)
            funcaoO += F[k] * Y[k];

        // Third therm: Penalty weight costs.
        for (IloInt i = 1; i <= N; i++)
            funcaoO += w[i] * T[i];
        modeloP.add(IloMinimize(cplex_env, funcaoO));


        /*
            Constraint 2:
            Certifies that each customer is visited just once.
        */
        for (IloInt i = 1; i <= N; i++) {
            IloExpr sum(cplex_env);
            for (IloInt k = 0; k < K; k++) {
                for (IloInt j = 0; j <= N; j++) {
                    if (j == i)
                        continue; //j!=i
                    sum += X[i][j][k];
                }
            }

            Constraints.add(sum == 1);
            sum.end();
        }

        /*
            Constraint 3:
            Vehicle in use can just visit at most the number of carried jobs.
        */
        for (IloInt k = 0; k < K; k++) {
            IloExpr sum2(cplex_env);
            for (IloInt i = 0; i <= N; i++) {
                for (IloInt j = 0; j <= N; j++) {

                    if (i == j)
                        continue; //i!=j
                    sum2 += s[i] * X[i][j][k];
                }
            }

            Constraints.add((Q[k] * Y[k]) - sum2 >= 0);
            sum2.end();
        }

        /*
            Constraint 4:
            Used vehicles must leave and return to Origin.
        */
        for (IloInt k = 0; k < K; k++) {
            IloExpr sum3(cplex_env);
            for (IloInt j = 1; j <= N; j++) {
                sum3 += X[0][j][k];
            }

            Constraints.add(Y[k] - sum3 == 0);
            sum3.end();
        }

        /*
            Constraint 5:
            If a vehicle visits a customer, it must leave that customer.
        */
        for (IloInt k = 0; k < K; k++) {
            for (IloInt h = 0; h <= N; h++) {

                IloExpr sum4(cplex_env);
                IloExpr sum5(cplex_env);

                for (IloInt i = 0; i <= N; i++) {
                    sum4 += X[i][h][k];
                }
                for (IloInt j = 0; j <= N; j++) {
                    sum5 += X[h][j][k];
                }

                Constraints.add(sum5 - sum4 == 0);

                sum4.end();
                sum5.end();
            }
        }

        /*
            Constraint 6:
            Certifies that solution won't be any impossible job sequence.
        */
        for (IloInt i = 0; i <= N; i++) {
            for (IloInt j = 0; j <= N; j++) {
                if (i == j)
                    continue;

                Constraints.add(A[i][j] + A[j][i] == 1);
            }
        }

        /*
            Constraint 7:
            Also certifies that solution won't be any impossible job sequence.
        */
        for (IloInt i = 0; i <= N; i++) {
            for (IloInt j = 0; j <= N; j++) {
                for (IloInt r = 0; r <= N; r++) {
                    if ((i == j) || (j == r) || (i == r))
                        continue;

                    Constraints.add(A[i][j] + A[j][r] + A[r][i] >= 1);
                }
            }
        }

        // Calculating Big M.
        int M = 0;
        int biggest_distance = -1;
        for (int i = 0; i <= N; i++) {
            M += P[i];
            biggest_distance = biggest_distance > t[0][i] ? biggest_distance : int(t[0][i]);
        }

        M = M + int(N * biggest_distance);
        M += N * biggest_distance;

        /*
            Constraint 8:
            Certifies that the vehicle's start time occurs after it's jobs finish processing.
        */
        for (IloInt k = 0; k < K; k++) {
            for (IloInt j = 1; j <= N; j++) {
                IloExpr sum7(cplex_env);

                for (IloInt i = 0; i <= N; i++) {
                    if (i == j)
                        continue;
                    sum7 += X[i][j][k];
                }

                Constraints.add(S[k] - (Ct[j] - M * (1 - sum7)) >= 0);
                sum7.end();
            }
        }

        /*
            Constraint 9:
            Certifies that the delivery time of first job is bigger or equal than it's (vehicle start time + route time).
        */
        for (IloInt k = 0; k < K; k++)
            for (IloInt j = 1; j <= N; j++)
                Constraints.add((D[j] - S[k]) - (t[0][j] - M * (1 - X[0][j][k])) >= 0);

        /*
            Constraint 10:
            Same as C9, it certifies that the delivery time of remaining jobs are bigger or equal than it's (vehicle start time + route time).
        */
        for (IloInt i = 1; i <= N; i++) {
            for (IloInt j = 1; j <= N; j++) {

                IloExpr sum8(cplex_env);

                for (IloInt k = 0; k < K; k++)
                    sum8 += X[i][j][k];

                IloExpr maiorIgual3 = t[i][j] - M * (1 - sum8);
                Constraints.add((D[j] - D[i]) - maiorIgual3 >= 0);

                maiorIgual3.end();
                sum8.end();
            }
        }
        /*
            Constraint 11:
            Certifies that late job delivery "tardiness T" is bigger or equal than (D - d).
        */
        for (IloInt i = 1; i <= N; i++)
            Constraints.add(T[i] - D[i] + d[i] >= 0);


        /*
            Constraint 12:
            Declaring Completion Time variables.
        */
        for (int j = 0; j <= N; j++) {
            IloExpr igual(cplex_env);
            for (int i = 0; i <= N; i++) {
                if (i == j)
                    continue;

                igual += P[i] * A[i][j];
            }
            igual += P[j];

            Constraints.add(Ct[j] - igual == 0);
            igual.end();
        }

        /*
            Constraint 13, 14, 15:
            Set the Origin(job_id: 0) variables as 0.
        */
        Constraints.add(Ct[0] == 0);
        Constraints.add(D[0] == 0);
        Constraints.add(T[0] == 0);

        modeloP.add(Constraints);

        // Create model.
        IloCplex cplex1(modeloP);

		// MipStart Procedure
        if(useMip){

    		string line;
			getline(mipIn, line);
			cout << "Line is\\"<<line<<"\n";

			// Get variables' values of given solution S.

			loadMipStart(line, mip_N, mip_K, mip_Y, mip_S, mip_T, mip_D, mip_A,
                         mip_Ct, mip_X, mip_P, mip_t, mip_Q, mip_F, mip_d, mip_w);
            cout << "Load MIP rodou.\n";
			// Adding variables/values to MIPStart.

       		IloNumVarArray mip_variables(cplex_env);
       		IloNumArray mip_values(cplex_env);

       		for(int i = 0; i <= N; i++){
       			for(int j = 0; j <= N; j++){

       				if(i == j)
       					continue;

       				mip_variables.add(A[i][j]);
       				mip_values.add(mip_A[i][j]);

       				for(int k = 0; k < mip_K; k++){

       					mip_variables.add(X[i][j][k]);
       					mip_values.add(mip_X[i][j][k]);

       				}
       			}

                cout << "Teste aleatorio.\n";

       			mip_variables.add(T[i]);
   				mip_values.add(mip_T[i]);

   				mip_variables.add(D[i]);
   				mip_values.add(mip_D[i]);

    			mip_variables.add(Ct[i]);
   				mip_values.add(mip_Ct[i]);
       		}

            cout << "Teste aleatorio2.\n";

       		for(int i = 0; i < mip_K; i++){

			    mip_variables.add(Y[i]);
   				mip_values.add(mip_Y[i]);

   				mip_variables.add(S[i]);
   				mip_values.add(mip_S[i]);

       		}

            cout << "Teste aleatorio3.\n";

       		// Adding variables and values to MIPStart and closing them.
       		cplex1.addMIPStart(mip_variables, mip_values);
       		mip_variables.end();
       		mip_values.end();

       		cout << "MIPStart carregado com sucesso.\n";

    	}


        //1: CPU time, 2: Wall-clock time
        cplex1.setParam(IloCplex::ClockType, 1);

        // Get maximum available threads.
        cplex1.getMax(IloCplex::Threads);

        // Export model's constraints values.
        cplex1.exportModel("modelo.lp");


        cplex1.setParam(IloCplex::TiLim, 15);
        double runtime = 0.0;
        clock_t start, finish;
        start = clock();


        //cplex.setParam(IloCplex::Threads, 8);
        //cplex.setParam(IloCplex::IntSolLim, 2); //O Cplex finaliza sua execucao quando encontrar a 2a melhor solucao inteira.
        //cplex.setParam(IloCplex::RINSHeur, 1);// Habilita a heurÌstica RINS para melhora da performance do CPLEX.

        char instance[50] = "";
        sprintf(instance, "%d_%d_%d_%.1lf_%.1lf", instance_number, N, K, mi, delta);
        cout << "\n\File: " << instance << endl;

        if (!cplex1.solve()) {

            finish = clock(); //
            runtime = (double)(finish - start) / CLOCKS_PER_SEC;

            char instance[50] = "";
            sprintf(instance, "%d_%d_%d_%.1lf_%.1lf", instance_number, N, K, mi, delta);

            myfile << setw(15) << instance;
            myfile << setw(12) << 0.0;
            myfile << setw(15) << 0.0;
            myfile << setw(12) << "Infeasible";
            myfile << setw(12) << runtime << "\t";
        }
        else {

            ofstream cplexOutput("cplex_Debug.txt");

            finish = clock();
            runtime = (double)(finish - start) / CLOCKS_PER_SEC;

            // Print objective function and it's separated parts.

            cplexOutput << "OBJ_F: " << cplex1.getObjValue() << endl;

            long long int first_term = 0, sec_term = 0, third_term = 0;
            for (int i = 0; i <= N; i++) {
                for (int j = 0; j <= N; j++) {
                    if (i == j)
                        continue;

                    for (int k = 0; k < K; k++)
                        first_term += (cplex1.getValue(X[i][j][k]) * t[i][j]);
                }
            }
            cplexOutput << "1_TERM: " << first_term << endl;

            for (int k = 0; k < K; k++)
                sec_term += F[k] * cplex1.getValue(Y[k]);
            cplexOutput << "2_TERM: " << sec_term << endl;

            for (int i = 1; i <= N; i++)
                third_term += w[i] * cplex1.getValue(T[i]);
            cplexOutput << "3_TERM: " << third_term << "\n\n\n";

            // Print decision variables' values.

            cplexOutput << "Ci\n";
            for (IloInt i = 1; i <= N; i++)
                cplexOutput << cplex1.getValue(Ct[i]) << " ";
            cplexOutput << "\n";

            cplexOutput << "Sk\n";
            for (IloInt k = 0; k < K; k++)
                cplexOutput << cplex1.getValue(S[k]) << " ";
            cplexOutput << "\n";

            cplexOutput << "Yk\n";
            for (IloInt k = 0; k < K; k++)
                cplexOutput << cplex1.getValue(Y[k]) << " ";
            cplexOutput << "\n";

            cplexOutput << "Di\n";
            for (IloInt i = 1; i <= N; i++)
                cplexOutput << cplex1.getValue(D[i]) << " ";
            cplexOutput << "\n";

            cplexOutput << "Ti\n";
            for (IloInt i = 1; i <= N; i++)
                cplexOutput << cplex1.getValue(T[i]) << " ";
            cplexOutput << "\n";

            cplexOutput << "Aij\n";
            for (IloInt i = 0; i <= N; i++){
                for (IloInt j = 0; j <= N; j++){
                    if(i == j)
                        cout << -1 << " ";
                    else
                    	cplexOutput << cplex1.getValue(A[i][j]) << " ";
                }
                cplexOutput << "\n";
            }
            cplexOutput << "\n";

            cplexOutput.close();

            myfile << setw(15) << instance;
            myfile << setw(12) << cplex1.getObjValue();
            myfile << setw(10) << first_term;
            myfile << setw(10) << sec_term;
            myfile << setw(11) << third_term;
            myfile << setw(14) << cplex1.getMIPRelativeGap() * 100;
            myfile << setw(12) << cplex1.getStatus();
            myfile << setw(12) << runtime << "\t";

            // Print job sequence.

            vector<pair<int, int> > job_sequence(N);
            for (int i = 0; i < N; i++)
                job_sequence[i] = make_pair(0, i + 1);

            for (int i = 1; i <= N; i++) {
                for (int j = 1; j <= N; j++) {

                    if (i == j)
                        continue;

                    VarValue = cplex1.getValue(A[i][j]);

                    if (VarValue >= 0.9 && VarValue <= 1.1)
                        ++job_sequence[j - 1].first;
                }
            }
            sort(job_sequence.begin(), job_sequence.end());

            for (int i = 0; i < N; i++) {
                myfile << job_sequence[i].second;

                if (i != N - 1)
                    myfile << "-";

            }

            myfile << "    ";


            // Printing delivery sequence.

            vector<pair<int, int> > delivery;
            for (int i = 0; i < K; i++) {

                VarValue = cplex1.getValue(Y[i]);

                if (VarValue >= 0.9 && VarValue <= 1.1)
                    delivery.push_back(make_pair(cplex1.getValue(S[i]), i));

            }
            sort(delivery.begin(), delivery.end());

            int visited = 0;

            for (int i = 0; i < delivery.size(); i++) {

                int whichCar = delivery[i].second;

                //if (i == 0)
                    myfile << " [" << whichCar << "] ";

                int whichIndex = 0;
                for (int b = 0; b <= N; b++) {
                    if (b == whichIndex)
                        continue;

                    if (cplex1.getValue(X[whichIndex][b][whichCar]) >= 0.9 && cplex1.getValue(X[whichIndex][b][whichCar]) <= 1.1) {
                        myfile << b;
                        ++visited;

                        if(visited < N)
                            myfile << "-";

                        whichIndex = b;
                        break;
                    }
                }
                while ((cplex1.getValue(X[whichIndex][0][whichCar]) <= 0.1)) {
                    for (int b = 0; b <= N; b++) {
                        if (cplex1.getValue(X[whichIndex][b][whichCar]) >= 0.9 && cplex1.getValue(X[whichIndex][b][whichCar]) <= 1.1) {

                            myfile << b;
                            visited++;
                            whichIndex = b;

                            if ((cplex1.getValue(X[whichIndex][0][whichCar]) <= 0.1) && visited < N)
                                myfile << "-";

                            break;
                        }
                    }
                }
                /*
                if (i != delivery.size() - 1)
                    myfile << " [" << whichCar << "] ";
                else
                    myfile << endl; */

            }
            myfile << endl;
            cplex_env.end();
        }
    }

    in.close();
    myfile.close();

    if(useMip)
    	mipIn.close();

    return 0;
}
