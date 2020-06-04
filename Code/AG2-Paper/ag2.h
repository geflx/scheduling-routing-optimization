#ifndef AG2_H
#define AG2_H

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

struct data{
    bool job; //If it is a job: 1, otherwise it is a Vehicle
    int id;  //Job or vehicle identification
    data(){
        job=true; //By default, the position is a job.
        id=-1;
    }
};

struct vehicleLoaded{
    int id;
    int initialPos;
    int finalPos;
};

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
    for(int i=0; i<N; i++) permutation[i] = i; // * 0-indexed

    random_shuffle(permutation.begin(), permutation.end());
    
    for(int i=0; i<N; i++){
        S.M[1][i] = permutation[i];
        S.M[0][i] = rand()%K;
    }
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

    //cout << "Travel C. " << TravelCosts << "/ Use C. " << UseCosts << "/ Penalty C. " << PenaltyCosts << "/ Overlap C. " << OverlapCosts << "\n";
    
    return UseCosts + TravelCosts + PenaltyCosts;
}

pair<double, double> tempObj(Solution &S, int N, int K, const vector<int> &P, const vector<int> &d, const vector<int> &s,
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
    assert( cont == N);
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
        idP1 = idP2 = rand() % popSize;

        while(idP2 == idP1)
            idP2 = rand() % popSize;
        
        Solution OFF1, OFF2;
        OFF1 = CrossOver(PP[idP1], PP[idP2], N, K, P, d, s, w, Q, F, t); // ! CO(P1, P2)
        OFF2 = CrossOver(PP[idP2], PP[idP1], N, K, P, d, s, w, Q, F, t); // ! CO(P2, P1)

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
            idP1 = idP2 = rand() % popSize;

            while(idP2 == idP1)
                idP2 = rand() % popSize;
                        
            Solution OFF1, OFF2;
            OFF1 = CrossOver(PP[idP1], PP[idP2], N, K, P, d, s, w, Q, F, t); // ! CO(P1, P2)
            OFF2 = CrossOver(PP[idP2], PP[idP1], N, K, P, d, s, w, Q, F, t); // ! CO(P2, P1)

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

        if(Local_S_best.Value < S_best.Value){

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

// * 1 Review
bool isFeasible(const Solution &S, int N, int K, const vector<int> &s, const vector<int> &Q)
{
    unordered_map<int, int> accSize; // * First: vehicle ID, Second: job sizes sum in vehicle 

    for(int i=0; i<N; i++){

        accSize[ S.M[0][i] ] += s[i];

        if( Q[ S.M[0][i] ] < accSize[ S.M[0][i] ]){
            return false;
        }
    }
    
    return true;
}

// * 1 Review
bool isSolution(const Solution &S, int N, int K){
    
    unordered_set<int> jobs;

    if(S.M[0].size() != N || S.M[1].size() != N){

        cout << "Positions not allocated properly \n";
        return false;
    }

    for(int i=0; i<N; i++){
        
        if( S.M[0][i] > K ){
            cout << "Not existing vehicle in use \n";
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

// * 1 Review
pair<vector<int>, int> getOverloadStatus(Solution &S, int N, int K, const vector<int> &s, const vector<int> &Q){
    
    vector<int> accSize (K, 0); // * acc job size per vehicle

    for(int i=0; i<N; i++)
        accSize[ S.M[0][i] ] += s[i];

    int countOver = 0;

    for(int i=0; i<K; i++)
        if(accSize[i] > Q[i])
            countOver++;
    
    return {accSize, countOver};
}

// * 1 Review
void makeFeasible(Solution &S, int N, int K, const vector<int> &P, const vector<int> &d, const vector<int> &s,
                    const vector<double> &w, const vector<int> &Q, const vector<int> &F,
                    const vector<vector<int>> &t)
{
    pair<vector<int>, int> status = getOverloadStatus(S, N, K, s, Q); // * First: job size sum (size: K), Second: total overload

    vector<vector<int>> m( K, vector<int>()); // Matrix with K vectors of carried jobs (disabled pos == -1)
    
    for(int i=0; i<N; i++)
        m[ S.M[0][i] ].push_back( i );
    
    for(int i=0; i<K; i++){
        if( status.first[i] <= Q[i] ) // Vehicle carried Job Size is OK
            continue;

        for(int j=0; j<m[i].size(); j++){ //For each job in it

            if(status.first[i] <= Q[i]) // Not a problem anymore!
                break;

            if(m[i][j] == -1) // ! Ignore disabled job positions 
                continue;

            for(int k=0; k<K; k++){
 
                if(i == k)  // ! If its the same vehicle
                    continue;

                if( (status.first[k] + s[ m[i][j] ]) <= Q[k] ){
                    
                    status.first[i] -= s[ m[i][j] ];
                    status.first[k] += s[ m[i][j] ];

                    m[k].push_back( m[i][j] );
                    m[i][j] = -1;

                    break;
                }
            }
        }
        
    }

    unordered_set<int> checkJobs;

    for(int i=0; i<K; i++){
        for(int j=0; j<m[i].size(); j++){
            
            if(m[i][j] != -1){
                assert( checkJobs.find(m[i][j]) == checkJobs.end() );
                
                checkJobs.insert(m[i][j]);
                S.M[0][ m[i][j] ] = i; // Assign job m[i][j] to vehicle i
            }
        }
    }

    assert( checkJobs.size() == N );

    assert( isSolution(S, N, K) == true );

    pair<vector<int>, int> finalStatus = getOverloadStatus(S, N, K, s, Q);
    
    // if(finalStatus.second > 0)
    //     cout << "Can't make solution feasible \n";
    // else
    //     cout << "Solution is now feasible \n";

    S.Value = calculateObj(S, N, K, P, d, s, w, Q, F, t);

}

bool compareSolution(const Solution &S1, const Solution &S2)
{   
    if( S1.Value < S2.Value)
        return true;
    else 
        return false;
}

// * 1 Review
Solution New_GA_Version_2 (int N, int K, int itNumber, int popSize, double mutateProb,
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

        for(int i=0; i<popSize; i+=2){ // ! i+=2

            int idP1, idP2;
            idP1 = idP2 = rand() % popSize;

            while(idP2 == idP1)
                idP2 = rand() % popSize;
            
            Solution OFF1, OFF2;
            OFF1 = CrossOver(PP[idP1], PP[idP2], N, K, P, d, s, w, Q, F, t); // ! CO(P1, P2)
            OFF2 = CrossOver(PP[idP2], PP[idP1], N, K, P, d, s, w, Q, F, t); // ! CO(P2, P1)

            double R = ( rand()%101 ) / 100.0;

            if( R <= mutateProb ){

                OFF1 = Mutation(OFF1, N, K, P, d, s, w, Q, F, t);
                OFF2 = Mutation(OFF2, N, K, P, d, s, w, Q, F, t);

            }
            bool feasibleOFF1 = isFeasible(OFF1, N, K, s, Q);
            bool feasibleOFF2 = isFeasible(OFF2, N, K, s, Q);

            if( !feasibleOFF1 || !feasibleOFF2 ){

                int B = rand()%2; // * 0 or 1

                // * if(B == 0) penalize obj functions: Ok.

                if(B == 1){

                    if(!feasibleOFF1)
                        makeFeasible(OFF1, N, K, P, d, s, w, Q, F, t);
                    
                    if(!feasibleOFF2)
                        makeFeasible(OFF2, N, K, P, d, s, w, Q, F, t);
                }
            }
            P_New.push_back( OFF1 );
            P_New.push_back( OFF2 );

            if(OFF1.Value < OFF2.Value)
                if(OFF1.Value < Local_S_best.Value)
                    Local_S_best = OFF1;
            else
                if(OFF2.Value < Local_S_best.Value)
                    Local_S_best = OFF2;
            
        }
        if(Local_S_best.Value < S_best.Value){

            S_best = Local_S_best;
            cont = 0;

        }

        // ! Adding P solutions to P' and sorting P'
        for(int i=0; i<PP.size(); i++)
            P_New.push_back( PP[i] );

        sort(P_New.begin(), P_New.end(), compareSolution); // ! Increasing results:  0 -> 0.001 -> 0.002 ...

        assert(P_New[0].Value <= P_New[1].Value);

        PP.clear();

        assert(PP.size() == 0);

        for(int i=0; i<popSize; i++){
            PP.push_back( P_New[i] );
        }

    }

    return S_best;
}

#endif