#include <bits/stdc++.h>
using namespace std;

int main(){

    for(int i=0; i<5; i++){
        ifstream input;
        ofstream out("fileOut.txt", ios::app);
        input.open("in");
        int x;
        while(input >> x){
            out << x << "\n";
        }
        out << "\n";
        out.close();
        input.close();
    }
}