#ifndef CORE_H
#define CORE_H

#include <vector>
#include <unordered_set>
#include <unordered_map>
#include <set>
#include <queue>
#include <iostream>
#include <fstream>
#include <iomanip>
#include <time.h>
#include <algorithm>
#include <chrono>
#include <random>
#include <assert.h>
using namespace std;

#define EPS 1e-5
#define INF 987654321

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

struct Execution {
    double value = -1, time = -1;
    vector<data> vec;
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

double ObjectiveFunction(const vector<data>& S, vector<vehicleLoaded>& vehicleOrder,
    bool refreshVOrder, int K, int N,
    const vector<vector<int> >& t, const vector<double>& w,
    const vector<int>& P, const vector<int>& d,
    const vector<int>& F, int v)
{
    double travelC = 0.0, useC = 0.0, weightC = 0.0, accPtime = 0.0;
    int vOrderSize = vehicleOrder.size();

    if (v != -1)
        vOrderSize = 1;

    if (refreshVOrder)
        vehicleOrder = getVOrder(S, K);

    for (int i = 0; i < vOrderSize; i++) {

        // * Single vehicle Objective Function.
        if (v != -1) {
            i = v;

            // Accumulate processing times from [begin, current vehicle].
            for (int j = 0; j < i; j++)
                for (int k = vehicleOrder[j].initialPos; k <= vehicleOrder[j].finalPos;
                     k++)
                    accPtime += P[S[k].id];
        }

        int iniJobPos = vehicleOrder[i].initialPos;
        int endJobPos = vehicleOrder[i].finalPos;

        for (int j = iniJobPos; j <= endJobPos; j++)
            accPtime += P[S[j].id];

        double localTravelC = t[0][S[iniJobPos].id + 1];

        for (int j = iniJobPos; j < endJobPos; j++) {

            // Calculate Job Tardiness and following the Penalty weight cost.
            double T = (accPtime + localTravelC) - d[S[j].id];
            if (T > 0.001)
                weightC += T * w[S[j].id];

            localTravelC += t[S[j].id + 1][S[j + 1].id + 1];
        }

        double T = (accPtime + localTravelC) - d[S[endJobPos].id];
        if (T > 0.001)
            weightC += T * w[S[endJobPos].id];

        localTravelC += t[0][S[endJobPos].id + 1]; // Last job to origin.

        useC += F[vehicleOrder[i].id];
        travelC += localTravelC;

        // * Calculate for jut one vehicle.
        if (v != -1)
            break;
    }

    // cout <<"tc2 " << travelC << endl;
    // cout <<"wc2 " << weightC << endl;
    // cout <<"uc2 " << useC << endl;
    return travelC + weightC + useC;
}

bool IsFeasible(const vector<data>& config, const vector<int>& capacities, const vector<int>& jobSize, int N, int K)
{
    unordered_set<int> jobsIn, carIn;

    // Missing first vehicle or incorrect size.
    if(config[0].job || config.size() != (N + K))
        return false;

    for (int i = 0; i < config.size(); i++) {

        // ID == -1.
        if (config[i].id == -1) return false;

        if (config[i].job) {

            // Check if job repeats.
            if (jobsIn.find(config[i].id) == jobsIn.end())
                jobsIn.insert(config[i].id);
            else
                return false;

            // Job's ID out of boundaries.
            if (config[i].id < 0 || config[i].id >= N) return false;
        }
        else {

            // Check if vehicle repeats.
            if (carIn.find(config[i].id) == carIn.end())
                carIn.insert(config[i].id);
            else
                return false; 

            // Vehicle ID out of boundaries.
            if (config[i].id < 0 || config[i].id >= K) return false;
        }
    }

    // Check vehicle integrity (capacity).
    int i = 0;
    while (i < config.size()) {

        int j = i;
        if (config[i].job == false) {

            j = i + 1;
            if (j >= config.size()) 
                break;

            int accCap = 0, carCapacity = capacities[config[i].id];

            while (j < config.size() && config[j].job) {
                accCap += jobSize[config[j].id];
                if (accCap > carCapacity) 
                    return false;
                ++j;
            }
        }
        i = j;
    }

    return true;
}

vector<data> RandomSolution(int N, int K, const vector<int>& vehicleCap, const vector<int>& jobSizes)
{

    // Generate a random config based on assigning random jobs to random cars
    // until the last ones are full.
    bool feasible = false;
    vector<data> solution(N + K);

    while (!feasible) {

        //[Random] Shuffled  Jobs and Shuffled Cars
        vector<int> s_jobs(N);
        vector<int> s_cars(K);

        for (int i = 0; i < N; i++)
            s_jobs[i] = i;
        for (int i = 0; i < K; i++)
            s_cars[i] = i;

        random_shuffle(s_jobs.begin(), s_jobs.end());
        random_shuffle(s_cars.begin(), s_cars.end());

        int checkjob = 0;
        int checkinsert = 0;

        for (int i = 0; i < K; i++) {

            solution[checkinsert].job = false;
            solution[checkinsert++].id = s_cars[i];

            int carry = 0;
            for (int j = checkjob; j < N; j++) {
                if ((jobSizes[s_jobs[j]] + carry) > vehicleCap[s_cars[i]]) {
                    break;
                }
                else {
                    solution[checkinsert].job = true;
                    solution[checkinsert++].id = s_jobs[j];

                    carry += jobSizes[s_jobs[j]];
                    checkjob = j + 1;
                }
            }
        }

        feasible = IsFeasible(solution, vehicleCap, jobSizes, N, K);
    }
    return solution;
}

void printConfig(ofstream &outFile, const Execution& S, double time, const vector<int>& carCap,
    const vector<int>& jobSize, int N, int K,
    const vector<int>& processTime,
    const vector<vector<int> >& t, const vector<int>& dueDate,
    const vector<double>& weight, const vector<int>& carPrices,
    int instNumber, double mi, double delta)
{

    char instanceId[50];
    sprintf(instanceId, "%d_%d_%d_%.1lf_%.1lf", instNumber, N, K, mi, delta);
    outFile << instanceId << " ";

    vector<vehicleLoaded> vehicleOrder = getVOrder(S.vec, K);

    int numJobsFixo = jobSize.size();

    if (numJobsFixo == 20)
        numJobsFixo -= 4;
    else if (numJobsFixo == 15)
        numJobsFixo -= 1;
    else if (numJobsFixo == 8)
        numJobsFixo += 2;

    outFile << setw(13) << "Time " << time << "    ";
    outFile << setw(13) << S.value << "    ";

    int numJobs = jobSize.size();

    string procOrder;

    for (int j = 0; j < S.vec.size(); j++) {
        if (S.vec[j].job) {
            procOrder += to_string(S.vec[j].id + 1);
            if (numJobs > 1) {
                procOrder += "-";
                numJobs--;
            }
        }
    }
    outFile << setw(numJobsFixo * 3) << procOrder;
    int j = 0;

    string deliveryOrder;

    for (int i = 0; i < S.vec.size(); i++) {
        if (S.vec[i].job) {
            deliveryOrder += to_string(S.vec[i].id + 1) + " ";
        }
        else {
            deliveryOrder += " [" + to_string(S.vec[i].id + 1) + "] ";
        }
    }
    for (int i = 1; i < deliveryOrder.size() - 1; i++) {
        if (deliveryOrder[i - 1] >= '0' && deliveryOrder[i - 1] <= '9' && deliveryOrder[i + 1] >= '0' && deliveryOrder[i + 1] <= '9') {
            deliveryOrder[i] = '-';
        }
    }
    outFile << setw(numJobsFixo * (5.5)) << deliveryOrder << "\n\n";
}

vector<int> getWeightByVehicle(vector<data>& config, const vector<int>& jobSize,
    int K)
{
    // Danger: confirmar se funciona
    vector<int> carry(K, 0);

    int walk = 0;

    while (walk < config.size()) {

        int walkAhead = walk;
        if (config[walk].job == false) {

            walkAhead = walk + 1;

            if (walkAhead >= config.size()) {
                break;
            }

            int accCap = 0;

            while (walkAhead < config.size() && config[walkAhead].job) {

                accCap += jobSize[config[walkAhead].id];
                ++walkAhead;
            }
            carry[config[walk].id] = accCap;
        }

        walk = walkAhead;
    }
    return carry;
}

void readInstance(ifstream& input, double& mi, double& delta, int& N, int& K,
    vector<int>& P, vector<int>& d, vector<int>& s,
    vector<double>& w, vector<int>& Q, vector<int>& F,
    vector<vector<int> >& t)
{
    input >> mi >> delta;

    input >> N >> K;

    P.resize(N);
    d.resize(N);
    s.resize(N);
    w.resize(N);

    Q.resize(K);
    F.resize(K);
    t.resize(N + 1, vector<int>(N + 1, 0));

    // Job Variables
    for (int i = 0; i < N; i++)
        input >> P[i];
    for (int i = 0; i < N; i++)
        input >> d[i];
    for (int i = 0; i < N; i++)
        input >> s[i];
    for (int i = 0; i < N; i++)
        input >> w[i];

    // Vehicle Variables
    for (int i = 0; i < K; i++)
        input >> Q[i];
    for (int i = 0; i < K; i++)
        input >> F[i];

    for (int i = 0; i <= N; i++)
        for (int j = 0; j <= N; j++)
            input >> t[i][j];
}

void getVariables(string& fileName, ifstream& input, int& metaheuristic,
    int& parameter1, int& parameter2)
{

    cout << "Input file name: ";
    getline(cin, fileName);

    input.open(fileName);

    cout << "Select ILS_RVND(1), ILS_RVND_2(2), GA_LS(3): ";
    cin >> metaheuristic;

    if (metaheuristic <= 2) {

        cout << "Input restart times (maxIter): ";
        cin >> parameter1;

        cout << "Input max. iterations (maxIterIls): ";
        cin >> parameter2;
    }
    else if (metaheuristic == 3) {

        cout << "Input Population Size (psize): ";
        cin >> parameter1;
    }
}

#endif
