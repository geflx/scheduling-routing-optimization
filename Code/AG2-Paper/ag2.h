#ifndef AG2_H
#define AG2_H

#include <limits>
#include <iostream>
#include <vector>
#include <algorithm>

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

double calculateObj(int N, int K, const vector<int> &P, const vector<int> &d, const vector<int> &s,
                    const vector<double> &w, const vector<int> &Q, const vector<int> &F,
                    const vector<vector<int>> &t)
{
    double Ans;
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
        S.Value = calculateObj(N, K, P, d, s, w, Q, F, t);

        if(minValue > S.Value){
            S_best = S;
            minValue = S.Value;
        }
    }


    return S_best;
}


#endif