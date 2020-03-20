#include <bits/stdc++.h>
#include <stdlib.h>
#include <time.h>
using namespace std;
// #define DEBUG_MODE
/*
Instancias sao da forma:
numInstancia(1 a 5) mi sigmaUm sigmaDois
n(numTrabalhos): 7
k(numVeiculos): 2
P(processing time): 48 30 .. 46 (numTrabalhos) (float)
d (due date): 140 130 .. 100
s(tam da tarefa): 32 15 7 .. 45 (numTrabalhos)
Q (cap veiculos): 77 52 (numVeiculos)
F (custo veiculos): 144 104 .. (numVeiculos)
wi (penalidade): 2 5 1 3 .. 1 (numTrabalhos)
tij: matriz simetrica de distancias, diagonal principal vale zero.


*/
double dist(int x1,int y1,int x2,int y2){
	return sqrt(((x1-x2)*(x1-x2))+((y1-y2)*(y1-y2)));
}
void generateTime(vector<vector<int> > &tij,int numTrabalhos,int numVeiculos,int varRo){
	vector<pair<int,int> > points(numTrabalhos+1);
	points[0].first= rand()%300 + 400; //Definindo a origem (x,y)
	points[0].second=rand()%300 + 400;
	for(int i=1;i<=numTrabalhos;i++){
		int x,y;
		while(true){
			x=rand()%1000;
			y=rand()%1000;
			if(dist(points[0].first,points[0].second,x,y)>min(points[0].first,points[0].second) 
			|| (dist(points[0].first,points[0].second,x,y)>varRo*numVeiculos)){
				//Forcando estar no primeiro quadrante
				continue;
			}else{
				break;
			}
		}
		points[i].first=x;
		points[i].second=y;
	}
	for(int i=0;i<=numTrabalhos;i++){
		for(int j=0;j<=numTrabalhos;j++){
			tij[i][j]=dist(points[i].first,points[i].second,points[j].first,points[j].second);
		}
	}
	return;
}

int main(){
	srand(time(NULL));
	vector<int> n{8,10,15,20};
	vector<int> k{2,4,5};
	vector<int> instancia{1,2,3,4,5};
	vector<float> mi{1,1.5,2}; //u
	vector<float> sig1{0.3,0.5,1};
	vector<float> sig2{0.5,1};
	vector<int> ro{100}; //p
	int cont=0;
	int contFiles=0;

	int constanteVeiculo=2;

	for(int numTrabalhos:n){
		string printFile= to_string(contFiles)+".txt";
		ofstream outFile;
		outFile.open(printFile);
		++contFiles;
		for(int numVeiculos:k){
			for(float varMi:mi){
				for(float sigUm:sig1){
					for(float sigDois:sig2){
						for(int varRo:ro){
							for(int inst:instancia){
								outFile<<inst<<" "<<varMi<<" "<<sigUm<<" "<<sigDois<<" "<<constanteVeiculo<<endl;
								outFile<<numTrabalhos<<endl; //1- numTrabalhos
								outFile<<numVeiculos<<endl; //2- numVeiculos
								vector<int> processingTime(numTrabalhos+1);
								vector<int> jobSizes(numTrabalhos+1);
								processingTime[0]=0;
								jobSizes[0]=0;
								for(int i=1;i<=numTrabalhos;i++){ //3- Processing time!
									#ifdef DEBUG_MODE
									outFile<<"c1"<<endl;
									#endif
									processingTime[i]=rand()%99+1;
									outFile<<processingTime[i];
									if(i!=numTrabalhos) outFile<<" ";
								}
								outFile<<endl;
								#ifdef DEBUG_MODE
								outFile<<"c2"<<endl;
								#endif
								for(int i=1;i<=numTrabalhos;i++){ //CALCULATING - Job sizes! (s) 
									jobSizes[i]=rand()%processingTime[i] +1;
								}

								vector<vector<int> > tij(numTrabalhos+1,vector<int>(numTrabalhos+1,0)); //CALCULATING - time!
								generateTime(tij,numTrabalhos,numVeiculos,varRo);
								int winf,wsup;
								vector<int> dueDate(numTrabalhos+1); // 4- Due Date!
								// int dueDate[numTrabalhos];
								for(int i=1;i<=numTrabalhos;i++){
									int limiteSuperior=ceil(sigUm*varRo*numTrabalhos/(1.0+numVeiculos));
									
									#ifdef DEBUG_MODE
									outFile<<"c3"<<endl;
									outFile<<"sigUm eh: "<<sigUm<<" e numTrabalhos/veiculos: "<<numTrabalhos<<" "<<numVeiculos<<endl;
									outFile<<"limiteSuperior eh: "<<limiteSuperior<<endl;
									#endif
									int pi= rand()% limiteSuperior;

									winf=processingTime[i]+jobSizes[i]+tij[0][i]+pi;
									limiteSuperior=ceil(sigDois*varRo);
									int ki=rand()% limiteSuperior;
									wsup= winf+ki;

									dueDate[i]= rand()%winf + (wsup-winf);  // dueDate[i] pertence ao intervalo [winf, wsup]
									outFile<<dueDate[i];
									if(i!=numTrabalhos) outFile<<" ";
								}
								outFile<<endl;

								for(int i=1;i<=numTrabalhos;i++){ //5- printing - Job sizes! (s) 
									outFile<<jobSizes[i];

									if(i!=numTrabalhos) outFile<<" ";
								}
								outFile<<endl;

								#ifdef DEBUG_MODE
								outFile<<"c4"<<endl;
								#endif
								
								vector<int> carCap(numVeiculos);
								int maxJobSize=-1;
								int jobSizeSum=0;
								for(int i=0;i<=numTrabalhos;i++){
									maxJobSize=max(maxJobSize,jobSizes[i]);
									jobSizeSum+=jobSizes[i];
								}
								for(int i=0;i<numVeiculos;i++){ //6- car capacity!
									int tetaMin= ceil( jobSizeSum/numVeiculos*(1.0));
									int C=tetaMin;
									tetaMin-=maxJobSize;
									int tetaMax= varMi*C;
									carCap[i]=maxJobSize+ rand()%tetaMax+tetaMin;
									outFile<<carCap[i];
									if(i!=numVeiculos-1) outFile<<" ";
								}
								outFile<<endl;

								#ifdef DEBUG_MODE
								outFile<<"c5"<<endl;
								#endif

								for(int i=0;i<numVeiculos;i++){ //6- car prices!
									outFile<<carCap[i]*constanteVeiculo;
									if(i!=numVeiculos-1) outFile<<" ";
								}
								outFile<<endl;
								for(int i=0;i<numTrabalhos;i++){  //8- penalty weight!
									float penalty=rand()%99+1;
									penalty=penalty/10.0;
									outFile<<penalty;
									if(i!=numTrabalhos-1) outFile<<" ";
								}
								outFile<<endl;
								#ifdef DEBUG_MODE
								outFile<<"c5"<<endl;
								#endif

								for(int i=0;i<=numTrabalhos;i++){ //8 printing - Time![i][j] (s) 
									for(int j=0;j<=numTrabalhos;j++){
										outFile<<tij[i][j];
										if(j!=numTrabalhos) outFile<<" ";
									}
									outFile<<endl;
								}
								outFile<<endl;
							}
						}
					}
				}
			}
		}
		outFile.close(); //Dividindo em arquivos pelo numero de trabalhos.

	}
}