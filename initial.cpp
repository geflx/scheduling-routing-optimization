#include <vector>
#include <iostream>
#include <fstream>
#include <stdlib.h>  /* rand(), srand() */
#include <time.h>    /* time() */
#define INF 987654321
using namespace std;
 
#define DEBUG_MODE
struct data{
    bool job; //If it is a job: 1, otherwise it is a Vehicle
    int id=-1;  //Job or vehicle identification
};
double objFunction(const vector<data> &configuration, const vector<vector<int> > &vehicleAdjList, 
                int numVeiculos, int numTrabalhos, const vector<vector<double> > &cost, const vector<double> &weight,
                const vector<int> &deliveryDelay, const vector<int> &carPrices){

    int travelCosts=0;
    int vehicleCosts=0;
    int penaltyCosts=0;


}
vector<int > calculatingDeliveryTime(vector<int> &startVehicleTime, const vector<vector<int> > &time,const vector<int> &procTime, 
                                    const vector<int> &vehicleOrder,int numTrabalhos, const vector<vector<int> > &vehicleAdjList){
    vector<int> deliveryTime(numTrabalhos,0);
    int acumulatedProcessTime=0;

    //Calculating the acumulated processing time for jobs and setting the starting time for vehicles.
    for(int i=0;i<vehicleOrder.size();i++){
        int whichCar=vehicleOrder[i]; // Setting the delivery times by delivery sequence.
        for(int j=0;j<vehicleAdjList[whichCar].size();j++){
            int whichJob=vehicleAdjList[whichCar][j];
            acumulatedProcessTime += procTime[ whichJob+1 ];
            deliveryTime[ whichJob ] = acumulatedProcessTime;
        }
        startVehicleTime[whichCar]=acumulatedProcessTime;
    }

    //Now calculating the travel time between delivered jobs
    for(int i=0;i<vehicleOrder.size();i++){
        int acumulatedDeliveryTime=0;
        int whichCar=vehicleOrder[i]; 
        for(int j=0;j<vehicleAdjList[whichCar].size();j++){
            int whichJob=vehicleAdjList[whichCar][j];
            if(j==0){
                acumulatedDeliveryTime+= time[ 0 ][ whichJob+1 ];  // Leaving origin to first job
            }else{
                int lastJob=vehicleAdjList[whichCar][j-1];

                acumulatedDeliveryTime+= time[ lastJob+1 ][ whichJob];
            }
            deliveryTime[ whichJob ]+=acumulatedDeliveryTime;
        }
    }
    return deliveryTime;
}



vector<vector<int> > generateAdjCarList(const vector<data> &config,int numVeiculos, 
                                        vector<int> &vehicleOrder){
    vector<vector<int> > adjList(numVeiculos);
    for(int i=0;i<config.size();i++){
        if(!config[i].job){ // We found a car, so lets update the jobs it carries
            int count = i+1;
            int whichCar = config[i].id; //Which car I found.
            while(count<config.size() && config[count].job){
                adjList[whichCar].push_back(config[count].id);
                count++;
            }
            if(adjList[whichCar].size()>0){
                vehicleOrder.push_back(whichCar);
            }
            i=count-1;
        }

    }
    return adjList;
}
vector<data> generateRandomConfig(int numTrabalhos, int numVeiculos){
    vector<data> configuration (numTrabalhos+ (numVeiculos));
    int configSize = configuration.size();
    vector<bool> visitedConfig(configSize,false);
    vector<bool> visitedCar(numVeiculos,false);
 
 
    int insertedVehicles=0;
    //Generating vehicles random positions
    while(insertedVehicles<numVeiculos){
        int randomPos= rand()%configSize;
        while(visitedConfig[randomPos]) {
            randomPos=rand()%configSize;
        }
        if(!visitedConfig[0]){ //Putting in the config the first car to be used in position[0] of the vector.
            randomPos= rand()%numVeiculos;
            configuration[0].job=false;
            configuration[0].id=randomPos;
            visitedConfig[0]=true;
            visitedCar[randomPos]=true;
            insertedVehicles++;
        }else{
            int randomVeic= rand()%numVeiculos;
            while(visitedCar[randomVeic]) {
                randomVeic=rand()%numVeiculos;
            }
            configuration[randomPos].job = false; //It is a vehicle, not a job.
            configuration[randomPos].id = randomVeic; //Identification of vehicle.
            visitedCar[randomVeic]=true;
            visitedConfig[randomPos]=true;
            insertedVehicles++;
        }
    }
    //Generating random positions for jobs
    for(int i=0;i<numTrabalhos;i++){
        int randomPos= rand()%configSize;
        while(visitedConfig[randomPos]) 
            randomPos=rand()%configSize;
        configuration[randomPos].job = true; //Yes, it is a job!
        configuration[randomPos].id = i; //Identification of jobs.
        visitedConfig[randomPos]=true;
    }
    return configuration;
}

int main(){
    srand(time(NULL));
    int numTrabalhos;
    int numVeiculos;
    //1- Tratamento de entrada de dados + Inicializacao das matrizes
 
    //1.1- Leitura dos dados (trabalhando com arquivos)
    string fileName;
    float varMi,sigmaUm,sigmaDois;
    int numInstancia;
    cout<<"Input file name + .ext to open: ";
    getline(cin,fileName);
    ifstream in(fileName);
    in>>numInstancia>>varMi>>sigmaDois;
    in>>numTrabalhos;
    in>>numVeiculos;
 
    vector<int> F(numVeiculos); //custo do veiculo
    vector<int> Q(numVeiculos); //capacidade do veiculo
 
    //0- Declaracao de variaveis e vetores constantes
    vector<vector<double> > C(numTrabalhos+1,vector<double>(numTrabalhos+1)); //custo
    vector<vector<int> > t(numTrabalhos+1,vector<int>(numTrabalhos+1)); //tempo da viagem
    vector<int> P(numTrabalhos+1); //tempo de processamento
    vector<double> w(numTrabalhos+1); //peso da penalidade
    vector<int> d(numTrabalhos+1); //vencimento
    vector<int> s(numTrabalhos+1); //tamanho da tarefa j E N
 
    P[0]=0; //Tarefa fantasma nao possui tempo de processamento.
    d[0]=0;
    w[0]=0;
    s[0]=0;
    for(int i=1;i<=numTrabalhos;i++)
        in>>P[i];
    for(int i=1;i<=numTrabalhos;i++)
        in>>d[i];
    for(int i=1;i<=numTrabalhos;i++)
        in>>s[i];
 
    for(int i=0;i<numVeiculos;i++)
        in>>Q[i];
    for(int i=0;i<numVeiculos;i++)
        in>>F[i];
    for(int i=1;i<=numTrabalhos;i++)
        in>>w[i];
    for(int i=0;i<=numTrabalhos;i++){
        for(int j=0;j<=numTrabalhos;j++){
            in>>t[i][j];
            //Cost is equal to time, by now:
            C[i][j]=t[i][j];

        }
    }
    in.close();
 
    //Generating random solution
    vector<data> initialConfig = generateRandomConfig(numTrabalhos,numVeiculos);

    //Generating adjList of jobs that each car carries
    //and getting the vehicle order of leaving origin!
    vector<int> vehicleOrder;
    vector<vector<int> > carMatching=generateAdjCarList(initialConfig,numVeiculos, vehicleOrder);
    
    //Calculating Delivery time(D) and Starting time(Sk)
    vector<int> startVehicleTime(numVeiculos,0);
    vector<int> deliveryTime = calculatingDeliveryTime(startVehicleTime,t,P,vehicleOrder,numTrabalhos,carMatching);

    #ifdef DEBUG_MODE //Testing the generated vector of data
        for(int i=0;i<initialConfig.size();i++){
            if(initialConfig[i].job){
                cout<<"Job "<<initialConfig[i].id<<endl;
            }else{
                cout<<"Car "<<initialConfig[i].id<<endl;
            }
        }
    #endif

     #ifdef DEBUG_MODE //Testing the generated adjList of job assignment to each car
        for(int i=0;i<carMatching.size();i++){
            if(carMatching[i].size()>0){
                cout<<"Vehicle "<<i<<": ";
            }
            for(int j=0;j<carMatching[i].size();j++){
                cout<<carMatching[i][j]<<" ";
            }
            cout<<endl;
        }
    #endif

    #ifdef DEBUG_MODE //Testing the delivery time and the Starting time
        cout<<"Delivery times: ";
        for(int i=0;i<numTrabalhos;i++){
            cout<<deliveryTime[i]<<" ";
        }
        cout<<endl<<"Start vehicle time: ";
        for(int j=0;j<numVeiculos;j++){
            cout<<startVehicleTime[j]<< " ";
        }
        cout<<endl;
    #endif
 

}