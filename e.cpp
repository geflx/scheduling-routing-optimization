#include <bits/stdc++.h>
using namespace std;

int main(){

    set<int> s;
    int n, k;
    cin >> n >> k;

    map<int,int> m;
    for(int i=0; i<n; i++){
        int oldsz = s.size();
        int temp; cin >> temp;
        s.insert(temp);
        int newsz = s.size();
        if( newsz != oldsz ){
            m[newsz] = i+1;
        }
    }
    if(m[k] != 0 ){
        cout<<1<<" "<<m[k]<<endl;
    }
}