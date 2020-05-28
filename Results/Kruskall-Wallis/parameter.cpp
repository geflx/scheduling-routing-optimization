#include <bits/stdc++.h>
using namespace std;

int main(){
    int n;
    cin>>n;

    vector<string> param(n);
    for(int i=0; i<n; i++){
        string p;
        cin >> param[i];
    }

    vector<vector<double>> m(n);


    cin.ignore();
    string line;
    while(getline(cin,line)){
        stringstream input(line);
        
        for(int i=0; i<n; i++){
            double v;
            input >> v;
            m[i].push_back(v);
        }
    }

    for(int i=0; i<n; i++){
        for(int j=0; j<m[i].size(); j++){
            cout<<m[i][j]<<" "<<param[i]<<"\n";
        }
    }
}