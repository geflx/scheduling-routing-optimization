#include <bits/stdc++.h>
using namespace std;

typedef long long ll;

bool ok(string a){

    set<char> s;
    for(int i=0; i<a.size(); i++){
        s.insert(a[i]);
    }
    return s.size() == 4;
}


int main(){
    int a;
    cin >> a;
    int b = a+1;

    while( true ){

        string c = to_string(b);
        if(ok (c)){
            break;
        }else{
            b++;
        }
    }
    cout << b << endl;
    
}