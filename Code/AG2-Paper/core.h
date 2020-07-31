#ifndef CORE_H
#define CORE_H

#include <vector>
#include <set>
#include <queue>
#include <unordered_map>
#include <unordered_set>

#include <fstream>
#include <assert.h>
#include <math.h>
#include <limits>
#include <iostream>
#include <algorithm>
#include <chrono>
#include <random>
#include <time.h>
#include <iomanip> //setw

//Select Method version
#define CROSSOVER_OPTION 1
#define MUTATION_OPTION 0

#define EPS 1e-9

using namespace std;

struct Solution{

    vector<vector<int>> M;
    double Value = -1;

    void alocate(int N){
        M.resize(2, vector<int>(N, -1));
    }
};

void readInstance(ifstream &input, double &mi, double &delta, int &N, int &K,
                  vector<int> &P, vector<int> &d, vector<int> &s,
                  vector<double> &w, vector<int> &Q, vector<int> &F,
                  vector<vector<int>> &t)
{
    input >> mi >> delta;
    
    input >> N >> K;

    P.resize(N); d.resize(N);
    s.resize(N); w.resize(N);

    Q.resize(K); F.resize(K);
    t.resize(N+1, vector<int>(N+1, 0));

    // Job Variables
    for(int i=0; i<N; i++) 
        input >> P[i];
    for(int i=0; i<N; i++) 
        input >> d[i];
    for(int i=0; i<N; i++) 
        input >> s[i];
    for(int i=0; i<N; i++) 
        input >> w[i];

    // Vehicle Variables
    for(int i=0; i<K; i++) 
        input >> Q[i];
    for(int i=0; i<K; i++) 
        input >> F[i];

    for(int i=0; i<= N; i++)
        for(int j=0; j<=N; j++)
            input >> t[i][j]; 
    
}

void debugRead(double &mi, double &delta, int &N, int &K,
               vector<int> &P, vector<int> &d, vector<int> &s,
               vector<double> &w, vector<int> &Q, vector<int> &F,
               vector<vector<int>> &t)
{
    cout << "mi: " << mi << " delta: " << delta << "\n";
    cout << "N: " << N << " K: " << K << "\n";

    cout << "P: ";
    for(int i=0; i<N; i++) 
        cout <<  P[i] << " ";
    cout << "\n";

    cout << "d: ";
    for(int i=0; i<N; i++) 
        cout <<  d[i] << " ";
    cout << "\n";

    cout << "s: ";
    for(int i=0; i<N; i++) 
        cout <<  s[i] << " ";
    cout << "\n";

    cout << "w: ";
    for(int i=0; i<N; i++) 
        cout <<  w[i] << " ";
    cout << "\n";

    for(int i=0; i<K; i++) 
        cout <<  Q[i] << " ";
    cout << "\n";

    for(int i=0; i<K; i++) 
        cout <<  F[i] << " ";
    cout << "\n";

    for(int i=0; i<= N; i++){
        for(int j=0; j<=N; j++)
            cout <<  t[i][j] << " "; 
        cout << "\n";
    }
    cout << "\n";

}

void printSolution(const Solution &S, int N){

     cout << "Solution Value: " << fixed << S.Value << "\n";

    for(int i=0; i<N; i++) 
        cout << "V" << S.M[0][i] << " "; 
    cout << "\n";

    for(int i=0; i<N; i++) 
        cout << "J" << S.M[1][i] << " "; 
    cout << "\n\n";
}

void getVariables(int argc, char **argv, string &fileName, ifstream& input, int &gaVersion, int &itNumber, int &popSize){

    // Input via ARGV.
    if(argc == 5){

        fileName = argv[1];
        input.open(fileName);

        gaVersion = atoi(argv[2]);
        itNumber = atoi(argv[3]);
        popSize = atoi(argv[4]);



    }else{

        // Manual Input.    
        cout << "File name (with extension):  ";
        getline(cin, fileName);

        input.open(fileName);

        cout << "G.A. Version (1,2,3): ";
        cin >> gaVersion;

        cout << "Iterations: ";
        cin >> itNumber;
        
        cout << "Pop. Size: ";
        cin >> popSize;
    }

    if(popSize == 1) {
        cout << "PopSize needs to be >= 2. Killed.\n";
        exit(0);
    }
    if(gaVersion <= 0 || gaVersion >= 3) {

        cout << "Input valid Genetic Algorithm code:\n";
        cout << "1: GA2_Repr1\n";
        cout << "2: GA2_Repr1_BL\n";
        cout << "Killed.\n";
        exit(0);

    }else if(!input.is_open()) {

        cout << "Error opening file, please check directory or file content. Killed.\n";
        exit(0);

    }
}

bool compareSolution(const Solution &S1, const Solution &S2)
{   
    if( S1.Value < S2.Value)
        return true;
    else 
        return false;
}

// Complexity: O(N)
bool isSolution(const Solution &S, int N, int K){
    
    unordered_set<int> jobs;

    if(S.M[0].size() != N || S.M[1].size() != N){
        cout << "Positions not allocated properly \n";
        return false;
    }

    for(int i=0; i<N; i++){
        
        if(S.M[0][i] == -1 || S.M[1][i] == -1){
            cout << "Invalid value in position: " << i << "\n";
            return false;
        }

        if( S.M[0][i] > K ){
            cout << "Non-existing vehicle in use \n";
            return false;
        }

        if( jobs.find(S.M[1][i]) != jobs.end() ){
            cout << "Duplicated job! \n";
            return false;
        }
        jobs.insert( S.M[1][i] );
    }

    if( jobs.size() != N ){
        cout << "A Job is missing \n";
        return false;
    }

    return true;
}

// Complexity: O(N)
bool isFeasible(const Solution &S, int N, int K, const vector<int> &s, const vector<int> &Q)
{
    unordered_map<int, int> accSize; // * First: vehicle ID, Second: job sizes sum in vehicle 

    for(int i=0; i<N; i++){

        accSize[ S.M[0][i] ] += s[i];

        if( Q[ S.M[0][i] ] < accSize[ S.M[0][i] ])
            return false;
        
    }
    
    return true;
}

int getMultiplier(int N){
    
    if(N == 8 || N == 10)
        return 100;
    else if( N ==15 || N == 20)
        return 500;
    else if( N >= 50 && N <= 100)
        return 1000;

    return -1;
}

// Complexity: O( 2K + 2N ) = O(max(N, K)) = O( N )
double calculateObj(Solution &S, int N, int K, const vector<int> &P, const vector<int> &d, const vector<int> &s,
                    const vector<double> &w, const vector<int> &Q, const vector<int> &F,
                    const vector<vector<int>> &t)
{
    unordered_map<int, vector<int>> Schedule; // * First: Vehicle ID, Sec: Jobs
    unordered_map<int, int> JobPath; // * First: JobId, Sec: Vehicle ID
    unordered_map<int, int> accPTime; // * First: Vehicle ID, Sec: acumulated P 
    
    unordered_set<int> Vehicle;
    vector<int> Order;

    double UseCosts, TravelCosts, PenaltyCosts, OverlapCosts;
    UseCosts = TravelCosts = PenaltyCosts = OverlapCosts = 0.0;

    for(int i=0; i<N; i++){

        accPTime[ S.M[0][i] ] += P[ i ];
        
        JobPath[i] = S.M[0][i];

    }
    for(int i=0; i<N; i++){

        Schedule[ JobPath[ S.M[1][i]] ].push_back( S.M[1][i] );

        if(Vehicle.find(S.M[0][i]) == Vehicle.end()){
            Order.push_back( S.M[0][i] );
            Vehicle.insert( S.M[0][i] );
        }
    }

    for(int i=1; i < Order.size(); i++){
        accPTime[ Order[i] ] += accPTime[ Order[i-1] ];
    }

    for(int &idV: Order){

        UseCosts += F[ idV ];      
        
        int jobsNb = Schedule[idV].size();

        int SizeSum = 0; 
        int RouteTime = 0;
        RouteTime += t[0][ Schedule[idV][0]+1 ]; // Origin to First

        for(int i=0; i<jobsNb; i++){
            
            int JobId = Schedule[idV][i];

            SizeSum += s[ JobId ];

            int Delivery = accPTime[idV] + RouteTime;

            if(Delivery > d[ JobId ])
                PenaltyCosts += (Delivery - d[ JobId ]) * w[ JobId ];

            if(i < jobsNb-1)
                RouteTime += t[ JobId+1 ][ Schedule[idV][i+1]+1 ]; // time: JobId -> NextJob
        }

        if(SizeSum > Q[ idV ])
            OverlapCosts += SizeSum - Q[ idV ];
        
        RouteTime += t[ Schedule[idV][ jobsNb-1 ]+1 ][ 0 ]; // Last to Origin
        TravelCosts += RouteTime;
    }
    
    return UseCosts + TravelCosts + PenaltyCosts;
}

// Returns {Obj_Function, Overlaps}.
pair<double, double> calculateObjWithOverlap(Solution &S, int N, int K, const vector<int> &P, const vector<int> &d, const vector<int> &s,
                    const vector<double> &w, const vector<int> &Q, const vector<int> &F,
                    const vector<vector<int>> &t)
{
    unordered_map<int, vector<int>> Schedule; // * First: Vehicle ID, Sec: Jobs
    unordered_map<int, int> JobPath; // * First: JobId, Sec: Vehicle ID
    unordered_map<int, int> accPTime; // * First: Vehicle ID, Sec: acumulated P 
    
    unordered_set<int> Vehicle;
    vector<int> Order;

    double UseCosts, TravelCosts, PenaltyCosts, OverlapCosts;
    UseCosts = TravelCosts = PenaltyCosts = OverlapCosts = 0.0;

    for(int i=0; i<N; i++){

        accPTime[ S.M[0][i] ] += P[ i ];
        
        JobPath[i] = S.M[0][i];

    }
    for(int i=0; i<N; i++){

        Schedule[ JobPath[ S.M[1][i]] ].push_back( S.M[1][i] );

        if(Vehicle.find(S.M[0][i]) == Vehicle.end()){
            Order.push_back( S.M[0][i] );
            Vehicle.insert( S.M[0][i] );
        }
    }

    for(int i=1; i < Order.size(); i++){
        accPTime[ Order[i] ] += accPTime[ Order[i-1] ];
    }

    for(int &idV: Order){

        UseCosts += F[ idV ];      
        
        int jobsNb = Schedule[idV].size();

        int SizeSum = 0; 
        int RouteTime = 0;
        RouteTime += t[0][ Schedule[idV][0]+1 ]; // Origin to First

        for(int i=0; i<jobsNb; i++){
            
            int JobId = Schedule[idV][i];

            SizeSum += s[ JobId ];

            int Delivery = accPTime[idV] + RouteTime;

            if(Delivery > d[ JobId ])
                PenaltyCosts += (Delivery - d[ JobId ]) * w[ JobId ];

            if(i < jobsNb-1)
                RouteTime += t[ JobId+1 ][ Schedule[idV][i+1]+1 ]; // time: JobId -> NextJob
        }

        if(SizeSum > Q[ idV ])
            OverlapCosts += SizeSum - Q[ idV ];
        
        RouteTime += t[ Schedule[idV][ jobsNb-1 ]+1 ][ 0 ]; // Last to Origin
        TravelCosts += RouteTime;
    }

    //cout << "Travel C. " << TravelCosts << "/ Use C. " << UseCosts << "/ Penalty C. " << PenaltyCosts << "/ Overlap C. " << OverlapCosts << "\n";
    
    return {UseCosts + TravelCosts + PenaltyCosts, OverlapCosts};
}

void randomSolution(Solution &S, int N, int K)
{
    vector<int> permutation(N);
    for(int i=0; i<N; i++) permutation[i] = i; // * 0-indexed

    random_shuffle(permutation.begin(), permutation.end());
    
    for(int i=0; i<N; i++){
        S.M[1][i] = permutation[i];
        S.M[0][i] = rand()%K;
    }
}


#endif