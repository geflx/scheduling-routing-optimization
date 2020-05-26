#ifndef AG2_H
#define AG2_H

#include <fstream>
#include <limits>
#include <iostream>
#include <vector>
#include <algorithm>
#include <unordered_map>
#include <unordered_set>

//Select Method version
#define CROSSOVER_OPTION 0
#define MUTATION_OPTION 0

using namespace std;

struct Solution{

    vector<vector<int>> M;
    double Value = -1;

    void alocate(int N){
        M.resize(2, vector<int>(N, -1));
    }
};

void randomSolution(Solution &S, int N, int K)
{
    vector<int> permutation(N);
    for(int i=1; i<=N; i++) permutation[i-1] = i; // * 1-indexed

    random_shuffle(permutation.begin(), permutation.end());
    
    for(int i=0; i<N; i++){
        S.M[1][i] = permutation[i];
        S.M[0][i] = rand()%K + 1;
    }
}

// Complexity: O( N + (K+N) ) = O(N)
double calculateObj(Solution &S, int N, int K, const vector<int> &P, const vector<int> &d, const vector<int> &s,
                    const vector<double> &w, const vector<int> &Q, const vector<int> &F,
                    const vector<vector<int>> &t)
{
    unordered_map<int, vector<int>> Schedule; // * First: Vehicle ID , Sec: Jobs
    unordered_map<int, int> accPTime; // * First: Vehicle ID, Sec: acumulated P 

    unordered_set<int> Vehicle;
    vector<int> Order;

    double UseCosts, TravelCosts, PenaltyCosts, OverlapCosts;
    UseCosts = TravelCosts = PenaltyCosts = OverlapCosts = 0.0;

    for(int i=0; i<N; i++){
        Schedule[ S.M[0][i] ].push_back( S.M[1][i] );

        if(Vehicle.find(S.M[0][i]) == Vehicle.end()){
            Order.push_back( S.M[0][i] );
            Vehicle.insert( S.M[0][i] );
        }
        accPTime[ S.M[0][i] ] += P[ S.M[1][i]-1 ];
    }

    for(int &idV: Order){

        UseCosts += F[ idV-1 ];      
        
        int jobsNb = Schedule[idV].size();

        int SizeSum = 0; 
        int RouteTime = 0;
        RouteTime += t[0][ Schedule[idV][0] ]; // Origin to First

        for(int i=0; i<jobsNb; i++){
            
            int JobId = Schedule[idV][i];

            SizeSum += s[ JobId-1 ];

            int Delivery = accPTime[idV] + RouteTime;

            if(Delivery > d[ JobId-1 ])
                PenaltyCosts += (Delivery - d[ JobId-1 ]) * w[ JobId-1 ];

            if(i < jobsNb-1)
                RouteTime += t[ JobId ][ Schedule[idV][i+1] ]; // time: JobId -> NextJob
        }

        if(SizeSum > Q[ idV-1 ])
            OverlapCosts += SizeSum - Q[ idV-1 ];
        
        RouteTime += t[ Schedule[idV][ jobsNb-1 ] ][ 0 ]; // Last to Origin
        TravelCosts += RouteTime;
    }

    // cout << "TravelCosts: " << TravelCosts << "\n";
    // cout << "UseCosts: " << UseCosts << "\n";
    // cout << "PenaltyCosts: " << PenaltyCosts << "\n";
    // cout << "OverlapCosts: " << OverlapCosts << "\n";
    return UseCosts + TravelCosts + PenaltyCosts + OverlapCosts;
}

// Complexity: O(N) due to Solution Evaluation
Solution CrossOver(const Solution &S1, const Solution &S2, int N, int K, const vector<int> &P, const vector<int> &d, const vector<int> &s,
                   const vector<double> &w, const vector<int> &Q, const vector<int> &F,
                   const vector<vector<int>> &t)
{
    Solution S;
    S = S1;

    int point = rand() % N;
    
    unordered_set<int> MixJobs;
    for(int i=point; i<N; i++) 
        MixJobs.insert( S.M[1][i] );
    
    int cont = point;

    for(int i=0; i<N; i++){
        
        if(cont == N)
            break;

        if( CROSSOVER_OPTION == 0){
            // Change Job and Vehicle  
            if( MixJobs.find(S2.M[1][i]) != MixJobs.end() ){

                S.M[0][cont] = S2.M[0][i];
                S.M[1][cont] = S2.M[1][i];

                ++cont;
            }

        }else if( CROSSOVER_OPTION == 1){
            // Change just Job, keeping the same Vehicle
            if( MixJobs.find(S2.M[1][i]) != MixJobs.end() ){

                S.M[1][cont] = S2.M[1][i];

                ++cont;
            }
        }
    }

    S.Value = calculateObj(S, N, K, P, d, s, w, Q, F, t);

    return S;

}

// Complexity: O(N) due to Solution Evaluation
Solution Mutation( Solution &S, int N, int K, const vector<int> &P, const vector<int> &d, const vector<int> &s,
                   const vector<double> &w, const vector<int> &Q, const vector<int> &F,
                   const vector<vector<int>> &t)
{
    int A, B;
    A = B = rand() % N;
    
    while(A == B){
        B = rand() % N;
    }
    if( A > B ) 
        swap(A, B);

    pair<int, int> TempA = { S.M[0][A], S.M[1][A] };

    // Mutate Job
    S.M[1][A] = S.M[1][B];
    S.M[1][B] = TempA.second;

    if( MUTATION_OPTION == 0){
        // Also mutate Vehicle
        S.M[0][A] = S.M[0][B];
        S.M[0][B] = TempA.first;

    }

    S.Value = calculateObj(S, N, K, P, d, s, w, Q, F, t);

    return S;
    
}

Solution GA_Version_1 (int N, int K, int itNumber, int popSize,
              const vector<int> &P, const vector<int> &d, const vector<int> &s,
              const vector<double> &w, const vector<int> &Q, const vector<int> &F,
              const vector<vector<int>> &t)
{
    Solution S_best;
    S_best.alocate(N);
    S_best.Value = numeric_limits<double>::infinity();

    vector<Solution> PP(popSize);

    for(Solution &S: PP){
        S.alocate(N);
        randomSolution(S, N, K);
        S.Value = calculateObj(S, N, K, P, d, s, w, Q, F, t);

        if(S_best.Value > S.Value){
            S_best = S;
        }
    }
    
    int cont = 0;

    while(cont < itNumber){

        ++cont;

        int idP1, idP2;
        idP1 = idP2 = -1;

        idP1 = rand() % popSize;
        while(idP2 == idP1 || idP2 == -1)
            idP2 = rand() % popSize;
        
        Solution P1, P2;  
        P1 = PP[idP1];
        P2 = PP[idP2];

        Solution OFF1, OFF2;
        OFF1 = CrossOver(P1, P2, N, K, P, d, s, w, Q, F, t); // ! CO(P1, P2)
        OFF2 = CrossOver(P2, P1, N, K, P, d, s, w, Q, F, t); // ! CO(P2, P1)

        Solution Local_S_best;
        if(OFF1.Value < OFF2.Value)
            Local_S_best = OFF1;
        else
            Local_S_best = OFF2;

        OFF1 = Mutation(OFF1, N, K, P, d, s, w, Q, F, t);
        OFF2 = Mutation(OFF2, N, K, P, d, s, w, Q, F, t);

        if(OFF1.Value <= OFF2.Value){
            if(OFF1.Value < Local_S_best.Value)
                Local_S_best = OFF1;
        }else{
            if(OFF2.Value < Local_S_best.Value)
                Local_S_best = OFF2;
        }
        // Parents will be replaced with Offsprings
        PP[idP1] = OFF1;
        PP[idP2] = OFF2;

        if(Local_S_best.Value < S_best.Value){
            S_best = Local_S_best;
            cont = 0;
        }
    }
    return S_best;
}

Solution GA_Version_2 (int N, int K, int itNumber, int popSize,
              const vector<int> &P, const vector<int> &d, const vector<int> &s,
              const vector<double> &w, const vector<int> &Q, const vector<int> &F,
              const vector<vector<int>> &t)
{
    Solution S_best;
    S_best.alocate(N);
    S_best.Value = numeric_limits<double>::infinity();

    vector<Solution> PP(popSize);

    for(Solution &S: PP){
        S.alocate(N);
        randomSolution(S, N, K);
        S.Value = calculateObj(S, N, K, P, d, s, w, Q, F, t);

        if(S_best.Value > S.Value){
            S_best = S;
        }
    }
    
    int cont = 0;

    while(cont < itNumber){

        ++cont;
        
        vector<Solution> P_New;

        Solution Local_S_best;
        Local_S_best.Value = numeric_limits<double>::infinity();

        for(int i=0; i< popSize; i+= 2){ // ! i+=2 : adding two new solutions

            int idP1, idP2;
            idP1 = idP2 = -1;

            idP1 = rand() % popSize;
            while(idP2 == idP1 || idP2 == -1)
                idP2 = rand() % popSize;
            
            Solution P1, P2;  
            P1 = PP[idP1];
            P2 = PP[idP2];

            Solution OFF1, OFF2;
            OFF1 = CrossOver(P1, P2, N, K, P, d, s, w, Q, F, t); // ! CO(P1, P2)
            OFF2 = CrossOver(P2, P1, N, K, P, d, s, w, Q, F, t); // ! CO(P2, P1)

            OFF1 = Mutation(OFF1, N, K, P, d, s, w, Q, F, t);
            OFF2 = Mutation(OFF2, N, K, P, d, s, w, Q, F, t);

            P_New.push_back(OFF1);
            P_New.push_back(OFF2);

            if(OFF1.Value <= OFF2.Value){
                if(OFF1.Value < Local_S_best.Value)
                    Local_S_best = OFF1;
            }else{
                if(OFF2.Value < Local_S_best.Value)
                    Local_S_best = OFF2;
            }
        }

        if(S_best.Value < Local_S_best.Value){

            S_best = Local_S_best;
            cont = 0;
        
        }

        // Update Population
        PP = P_New; 
        
        int randomSol = rand() % PP.size();
        PP[ randomSol ] = S_best;
                       
    }

    return S_best;
}

#endif