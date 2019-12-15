#include <bits/stdc++.h>
using namespace std;
#define INF 987654
struct Ponto{
	int x,y;
}; 

double distEuclidiana(const Ponto &a, const Ponto &b){
	return sqrt((a.x-b.x)*(a.x-b.x) + (a.y-b.y)*(a.y-b.y));
}
void calculaDistancia( vector<vector<double>> &time, int numPontos, const vector<Ponto> &mapa){
	for(int i=0; i<numPontos; i++){
		for(int j=i; j<numPontos; j++){
			double calc= distEuclidiana(mapa[i],mapa[j]);
			time[i][j] = calc;
			time[j][i] = calc;

		}
	}
}
void gulosoTSP(vector<int> &caminho, int numPontos, const vector<vector<double>> &time){
	
	vector<bool> visited(numPontos,false);

	visited[0] = true; //A origem esta implicita

	int numInseridos = 1;

	while(numInseridos < numPontos ){

		double min= INF;
		double idMin = -1;

		for(int i=1; i<numPontos; i++){

			if( !visited[i] && time[ numInseridos-1][i] < min){
				
				min= time[ numInseridos-1][i];
				idMin=i;
			}
		}

		caminho[ numInseridos ] = idMin;
		visited[ idMin ] = true;
		numInseridos++;
	}
}

double objValueTSP( vector<int> &caminho, int numPontos, const vector<vector<double>> &time ){
	double total=0;

	for(int i=0; i<numPontos-1; i++){

		total += time[ caminho[i] ][ caminho [i+1] ];

	}
	total += time[ 0 ][ caminho[0] ]; //Origem ao primeiro

	total += time[ caminho[numPontos-1] ][ 0 ]; //Ultimo a origem
	return total;
}

//atencao na copia do set aqui em baixo
void improvePath( double &currObjValue, vector<int> &caminho, int numPontos, set<int> restricted,  
				  int depth, int alpha, const vector<vector<double>> &time ){

	if(depth == numPontos)
		return;
	if( depth < alpha){
		cout<<"depth "<<depth<<endl;
		for(int i=0; i<numPontos-1; i++){
			if( restricted.find(i)== restricted.end()){

				//Vertice ainda nao foi alterado

				double g= time[ caminho[i]][caminho[i+1]] -time[ caminho[i]][ caminho[numPontos-1]];
				

				if (g>0){ //Parece interessante
					cout<<" swapei"<<endl;
					cout<<caminho[i]<<" "<<caminho[numPontos-1]<<endl;
					swap(caminho[i],caminho[numPontos-1]);
					cout<<caminho[i]<<" "<<caminho[numPontos-1]<<endl;

					double newObjValue = objValueTSP(caminho, numPontos, time);

					if(newObjValue < currObjValue){
						//improve
						cout<<"De "<<currObjValue<<" para "<<newObjValue<<endl;

						currObjValue = newObjValue;
						cout<<"retornei\n";
						return;
					}else{
						// swap(caminho[i],caminho[numPontos-1]); Nao destroca!
						restricted.insert(i);
						cout<<"Inseri "<<i<<" na restricao\n";
						cout<<"Nao melhorei tenta depth+1\n";

						improvePath( currObjValue, caminho, numPontos, restricted, depth+1, alpha,time );
					}
				}
			}
		}

	}else{
		bool aindaTemMelhoria=false;

		pair<int,int> best;
		double bestImpObjValue=-1;
		for(int i=0; i<numPontos-1 ; i++){
			//Vertice ainda nao foi alterado
			// cout<<"i: "<<i<<" e i+1: "<<i+1<<endl;
			double g= time[ caminho[i]][caminho[i+1]] -time[ caminho[i]][ caminho[numPontos-1]];
			

			if (g>0){ //Parece interessante
				aindaTemMelhoria=true;

				swap(caminho[i],caminho[numPontos-1]);

				double newObjValue = objValueTSP(caminho, numPontos, time);

				if(newObjValue < currObjValue){
					//improve
					aindaTemMelhoria  = true;
					bestImpObjValue = newObjValue;
					best = make_pair(i, numPontos-1);
					cout<<"De "<<currObjValue<<" para "<<newObjValue<<endl;
					currObjValue = newObjValue;
					return;
				}
				swap(caminho[i],caminho[numPontos-1]);

			}
		}

		if(!aindaTemMelhoria){
			cout<<"retornei\n";
			return;
		}else{
			swap(caminho[best.first], caminho[best.second]);
			improvePath( bestImpObjValue , caminho, numPontos, restricted, depth+1, alpha,time );
		}
	}
}

int main(){
	int numPontos;
	cin>>numPontos;
	vector<Ponto> mapa(numPontos);

	for(int i=0; i<numPontos; i++){
		cin>>mapa[i].x>>mapa[i].y;
	}

	vector<vector<double>> time(numPontos,vector<double>(numPontos,0.0));
	calculaDistancia (time, numPontos, mapa);
	vector<int> caminho(numPontos-1,0);
	gulosoTSP( caminho, numPontos, time );

	double objValue= objValueTSP(caminho, numPontos, time);
	set<int> restricted;

	cout<<"F. Obj Guloso: "<<objValue<<endl;

	int ite = 10;
	while(ite--){
		improvePath(objValue, caminho, numPontos, restricted, 0, 3, time);
	}

	cout<<"F nova "<<objValue<<endl;
}