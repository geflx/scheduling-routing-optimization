#include <bits/stdc++.h>
using namespace std;

int main(){

    string total;
    cin>>total;

    string crib;
    cin>>crib;

    map<char,unordered_set<int>> m;
    for(int i=0; i<crib.size(); i++){
        m[ crib[i] ].insert(i);
    }

    int possible = 0;
    for(int i=0; i<total.size()-crib.size()+1; i++){

        bool can=true;
        int posi = 0;

        // cout<<"mstest:";
        // for(int k=i;k<i+crib.size();k++){
        //     cout<<total[k];
        // }cout<<endl;

        for(int j=i;j<i+crib.size();j++){

            if( m[total[j]].find(posi) != m[total[j]].end()){
                can=false;
                break;
            }
            ++posi;
        }
        // cout<<"result:"<<can<<endl;
        if( can ){
            ++possible;
        }
    }
    cout<<possible<<endl;
}
