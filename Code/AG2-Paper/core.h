#ifndef CORE_H
#define CORE_H

using namespace std;

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

#endif