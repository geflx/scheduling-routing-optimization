#ifndef CORE_H
#define CORE_H

#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <queue>

#include <fstream>
#include <assert.h>
#include <math.h>
#include <limits>
#include <iostream>
#include <algorithm>
#include <chrono>
#include <random>
#include <time.h>

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

void getVariables(string &fileName, ifstream& input, int &gaVersion, int &runNb, int &itNumber, int &popSize){

    
    cout << "Input file name with '.txt' :  ";
    getline(cin, fileName);

    input.open(fileName);

    cout << "Which GA Version (1,2)? ";
    cin >> gaVersion;

    cout << "How many times to run? ";
    cin >> runNb;

    cout << "Iterations: ";
    cin >> itNumber;
    
    cout << "Population Size: ";
    cin >> popSize;


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
}
#endif