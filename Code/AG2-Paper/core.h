#ifndef CORE_H
#define CORE_H

#include <fstream>

void readInstance(ifstream &input, double &mi, double &delta, int &N, int &K,
                  vector<int> &P, vector<int> &d, vector<int> &s,
                  vector<double> &w, vector<int> &Q, vector<int> &F,
                  vector<vector<int>> &t)
{
    input >> mi >> delta;
    
    input >> N >> K;

    for(int i=0; i<N; i++) input >> P[i];
    for(int i=0; i<N; i++) input >> d[i];
    for(int i=0; i<N; i++) input >> s[i];
    for(int i=0; i<N; i++) input >> w[i];

    for(int i=0; i<K; i++) input >> Q[i];
    for(int i=0; i<K; i++) input >> F[i];

    for(int i=0; i<= N; i++)
        for(int j=0; i<=N; i++)
            input >> t[i][j]; 
}

#endif