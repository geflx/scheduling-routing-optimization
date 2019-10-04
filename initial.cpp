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

struct vehicleLoaded{
    int id;
    int initialPos;
    int finalPos;
};

double objFunction(const vector<vehicleLoaded> &vehicleOrder, const vector<data> &configuration,int numVeiculos, int numTrabalhos, const vector<vector<int> > &time, 
                const vector<double> &weight,const vector<int> &jobTardiness, const vector<int> &carPrices){

    int travelCosts=0;
    int vehicleCosts=0;
    double penaltyCosts=0;

    //First part of Obj Function, calculating the travel costs between origin->customers1->customers2 etc
    for(int i=0;i<vehicleOrder.size();i++){

        int initialIndex=vehicleOrder[i].initialPos; //index of first job carried by that car.
        int whichJob= configuration[initialIndex].id;
        travelCosts+=time[0][whichJob+1]; // Origin to the first job

        int finalIndex=vehicleOrder[i].finalPos; //index of last job carried by that car.


        for(int j=initialIndex;j<finalIndex;j++){
            int jobA= configuration[j].id;
            int jobB= configuration[j+1].id;
            travelCosts+= time[ jobA+1 ][ jobB+1];
        }

        whichJob= configuration[finalIndex].id;
        travelCosts+= time[ whichJob+1][ 0 ] ; //Last job to origin
    }

    //Second part of Obj Function, looking to the cars' prices
    for(int k=0;k<vehicleOrder.size();k++){

        vehicleCosts+= carPrices[ vehicleOrder[k].id ];
    }

    for(int i=0;i<numTrabalhos;i++){

        penaltyCosts+= jobTardiness[i]* weight[i];
    }

    return (travelCosts+vehicleCosts+penaltyCosts) ;
}

vector<int > calculatingDeliveryTime(const vector<data> &config, vector<int> &startVehicleTime, const vector<vector<int> > &time,const vector<int> &procTime, 
                                    const vector<vehicleLoaded> &vehicleOrder,int numTrabalhos){
    vector<int> deliveryTime(numTrabalhos,0);
    int acumulatedProcessTime=0;

    //Calculating the acumulated processing time for jobs and setting the starting time for vehicles.
    for(int i=0;i<vehicleOrder.size();i++){
        int whichCar=vehicleOrder[i].id; // Setting the delivery times by delivery sequence.
        for(int j=vehicleOrder[i].initialPos;j<=vehicleOrder[i].finalPos;j++){
            int whichJob=config[j].id;
            acumulatedProcessTime += procTime[ whichJob ];
            deliveryTime[ whichJob ] = acumulatedProcessTime;
        }
        startVehicleTime[whichCar]=acumulatedProcessTime;
    }

    //Now calculating the travel time between delivered jobs
    for(int i=0;i<vehicleOrder.size();i++){
        int acumulatedDeliveryTime=0;
        int whichCar=vehicleOrder[i].id; 
        int lastJob;
        for(int j=vehicleOrder[i].initialPos;j<=vehicleOrder[i].finalPos;j++){
            int whichJob=config[j].id;
            if(j==vehicleOrder[i].initialPos){ // Leaving origin to first job
                acumulatedDeliveryTime+= time[ 0 ][ whichJob+1 ];  
            }else{
                acumulatedDeliveryTime+= time[ lastJob+1 ][ whichJob+1];
            }
            deliveryTime[ whichJob ]+=acumulatedDeliveryTime;
            lastJob=whichJob;
        }
    }
    return deliveryTime;
}



vector<vehicleLoaded> generateVehicleOrder(const vector<data> &config,int numVeiculos){
    vector<vehicleLoaded> vehicleList;
    for(int i=0;i<config.size();i++){
        if(!config[i].job){ // We found a car, so lets update the jobs it carries
            vehicleLoaded temp; //Temporary variable
            temp.id=config[i].id;//Which car I found.
            int count = i+1;
            temp.initialPos=count;
            while(count<config.size() && config[count].job){
                count++;
            }
            temp.finalPos=count-1; // Final position in array of config which we found a job for this vehicle.
            if(! (count == (i+1)) ){ //This vehicle doesnt carry any job. Ignore it.
                vehicleList.push_back(temp);
            }
            i=count-1;
        }

    }
    return vehicleList;
}
vector<int> calculatingJobTardiness(const vector<int> &deliveryTime,int numTrabalhos, const vector<int> &dueDate){
    vector<int> T(numTrabalhos);
    for(int i=0;i<numTrabalhos;i++){
        T[i]=deliveryTime[i]-dueDate[i];
    }
    return T;
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

vector<data> generateValidRandomConfig(int numTrabalhos, int numVeiculos, const vector<int> &vehicleCap, const vector<int> &jobSizes){


    vector<data> configuration (numTrabalhos+ (numVeiculos));
    bool validConfig=false;

    while(!validConfig){ 

        validConfig=true;

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

        //Checking if the generated solution is valid. Fastest way to code the check. (O (N+K))
        for(int j=0;j<configuration.size()-1;j++){

            if(!configuration[j].job){ //Checking the car's content.

                int jobSum = 0;
                int contJob = j+1;
                while(configuration[ contJob ].job && contJob < configuration.size() ){

                    jobSum+= jobSizes[ configuration[contJob].id ];
                    contJob++;


                }
                if(  (jobSum > vehicleCap[ configuration[j].id ]) ){ //T Job size sum inside the car isnt valid.
                    
                    validConfig = false;
                }


                j=contJob-1; //Refreshing the index of extern loop.

            }
        }
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
    vector<vector<int> > t(numTrabalhos+1,vector<int>(numTrabalhos+1)); //tempo da viagem
    vector<int> P(numTrabalhos); //tempo de processamento
    vector<double> w(numTrabalhos); //peso da penalidade
    vector<int> d(numTrabalhos); //vencimento
    vector<int> s(numTrabalhos); //tamanho da tarefa j E N
 
    for(int i=0;i<numTrabalhos;i++)
        in>>P[i];
    for(int i=0;i<numTrabalhos;i++)
        in>>d[i];
    for(int i=0;i<numTrabalhos;i++)
        in>>s[i];
 
    for(int i=0;i<numVeiculos;i++)
        in>>Q[i];
    for(int i=0;i<numVeiculos;i++)
        in>>F[i];
    for(int i=0;i<numTrabalhos;i++)
        in>>w[i];
    for(int i=0;i<=numTrabalhos;i++){
        for(int j=0;j<=numTrabalhos;j++){
            in>>t[i][j];
        }
    }
    in.close();
 
    //Generating random solution
    vector<data> initialConfig = generateValidRandomConfig(numTrabalhos,numVeiculos,Q,s);

    //Generating info about vehicle transportation
    vector<vehicleLoaded> vehicleOrder=generateVehicleOrder(initialConfig,numVeiculos);
    
    #ifdef DEBUG_MODE //Testing the generated vector of data
        cout<<"vehicleOrder size: "<<vehicleOrder.size()<<endl;
        for(int i=0;i<vehicleOrder.size();i++){
            cout<<"Vehicle: "<<vehicleOrder[i].id<<" inicia em "<<vehicleOrder[i].initialPos<<" e termina em: "<<vehicleOrder[i].finalPos<<endl;
        }
    #endif
    //Calculating Delivery time(D) and Starting time(Sk) 
    vector<int> startVehicleTime(numVeiculos,0);
    vector<int> deliveryTime = calculatingDeliveryTime(initialConfig,startVehicleTime,t,P,vehicleOrder,numTrabalhos);

    //Generating Job Tardiness (T) of each job (O(N))
    vector<int> jobTardiness = calculatingJobTardiness(deliveryTime,numTrabalhos,d);

    #ifdef DEBUG_MODE //Testing the generated vector of data
        for(int i=0;i<initialConfig.size();i++){
            if(initialConfig[i].job){
                cout<<"Job "<<initialConfig[i].id<<endl;
            }else{
                cout<<"Car "<<initialConfig[i].id<<endl;
            }
        }
    #endif

    #ifdef DEBUG_MODE //Testing the delivery time and the Starting time
        cout<<"Delivery times: ";
        for(int i=0;i<numTrabalhos;i++){
            cout<<deliveryTime[i]<<" ";
        }
        cout<<endl<<"Job Tardiness times: ";
        for(int i=0;i<numTrabalhos;i++){
            cout<<jobTardiness[i]<<" ";
        }
        cout<<endl<<"Start vehicle time:\n";
        for(int j=0;j<numVeiculos;j++){
            if(startVehicleTime[j]!=0){
                cout<<"(V:"<<j<<","<<startVehicleTime[j]<<")\n";
            }else{
                cout<<"(V:"<<j<<",off"<<")\n";

            }
        }
        cout<<endl;

        double resultObjFunction= objFunction(vehicleOrder,initialConfig,numVeiculos,numTrabalhos,t,w,jobTardiness,F);
        cout<<"Objective Function: "<<resultObjFunction<<endl;
    #endif
 

}