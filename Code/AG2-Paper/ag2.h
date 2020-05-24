#ifndef AG2_H
#define AG2_H

#include <limits>
#include <iostream>
#include <vector>
#include <algorithm>
#include <unordered_map>
#include <unordered_set>

using namespace std;

struct Solution{

    vector<vector<int>> M = vector<vector<int>>(2);
    double Value = -1;

    void alocate(int N){
        M[0].resize(N, -1);
        M[1].resize(N, -1);
    }
};

void randomSolution(Solution &S, int N, int K)
{
    vector<int> permutation(N);
    for(int i=1; i<=N; i++) permutation[i] = i; // * 1-indexed

    random_shuffle(permutation.begin(), permutation.end());
    
    for(int i=0; i<N; i++){
        S.M[1][i] = permutation[i];
        S.M[0][i] = rand()%K + 1;
    }
}

double calculateObj(Solution &S, int N, int K, const vector<int> &P, const vector<int> &d, const vector<int> &s,
                    const vector<double> &w, const vector<int> &Q, const vector<int> &F,
                    const vector<vector<int>> &t)
{
    double Ans;

    unordered_map<int, vector<int>> Schedule; // * First: Vehicle ID , Sec: Jobs

    unordered_set<int> Vehicle;
    vector<int> Order;

    double UseCosts, TravelCosts, PenaltyCosts, OverlapCosts;
    UseCosts = TravelCosts = PenaltyCosts = OverlapCosts = 0;

    for(int i=0; i<N; i++){
        Schedule[ S.M[0][i] ].push_back( S.M[1][i] );

        if(Vehicle.find(S.M[0][i]) == Vehicle.end()){
            Order.push_back( S.M[0][i] );
            Vehicle.insert( S.M[0][i] );
        }
    }

    int accPTime = 0;
    for(int &idV: Order){
        UseCosts += F[ idV-1 ];      

        int SizeSum = 0; 

        int jobsNb = Schedule[idV].size();
        int RouteTime =0;
        RouteTime += t[0][ Schedule[idV][0] ]; // Origin to First

        for(int i=0; i<jobsNb; i++){
            
            int JobId = Schedule[idV][i];

            SizeSum += s[ JobId-1 ];
            accPTime += P[ JobId-1 ];

            int Delivery = accPTime + RouteTime;
            if(Delivery > d[ JobId-1 ]){
                PenaltyCosts += (Delivery - d[ JobId-1 ]) * w[ JobId-1 ];
            }

            if(i < jobsNb-1)
                RouteTime += t[ Schedule[idV][i] ][ Schedule[idV][i+1] ];
        }
        
        if(SizeSum > Q[ idV-1 ])
            OverlapCosts += SizeSum - Q[ idV-1 ];

        RouteTime += t[ Schedule[idV][ jobsNb-1 ] ][ 0 ]; // Last to Origin
        TravelCosts += RouteTime;
    }


    return UseCosts + TravelCosts + PenaltyCosts + OverlapCosts;
}

Solution ag2 (int N, int K, int itNumber, int popSize,
              const vector<int> &P, const vector<int> &d, const vector<int> &s,
              const vector<double> &w, const vector<int> &Q, const vector<int> &F,
              const vector<vector<int>> &t)
{
    vector<Solution> PP(popSize);

    Solution S_best;
    S_best.alocate(N);
    double minValue = numeric_limits<double>::infinity();

    for(Solution &S: PP){
        S.alocate(N);
        randomSolution(S, N, K);
        S.Value = calculateObj(S, N, K, P, d, s, w, Q, F, t);

        if(minValue > S.Value){
            S_best = S;
            minValue = S.Value;
        }
    }

    return S_best;
}


#endif