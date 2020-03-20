#include <bits/stdc++.h>
#include <stdlib.h>
#include <time.h>
using namespace std;
// #define DEBUG_MODE
/*
Instancias sao da forma:
numInstancia(1 a 5) mi delta
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
void generateTime(vector<vector<int> > &tij,int numTrabalhos,int numVeiculos,int rho){
	vector<pair<int,int> > points(numTrabalhos+1);
	points[0].first= rand()%300 + 400; //Definindo a origem (x,y)
	points[0].second=rand()%300 + 400;
	for(int i=1;i<=numTrabalhos;i++){
		int x,y;
		while(true){
			x=rand()%1000;
			y=rand()%1000;
			if(x<0||y<0||(dist(points[0].first,points[0].second,x,y)>rho*numVeiculos)){
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
	vector<int> k{3,4,5,6};
	vector<int> instancia{1,2,3,4,5};
	vector<float> mi{1.0,1.5,2.0}; //u
	vector<float> delta{0.5,1.0,1.5,2.0,2.5};
	int rho=100;

	int cont=0;
	int contFiles=0;

	for(int numTrabalhos:n){
		string printFile= to_string(contFiles)+".txt";
		ofstream outFile;
		outFile.open(printFile);
		++contFiles;
		for(int numVeiculos:k){
			for(float varMi:mi){
				for(float deltaVar:delta){
					for(int inst:instancia){
						outFile<<inst<<" "<<varMi<<" "<<deltaVar<<endl;
						outFile<<numTrabalhos<<endl; //1- numTrabalhos
						outFile<<numVeiculos<<endl; //2- numVeiculos
						vector<int> processingTime(numTrabalhos+1);
						vector<int> jobSizes(numTrabalhos+1);
						processingTime[0]=0;
						jobSizes[0]=0;
						for(int i=1;i<=numTrabalhos;i++){ //3- Processing time!
							
							processingTime[i]=rand()%99+1;
							outFile<<processingTime[i];
							if(i!=numTrabalhos) outFile<<" ";

						}
						outFile<<endl;

						for(int i=1;i<=numTrabalhos;i++){ //CALCULATING - Job sizes! (s) 
							jobSizes[i]=rand()%processingTime[i] +1;
						}

						vector<vector<int> > tij(numTrabalhos+1,vector<int>(numTrabalhos+1,0)); //CALCULATING - time!
						generateTime(tij,numTrabalhos,numVeiculos,rho);
						int winf,wsup;
						vector<int> dueDate(numTrabalhos+1); // 4- Due Date!

						int somatorioP=0; //Somatorio de processing time
						for(int i=1;i<=numTrabalhos;i++){
							somatorioP+=processingTime[i];
						}
						int limiteSuperior=somatorioP/(numVeiculos+1);

						for(int i=1;i<=numTrabalhos;i++){
							
							int pi= rand()% limiteSuperior;

							winf=processingTime[i]+tij[0][i]+pi;

							int limiteSuperiorWsup=ceil(deltaVar*rho);

							int ki=rand()% limiteSuperiorWsup;
							wsup= winf+ki;

							int intervalValue= wsup-winf;
							int randNumber=0;
							if(intervalValue>0){
								randNumber=rand()%intervalValue;
							}
							dueDate[i]= randNumber + winf ;  // dueDate[i] pertence ao intervalo [winf, wsup]
							
							outFile<<dueDate[i];
							if(i!=numTrabalhos) outFile<<" ";
						}
						outFile<<endl;

						for(int i=1;i<=numTrabalhos;i++){ //5- printing - Job sizes! (s) 
							outFile<<jobSizes[i];

							if(i!=numTrabalhos) outFile<<" ";
						}
						outFile<<endl;
						


						
						vector<int> carCap(numVeiculos); //6- car capacity! Gera enquanto as tarefas não caberem nos veiculos.
						int maxJobSize=-1;
						int jobSizeSum=0;	
						for(int i=1;i<=numTrabalhos;i++){
							maxJobSize=max(maxJobSize,jobSizes[i]);
							jobSizeSum+=jobSizes[i];
						}
						bool precisaGerar=true;
						while(precisaGerar){
							int carCapSum=0;

							for(int i=0;i<numVeiculos;i++){ 

								int tetaMin= ceil( jobSizeSum/numTrabalhos*(1.0));
								int tetaMax= ceil( jobSizeSum/numVeiculos*(1.0));
								tetaMax*= varMi;

								carCap[i]=maxJobSize+ rand()%(tetaMax-tetaMin) +tetaMin;
								carCapSum+=carCap[i];
							}
							if(carCapSum < jobSizeSum){ //Gere novamente
								precisaGerar=true;
							}else{
								precisaGerar=false;
							}

						}
						for(int i=0;i<numVeiculos;i++){ //Printing valid capacities...
							outFile<<carCap[i];
							if(i!=numVeiculos-1) outFile<<" ";
						}
						outFile<<endl;

						


						for(int i=0;i<numVeiculos;i++){ //6- car prices!
							outFile<< carCap[i] * numTrabalhos;
							if(i!=numVeiculos-1) outFile<<" ";
						}
						outFile<<endl;
						for(int i=0;i<numTrabalhos;i++){  //8- penalty weight!
							float penalty=(rand()%41);
							penalty=penalty/10.0 +1.0;
							outFile<<penalty;
							if(i!=numTrabalhos-1) outFile<<" ";
						}
						outFile<<endl;

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
		outFile.close(); //Dividindo em arquivos pelo numero de trabalhos.

	}
}