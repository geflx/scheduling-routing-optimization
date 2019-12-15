#include<bits/stdc++.h>
using namespace std;
int main(){
	int num;
	cin>>num;

	set<pair<int,int> > ja;

	while(num--){
		int a= rand()%10000;
		int b= rand()%10000;
		pair<int,int> testeA = make_pair(a,b);
		pair<int,int> testeB = make_pair(b,a);
		bool consigo=false;
		if( ja.find(testeA)==ja.end()){
			consigo=true;
			ja.insert(testeA);
			cout<<a<<" "<<b<<endl;
		}else if (ja.find(testeB)==ja.end()){
			consigo=true;
			ja.insert(testeB);
			cout<<a<<" "<<a<<endl;
		}
		if(!consigo)
			num++;
	}
}