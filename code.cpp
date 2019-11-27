#include <vector>
#include <iostream>
#include <queue>
#include <set>
#include <unordered_set>
#include <fstream>
#include <cmath> 
#include <iomanip>
#include <stdlib.h>  /* rand(), srand() */
#include <time.h>    /* time() */
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <algorithm>

// #define VIEW_OBJ_DECREASE
// #define VIEW_DECREASE
// #define PRINT_VND_IMPROVE
// #define DEBUG_MODE


#define INF 987654321
using namespace std;
  
 
struct data{
    bool job; //If it is a job: 1, otherwise it is a Vehicle
    int id;  //Job or vehicle identification
    data(){
        job=true; //By default, the position is a job.
        id=-1;
    }
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

  //   cout<<"Job Tardiness: "<<endl;
  //   for(const int &u: jobTardiness){
  //   	cout<<u<<" ";
  //   }cout<<endl;
 	// cout<<"Penalty Weight: "<<endl;
 	// for(const double &u:weight){
 	// 	cout<<u<<" ";
 	// }cout<<endl;

 	// cout<<"Obj. Travel costs: "<<travelCosts<<endl;
 	// cout<<"Obj. Vehicle costs: "<<vehicleCosts<<endl;
 	// cout<<"Obj. Penalty costs: "<<penaltyCosts<<endl;
    return (travelCosts+vehicleCosts+penaltyCosts) ;
}
 
double objFVehicle(int i, const vector<vehicleLoaded> &vehicleOrder, const vector<data> &configuration,int numVeiculos, int numTrabalhos, const vector<vector<int> > &time, 
    //To use this function, you first need to recalculate the deliveryTime and JobTardiness

    const vector<double> &weight,const vector<int> &jobTardiness, const vector<int> &carPrices){
 
    int travelCosts=0;
    int vehicleCosts=0;
    double penaltyCosts=0;
 
    //First part of Obj Function, calculating the travel costs between origin->customers1->customers2 etc
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
    
 
    //Second part of Obj Function, looking to the cars' prices
 
    vehicleCosts+= carPrices[ vehicleOrder[i].id ];
 
    for(int j=initialIndex;j<=finalIndex;j++){
        int job= configuration[j].id;
        penaltyCosts+= jobTardiness[job]* weight[job];
    }
 
 	// cout<<"Obj. Travel costs: "<<travelCosts<<endl;
 	// cout<<"Obj. Vehicle costs: "<<vehicleCosts<<endl;
 	// cout<<"Obj. Penalty costs: "<<penaltyCosts<<endl;
    return (travelCosts+vehicleCosts+penaltyCosts) ;
}

vector<int > calculatingDeliveryTime(const vector<data> &config, vector<int> &startVehicleTime, const vector<vector<int> > &time,const vector<int> &procTime, 
    const vector<vehicleLoaded> &vehicleOrder,int numTrabalhos){
    vector<int> deliveryTime(numTrabalhos,0);
    int acumulatedProcessTime=0;
 
    // for(int i=0;i<vehicleOrder.size();i++){
    // 	cout<<"Vehicle "<<vehicleOrder[i].id<<" carrega: ";

    // 	for(int j=vehicleOrder[i].initialPos;j<=vehicleOrder[i].finalPos;j++){
    // 		cout<<config[ j ].id<<" ";
    // 	}
    // 	cout<<endl;
    // }
    // cout<<"Processing time e: ";
    // for(int u: procTime){
    // 	cout<<u<<" ";
    // }cout<<endl;


    //Calculating the acumulated processing time for jobs and setting the starting time for vehicles.
    for(int i=0;i<vehicleOrder.size();i++){
        int whichCar=vehicleOrder[i].id; // Setting the delivery times by delivery sequence.
        for(int j=vehicleOrder[i].initialPos;j<=vehicleOrder[i].finalPos;j++){
            int whichJob=config[j].id;

            acumulatedProcessTime += procTime[ whichJob ];
        }
        startVehicleTime[whichCar]=acumulatedProcessTime;
    

    //Now calculating the travel time between delivered jobs
        int acumulatedDeliveryTime=0;
        int lastJob;
        // Leaving origin to first job
        acumulatedDeliveryTime = time[ 0 ][  config[vehicleOrder[i].initialPos].id +1];

        for(int j=vehicleOrder[i].initialPos;j<vehicleOrder[i].finalPos;j++){
            int whichJob=config[j].id;
            int nextJob=config[j+1].id;

            deliveryTime[ whichJob ]=acumulatedDeliveryTime + acumulatedProcessTime;
            acumulatedDeliveryTime+= time[ whichJob+1 ][ nextJob+1];
            
        }

        deliveryTime[ config[vehicleOrder[i].finalPos].id  ] = acumulatedProcessTime + acumulatedDeliveryTime;

    }

    // cout<<"Delivery time foi: ";
    // for(int u: deliveryTime){
    // 	cout<<u<<" ";
    // }cout<<endl;

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

    	if(T[i]<0)
    		T[i]=0;
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
                while(contJob < configuration.size() && configuration[ contJob ].job ){
 
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
 
vector<data> generateATC( const vector<int> &jobSize, int numTrabalhos, int numVeiculos, const vector<double> &weight, 
    const vector<int> &procTime, const vector<int> &dueDate, const vector<int> &carPrices, const vector<int> &carCap) {
    //Calculating processing time average
    double procTimeAvg=0;
    bool success= true; // If we generate a valid configuration
 
    for(int i=0;i<numTrabalhos;i++){
     
        procTimeAvg+=procTime[i];
     
    }
    procTimeAvg /= numTrabalhos;
 
    //Variables to generate processing order of jobs:
    vector<data> jobOrder(numTrabalhos);
    vector<bool> visitedJob(numTrabalhos,false);
    int acumulatedD=0;
 
    // cout<<"numtrbs"<<numTrabalhos<<endl;
    for(int i=0;i<numTrabalhos;i++){
 
        //Create priority queue of Dispatching Rule (ATC)
        priority_queue<pair<double, int> > atc;
 
        for(int j=0;j<numTrabalhos;j++){
 
            if(visitedJob[j])
                continue;
 
            double atcValue= (weight[j]/procTime[j]) * exp( (-1) * (max( dueDate[j]- procTime[j]- acumulatedD ,0 ))/ procTimeAvg );
            atc.push( make_pair( atcValue,j) );
 
        }
 
        jobOrder[i].id = ( atc.top() ).second;
        int jobInserted= jobOrder[i].id;
        // cout<<"inseri "<<jobInserted<<endl;
 
        visitedJob[ jobInserted ] = true;
 
        acumulatedD+= procTime[ jobInserted ];
         
    }
 
    // Puting in a greedy way the cars to carry the jobs, choosing the cheaper one.
    vector<data> vehicleDispatching(numVeiculos);
         
    priority_queue<pair<int,int> > vehiclePrice; //We will select the cars in the cheapest order.
 
    for(int i=0; i<numVeiculos; i++){
 
        vehiclePrice.push( make_pair( (-1) * carPrices[i], i ) ); //Ascending order ( *(-1) )
 
    }
 
    vector<data> configuration( numTrabalhos + numVeiculos );
    int configSize = configuration.size();
    // cout<<"configsize "<<configSize<<endl;
    vector<bool> visitedConfig(configSize,false);
 
    int insertedItems = 0;
    int jobToInsert=0;
 
    while(insertedItems<configSize){
 
        int currentCar = (vehiclePrice.top()).second;
        configuration[ insertedItems ].job = false;
        configuration[ insertedItems ].id = currentCar;
        vehiclePrice.pop();
        insertedItems++;
 
        if(insertedItems >= configSize)
            break;
 
        int currentVolume=0;
        while(insertedItems < configSize && jobToInsert<=numTrabalhos && (currentVolume + jobSize[ jobOrder[jobToInsert].id ] <= carCap[ currentCar ]) ){
 
 
            configuration[ insertedItems ].job = true;
            configuration[ insertedItems ].id = jobOrder[ jobToInsert ].id;
 
            currentVolume+= jobSize [  jobOrder[jobToInsert].id ];
            jobToInsert++;
            insertedItems++;
            // cout<<"jobToInsert "<<jobToInsert-1<<endl;
            // cout<<"isnerted itens "<<insertedItems<<endl;
            // cout<<"capacidade atual"<<currentVolume<<endl;
            // cout<<"total: "<<carCap[ currentCar]<<endl;
            // cout<<"vou inserir: "<< jobSize[ jobOrder[jobToInsert].id ]<<" do trabalho: "<<jobOrder[jobToInsert].id<<endl;
        }
        // cout<<"deibreak\n";
 
    }
 
    return configuration;
}
 
vector<data> generateWMDD( const vector<int> &jobSize, int numTrabalhos, int numVeiculos, const vector<double> &weight, 
    const vector<int> &procTime, const vector<int> &dueDate, const vector<int> &carPrices, const vector<int> &carCap) {
 
    //Variables to generate processing order of jobs:
    vector<data> jobOrder(numTrabalhos);
    vector<bool> visitedJob(numTrabalhos,false);
    int acumulatedD=0;
 
    for(int i=0;i<numTrabalhos;i++){
 
        //Create priority queue of Dispatching Rule (WMDD)
        priority_queue<pair<double, int> > wmdd;
 
        for(int j=0;j<numTrabalhos;j++){
 
            if(visitedJob[j])
                continue;
 
            double wmddValue=  (1 /weight[j])* max(procTime[j], dueDate[j] - acumulatedD) ;
            wmdd.push( make_pair( (-1) * wmddValue,j) );
 
        }
 
        jobOrder[i].id = ( wmdd.top() ).second;
        int jobInserted= jobOrder[i].id;
        visitedJob[ jobInserted ] = true;
 
        acumulatedD+= procTime[ jobInserted ];
         
    }
 
    // Puting in a greedy way the cars to carry the jobs, choosing the cheaper one.
    vector<data> vehicleDispatching(numVeiculos);
         
    priority_queue<pair<int,int> > vehiclePrice; //We will select the cars in the cheapest order.
 
    for(int i=0; i<numVeiculos; i++){
 
        vehiclePrice.push( make_pair( (-1) * carPrices[i], i ) ); //Ascending order ( *(-1) )
 
    }
 
    vector<data> configuration( numTrabalhos + numVeiculos );
    int configSize = configuration.size();
    vector<bool> visitedConfig(configSize,false);
 
    int insertedItems = 0;
    int jobToInsert=0;
 
    while(insertedItems<configSize){
 
        int currentCar = (vehiclePrice.top()).second;
        configuration[ insertedItems ].job = false;
        configuration[ insertedItems ].id = currentCar;
        vehiclePrice.pop();
        insertedItems++;
 
        if(insertedItems >= configSize)
            break;
 
        int currentVolume=0;
         while(insertedItems < configSize && jobToInsert<numTrabalhos && (currentVolume + jobSize[ jobOrder[jobToInsert].id ] <= carCap[ currentCar ]) ){
 
 
            configuration[ insertedItems ].job = true;
            configuration[ insertedItems ].id = jobOrder[ jobToInsert ].id;
            jobToInsert++;
            insertedItems++;
 
            currentVolume+= jobSize [  jobOrder[jobToInsert-1].id ];
        }
 
    }
 
    return configuration;
}
 
vector<data> generateWEDD( const vector<int> &jobSize, int numTrabalhos, int numVeiculos, const vector<double> &weight, 
    const vector<int> &procTime, const vector<int> &dueDate, const vector<int> &carPrices, const vector<int> &carCap) {
 
    //Variables to generate processing order of jobs:
    vector<data> jobOrder(numTrabalhos);
    vector<bool> visitedJob(numTrabalhos,false);
 
    for(int i=0;i<numTrabalhos;i++){
 
        //Create priority queue of Dispatching Rule (WEDD)
        priority_queue<pair<double, int> > wedd;
 
        for(int j=0;j<numTrabalhos;j++){
 
            if(visitedJob[j])
                continue;
 
            double weddValue=  ( dueDate[j]/weight[j]) ;
            wedd.push( make_pair( (-1) * weddValue,j) );
 
        }
 
        jobOrder[i].id = ( wedd.top() ).second;
        int jobInserted= jobOrder[i].id;
        visitedJob[ jobInserted ] = true;
         
    }
 
    // Puting in a greedy way the cars to carry the jobs, choosing the cheaper one.
    vector<data> vehicleDispatching(numVeiculos);
         
    priority_queue<pair<int,int> > vehiclePrice; //We will select the cars in the cheapest order.
 
    for(int i=0; i<numVeiculos; i++){
 
        vehiclePrice.push( make_pair( (-1) * carPrices[i], i ) ); //Ascending order ( *(-1) )
 
    }
 
    vector<data> configuration( numTrabalhos + numVeiculos );
    int configSize = configuration.size();
    vector<bool> visitedConfig(configSize,false);
 
    int insertedItems = 0;
    int jobToInsert=0;
 
    while(insertedItems<configSize){
 
        int currentCar = (vehiclePrice.top()).second;
        configuration[ insertedItems ].job = false;
        configuration[ insertedItems ].id = currentCar;
        vehiclePrice.pop();
        insertedItems++;
 
        if(insertedItems >= configSize)
            break;
 
        int currentVolume=0;
         while(insertedItems < configSize && jobToInsert<numTrabalhos && (currentVolume + jobSize[ jobOrder[jobToInsert].id ] <= carCap[ currentCar ]) ){
 
 
            configuration[ insertedItems ].job = true;
            configuration[ insertedItems ].id = jobOrder[ jobToInsert ].id;
            jobToInsert++;
            insertedItems++;
 
            currentVolume+= jobSize [  jobOrder[jobToInsert-1].id ];
        }
 
    }
 
    return configuration;
}
 
void printConfig( double fObj, const char* message, const vector<data> &config, const vector<int> &carCap, const vector<int> &jobSize){
 
    int numJobsFixo = jobSize.size();
    if(numJobsFixo==20){
        numJobsFixo-=4;
    }else if(numJobsFixo==15){
        numJobsFixo-=1;
    }else if(numJobsFixo==8){
        numJobsFixo+=2;
    }
    cout<<setw(13)<<message;
    cout<<setw(13)<<fObj<<"    ";
 
    int numJobs= jobSize.size();
 
    string procOrder;
 
    for(int j=0;j<config.size();j++){
        if( config[j].job ){
            procOrder+= to_string( config[j].id +1);
            if(numJobs>1){
                procOrder+="-";
                numJobs--;
            }
        }
    }
    cout<<setw(numJobsFixo*3)<<procOrder;
    int j=0;
    string deliveryOrder;
    for(int i=0;i<config.size();i++){
        if(config[i].job){
            deliveryOrder+= to_string( config[i].id+1)+" ";
        }else{
            deliveryOrder+= " ["+ to_string(config[i].id+1) + "] ";
        }
    }
    for(int i=1; i<deliveryOrder.size()-1;i++){
        if( deliveryOrder[i-1]>='0' &&  deliveryOrder[i-1]<='9' && deliveryOrder[i+1]>='0' && deliveryOrder[i+1]<='9' ){
            deliveryOrder[i] = '-';
        }
    }
    cout<<setw(numJobsFixo*(5.5))<<deliveryOrder<<"\n\n";
}
 
bool vizi1_FI(vector<data> &configuration, vector<vehicleLoaded> &vehicleOrder, int numTrabalhos, int numVeiculos, const vector<double> &w,
	const vector<int> &P, const vector<vector<int>> &t, const vector<int> &F , const vector<int> &d){
 
    /* Change vehicle routing inside a vehicle .*/

    vector<int> startVehicleTime(numVeiculos,0);
	vector<int> deliveryTime = calculatingDeliveryTime(configuration,startVehicleTime,t,P,vehicleOrder,numTrabalhos);
    vector<int> T = calculatingJobTardiness(deliveryTime,numTrabalhos,d);
	

    
	for(int i=0;i<vehicleOrder.size();i++){
 
		// Calculate the obj function value for each car

		double bestImpObj = objFVehicle(i, vehicleOrder,configuration, numVeiculos, numTrabalhos,t,w,T,F); 

		if( vehicleOrder[i].initialPos == vehicleOrder[i].finalPos ){
			continue; //There is no more than one job in the vehicle
		}


		vector<int> randomPos (vehicleOrder[i].finalPos - vehicleOrder[i].initialPos + 1);

		int iniContJob = vehicleOrder[ i ].initialPos;

		for(int u=0;u<randomPos.size();u++){
			randomPos[ u ] = iniContJob++;
		}
		unsigned seed = 0;
		random_shuffle( randomPos.begin() , randomPos.end());


		for(int j: randomPos){

			for(int k= (j+1); k<=vehicleOrder[i].finalPos; k++){

				swap( configuration[ j ], configuration[ k ]);
				
				deliveryTime = calculatingDeliveryTime(configuration,startVehicleTime,t,P,vehicleOrder,numTrabalhos);
        		T = calculatingJobTardiness(deliveryTime,numTrabalhos,d);

				double newObj = objFVehicle(i, vehicleOrder,configuration, numVeiculos, numTrabalhos,t,w,T,F); 

				if(newObj < bestImpObj){

                    #ifdef VIEW_DECREASE
					cout<<"VZ1 FI: "<< newObj -  bestImpObj <<endl;
                    #endif

					return true;

				}
				//Swap back the jobs
				swap( configuration[ j ], configuration[ k ]);

			}

		}
	}
	


	return false;
}


bool vizi1_BI(vector<data> &configuration, vector<vehicleLoaded> &vehicleOrder, int numTrabalhos, int numVeiculos, const vector<double> &w,
    const vector<int> &P, const vector<vector<int>> &t, const vector<int> &F , const vector<int> &d){
    
    /* Change vehicle routing inside a vehicle .*/

    vector<int> startVehicleTime(numVeiculos,0);
    vector<int> deliveryTime = calculatingDeliveryTime(configuration,startVehicleTime,t,P,vehicleOrder,numTrabalhos);
    vector<int> T = calculatingJobTardiness(deliveryTime,numTrabalhos,d);
    

    vector<data> bestConfig;
    bool improved = false;
    double biggestImprove = 0;
    
    double initialVObj = objFunction(vehicleOrder,configuration, numVeiculos, numTrabalhos,t,w,T,F); 
    double bestObj = initialVObj;

    for(int i=0;i<vehicleOrder.size();i++){
 
        // Calculate the obj function value for each car


    	bool improvedCar = false;

        if( vehicleOrder[i].initialPos == vehicleOrder[i].finalPos ){
            continue; //There is no more than one job in the vehicle
        }

        for(int j=vehicleOrder[i].initialPos; j<vehicleOrder[i].finalPos; j++){

            for(int k= (j+1); k<=vehicleOrder[i].finalPos; k++){

                swap( configuration[ j ], configuration[ k ]);
                
                deliveryTime = calculatingDeliveryTime(configuration,startVehicleTime,t,P,vehicleOrder,numTrabalhos);
                T = calculatingJobTardiness(deliveryTime,numTrabalhos,d);

                double newObj = objFunction(vehicleOrder,configuration, numVeiculos, numTrabalhos,t,w,T,F); 

                if(newObj < bestObj ){

                    improved = true;
                    bestObj = newObj;
                    bestConfig = configuration;
                }
                //Swap back the jobs
                swap( configuration[ j ], configuration[ k ]);

            }

        }
    }

    if( improved ){
        configuration = bestConfig;

        #ifdef VIEW_DECREASE
        cout<<"Vz1. BI: "<<bestObj-initialVObj<<endl;
        #endif

        #ifdef VIEW_OBJ_DECREASE
        cout<<"Vz1. BI: "<<bestObj<<endl;
        #endif

        return true;
    }else{
        return false;
    }
}

bool vizi2_FI(vector<data> &configuration, vector<vehicleLoaded> &vehicleOrder, int numTrabalhos, int numVeiculos, const vector<double> &w,
	const vector<int> &P, const vector<vector<int>> &t, const vector<int> &F , const vector<int> &d, const vector<int> &Q, const vector<int> &s){

	/* Job swap between vehicles. */

	vector<int> currCap( numVeiculos, 0 );
	for(int i=0;i< vehicleOrder.size(); i++){

		int whichVehicle = vehicleOrder[i].id;

		for(int j=vehicleOrder[i].initialPos; j<= vehicleOrder[i].finalPos; j++){
			currCap[ whichVehicle ] += s[ configuration[j].id ];
		}

	}

	vector<int> startVehicleTime(numVeiculos,0);
	vector<int> deliveryTime = calculatingDeliveryTime(configuration,startVehicleTime,t,P,vehicleOrder,numTrabalhos);
    vector<int> T = calculatingJobTardiness(deliveryTime,numTrabalhos,d);
 
	for(int i=0; i< vehicleOrder.size()-1; i++){

		


		vector<int> randomPos (vehicleOrder[i].finalPos - vehicleOrder[i].initialPos + 1);

		int iniContJob = vehicleOrder[ i ].initialPos;

		for(int u=0;u<randomPos.size();u++){
			randomPos[ u ] = iniContJob++;
		}
		unsigned seed = 0;
		random_shuffle( randomPos.begin() , randomPos.end());


		for(int j: randomPos){


			for(int next= i+1; next<vehicleOrder.size(); next++){

		

				/* Otimizar mais tarde.
				double bestObj = objFVehicle(i, vehicleOrder,configuration, numVeiculos, numTrabalhos,t,w,T,F); 
			    bestObj += objFVehicle(next, vehicleOrder,configuration, numVeiculos, numTrabalhos,t,w,T,F); 
				*/
				deliveryTime = calculatingDeliveryTime(configuration,startVehicleTime,t,P,vehicleOrder,numTrabalhos);
				T = calculatingJobTardiness(deliveryTime,numTrabalhos,d);

				double bestObj = objFunction(vehicleOrder,configuration, numVeiculos, numTrabalhos,t,w,T,F);  

				for(int k= vehicleOrder[next].initialPos; k<= vehicleOrder[next].finalPos; k++){

					//Checking if the capacity of each car will be respected

					if( (currCap[ vehicleOrder[ i ].id]- s[ configuration[j].id ]+ s[ configuration[k].id ]) <= Q[ vehicleOrder[i].id]
						&& (currCap[ vehicleOrder[ next ].id] - s[configuration[k].id]+ s[ configuration[j].id]) <= Q[ vehicleOrder[next].id] ){

						swap( configuration[j], configuration[k]);

						// deliveryTime = calculatingDeliveryTime(configuration,startVehicleTime,t,P,vehicleOrder,numTrabalhos);
						// T = calculatingJobTardiness(deliveryTime,numTrabalhos,d);

				
					
						deliveryTime = calculatingDeliveryTime(configuration,startVehicleTime,t,P,vehicleOrder,numTrabalhos);
					    T = calculatingJobTardiness(deliveryTime,numTrabalhos,d);


					 	
					 	/* Otimizar mais tarde.
						double newDoubleConfig = objFVehicle(i, vehicleOrder,configuration, numVeiculos, numTrabalhos,t,w,T,F); 
				   		newDoubleConfig += objFVehicle(next, vehicleOrder,configuration, numVeiculos, numTrabalhos,t,w,T,F); 
						*/
				   			
				   		double newDoubleConfig = objFunction(vehicleOrder,configuration, numVeiculos, numTrabalhos,t,w,T,F);  

				   		if( newDoubleConfig < bestObj ){

                            #ifdef VIEW_DECREASE
							cout<<"VZ2 FI: "<<newDoubleConfig-bestObj<<endl;
                            #endif

				   			return true;


				   		}
						swap( configuration[j], configuration[k]);

					}
				}

			}

			


		}

	}

	return false;
}

bool vizi2_BI(vector<data> &configuration, vector<vehicleLoaded> &vehicleOrder, int numTrabalhos, int numVeiculos, const vector<double> &w,
    const vector<int> &P, const vector<vector<int>> &t, const vector<int> &F , const vector<int> &d, const vector<int> &Q, const vector<int> &s){

    /* Job swap between vehicles. */

    vector<int> currCap( numVeiculos, 0 );
    for(int i=0;i< vehicleOrder.size(); i++){

        int whichVehicle = vehicleOrder[i].id;

        for(int j=vehicleOrder[i].initialPos; j<= vehicleOrder[i].finalPos; j++){
            currCap[ whichVehicle ] += s[ configuration[j].id ];
        }

    }

    vector<int> startVehicleTime(numVeiculos,0);
    vector<int> deliveryTime = calculatingDeliveryTime(configuration,startVehicleTime,t,P,vehicleOrder,numTrabalhos);
    vector<int> T = calculatingJobTardiness(deliveryTime,numTrabalhos,d);
    
    
    double bestObj = objFunction(vehicleOrder,configuration, numVeiculos, numTrabalhos,t,w,T,F);  
    double iniObj = bestObj;
    bool improved = false;
    vector<data> bestConfig = configuration;

    for(int i=0; i< vehicleOrder.size()-1; i++){

        
        for(int j= vehicleOrder[i].initialPos; j<=vehicleOrder[i].finalPos; j++){


            for(int next= i+1; next<vehicleOrder.size(); next++){

        

                /* Otimizar mais tarde.
                double bestObj = objFVehicle(i, vehicleOrder,configuration, numVeiculos, numTrabalhos,t,w,T,F); 
                bestObj += objFVehicle(next, vehicleOrder,configuration, numVeiculos, numTrabalhos,t,w,T,F); 
                */
                deliveryTime = calculatingDeliveryTime(configuration,startVehicleTime,t,P,vehicleOrder,numTrabalhos);
                T = calculatingJobTardiness(deliveryTime,numTrabalhos,d);


                for(int k= vehicleOrder[next].initialPos; k<= vehicleOrder[next].finalPos; k++){

                    //Checking if the capacity of each car will be respected

                    if( (currCap[ vehicleOrder[ i ].id]- s[ configuration[j].id ]+ s[ configuration[k].id ]) <= Q[ vehicleOrder[i].id]
                        && (currCap[ vehicleOrder[ next ].id] - s[configuration[k].id]+ s[ configuration[j].id]) <= Q[ vehicleOrder[next].id] ){

                        swap( configuration[j], configuration[k]);

                        // deliveryTime = calculatingDeliveryTime(configuration,startVehicleTime,t,P,vehicleOrder,numTrabalhos);
                        // T = calculatingJobTardiness(deliveryTime,numTrabalhos,d);

                
                    
                        deliveryTime = calculatingDeliveryTime(configuration,startVehicleTime,t,P,vehicleOrder,numTrabalhos);
                        T = calculatingJobTardiness(deliveryTime,numTrabalhos,d);


                        
                        /* Otimizar mais tarde.
                        double newDoubleConfig = objFVehicle(i, vehicleOrder,configuration, numVeiculos, numTrabalhos,t,w,T,F); 
                        newDoubleConfig += objFVehicle(next, vehicleOrder,configuration, numVeiculos, numTrabalhos,t,w,T,F); 
                        */
                            
                        double newDoubleConfig = objFunction(vehicleOrder,configuration, numVeiculos, numTrabalhos,t,w,T,F);  

                        if( newDoubleConfig < bestObj ){

                            improved = true;
                            bestConfig = configuration;
                            bestObj = newDoubleConfig;

                            // cout<<"VZ2: "<<newDoubleConfig-bestObj<<endl;


                        }
                        swap( configuration[j], configuration[k]);

                    }
                }
            }
        }
    }

    if( improved ){

    	#ifdef VIEW_DECREASE
        cout<<"Vz2. BI: "<<bestObj - iniObj<<endl;
        #endif

        #ifdef VIEW_OBJ_DECREASE
        cout<<"Vz2. BI: "<<bestObj<<endl;
        #endif

        configuration = bestConfig;
        return true;
    }else{
        return false;
    }
}

bool vizi3_FI(vector<data> &configuration, vector<vehicleLoaded> &vehicleOrder, int numTrabalhos, int numVeiculos, const vector<double> &w,
	const vector<int> &P, const vector<vector<int>> &t, const vector<int> &F , const vector<int> &d){

    /* Inserting jobs inside a vehicle */

	vector<int> startVehicleTime(numVeiculos,0);
	vector<int> deliveryTime = calculatingDeliveryTime(configuration,startVehicleTime,t,P,vehicleOrder,numTrabalhos);
    vector<int> T = calculatingJobTardiness(deliveryTime,numTrabalhos,d);
	
	for(int i=0; i< vehicleOrder.size();i++){

		if( vehicleOrder[i].initialPos == vehicleOrder[i].finalPos){
			continue;
		}
		double bestObj = objFVehicle(i, vehicleOrder,configuration, numVeiculos, numTrabalhos,t,w,T,F); 

		//Saving the original configuration of jobs in that vehicle
		vector<int> original;
		for(int k= vehicleOrder[i].initialPos; k<= vehicleOrder[i].finalPos; k++){
				original.push_back( configuration[ k ].id );
		}
	

		vector<int> randomPos (vehicleOrder[i].finalPos - vehicleOrder[i].initialPos + 1);

		int iniContJob = vehicleOrder[ i ].initialPos;

		for(int u=0;u<randomPos.size();u++){
			randomPos[ u ] = iniContJob++;
		}
		unsigned seed = 0;
		random_shuffle( randomPos.begin() , randomPos.end());


		for(int j: randomPos){

			int inserirPraTras = j - vehicleOrder[i].initialPos - 1;

			int contUpward = 0; 

			// cout<<"Inserir pra tras: "<<inserirPraTras<<" size: "<<vehicleOrder[i].finalPos-vehicleOrder[i].initialPos<<endl;
			//Inserting jobs leftward ( except for the immediatly close)
			while( inserirPraTras-- >0){
				// cout<<"e eu entrei\n";
				vector<int> newvector ( vehicleOrder[i].finalPos - vehicleOrder[i].initialPos +1, -1);
				set<int> jobsIn;

				int contInserir = 0;
				
				newvector[ contUpward ] = configuration[j].id; //Inserting job in the required place
				jobsIn.insert( configuration[j].id );
				// cout<<"Eu ja!! inseri "<<configuration[j].id<<"na posicao "<<contUpward<<endl;
				for(int k= vehicleOrder[i].initialPos; k<= vehicleOrder[i].finalPos; k++){
					// cout<<"Posicao "<<k<<" do trabalho "<< configuration[k].id<<" ";
					if( jobsIn.find( configuration[k].id ) == jobsIn.end()){
                        while( contInserir < newvector.size() && newvector[contInserir]!=-1){
                            contInserir++;
                        }
						// cout<<"e eu ainda nao o inseri\n";
						newvector[ contInserir ] = configuration[k].id;
						jobsIn.insert( configuration[k].id );
					}else{
						// cout<<"e eu ja o inseri\n";
					}
				}

				int contCopiar = 0;

				for(int k= vehicleOrder[i].initialPos; k<= vehicleOrder[i].finalPos; k++){
					configuration[ k ].id = newvector[ contCopiar ];
					contCopiar++;
				}

				deliveryTime = calculatingDeliveryTime(configuration,startVehicleTime,t,P,vehicleOrder,numTrabalhos);
    			T = calculatingJobTardiness(deliveryTime,numTrabalhos,d);
				double newObj = objFVehicle(i, vehicleOrder,configuration, numVeiculos, numTrabalhos,t,w,T,F); 
				if( newObj < bestObj){

                    #ifdef VIEW_DECREASE
					cout<<"VZ3 FI: "<<newObj-bestObj<<endl;
                    #endif

					return true;
				
				}

				contCopiar = 0;
				for(int k= vehicleOrder[i].initialPos; k<= vehicleOrder[i].finalPos; k++){
					configuration[ k ].id = original[ contCopiar ];
					contCopiar++;
				}


				contUpward++;
			}

			int ondeInserir = j - vehicleOrder[i].initialPos+ 1;
			int quantosInserir = j + 1 ;

			while(quantosInserir <= vehicleOrder[i].finalPos){

				// cout<<"Entrei pra inserir pra frente\n";
				vector<int> newvector( vehicleOrder[i].finalPos - vehicleOrder[i].initialPos +1, -1);
				set<int> jobsIn;


				newvector[ ondeInserir ] = configuration[j].id;
				jobsIn.insert( configuration[ j ].id );

				int contInserirVec = 0;

				for(int k= vehicleOrder[i].initialPos; k<= vehicleOrder[i].finalPos; k++){

					if( jobsIn.find( configuration[k].id ) == jobsIn.end()){

                        while( contInserirVec< newvector.size() && newvector[ contInserirVec]!=-1){
                            contInserirVec++;
                        }
						newvector[ contInserirVec ] = configuration[k].id;
						jobsIn.insert( configuration[k].id );
                        contInserirVec++;
					}
				}

				int contCopiar=0;
				for(int k= vehicleOrder[i].initialPos; k<= vehicleOrder[i].finalPos; k++){
					configuration[ k ].id = newvector[ contCopiar ];
					contCopiar++;
				}


				deliveryTime = calculatingDeliveryTime(configuration,startVehicleTime,t,P,vehicleOrder,numTrabalhos);
    			T = calculatingJobTardiness(deliveryTime,numTrabalhos,d);
				double newObj = objFVehicle(i, vehicleOrder,configuration, numVeiculos, numTrabalhos,t,w,T,F); 

				if( newObj < bestObj){

                    #ifdef VIEW_DECREASE
					cout<<"VZ3 FI: "<<newObj-bestObj<<endl;
                    #endif

					return true;
				
				}


				contCopiar=0;
				for(int k= vehicleOrder[i].initialPos; k<= vehicleOrder[i].finalPos; k++){
					configuration[ k ].id = original[ contCopiar ];
					contCopiar++;
				}

				quantosInserir++;
				ondeInserir++;
			}

		}
	}
	return false;
}

bool vizi3_BI(vector<data> &configuration, vector<vehicleLoaded> &vehicleOrder, int numTrabalhos, int numVeiculos, const vector<double> &w,
    const vector<int> &P, const vector<vector<int>> &t, const vector<int> &F , const vector<int> &d){

    /* Inserting jobs inside a vehicle */

    vector<int> startVehicleTime(numVeiculos,0);
    vector<int> deliveryTime = calculatingDeliveryTime(configuration,startVehicleTime,t,P,vehicleOrder,numTrabalhos);
    vector<int> T = calculatingJobTardiness(deliveryTime,numTrabalhos,d);
    

    double bestImprove = 0;
    bool improved = false;
    vector<data> bestConfig = configuration;

   double iniObj = objFunction(vehicleOrder,configuration, numVeiculos, numTrabalhos,t,w,T,F); 


    for(int i=0; i< vehicleOrder.size();i++){

        if( vehicleOrder[i].initialPos == vehicleOrder[i].finalPos){
            continue;
        }
        double bestObj = objFVehicle(i, vehicleOrder,configuration, numVeiculos, numTrabalhos,t,w,T,F); 

        //Saving the original configuration of jobs in that vehicle
        vector<int> original;
        for(int k= vehicleOrder[i].initialPos; k<= vehicleOrder[i].finalPos; k++){
                original.push_back( configuration[ k ].id );
        }
    

        for(int j= vehicleOrder[i].initialPos; j<=vehicleOrder[i].finalPos; j++){

            int inserirPraTras = j - vehicleOrder[i].initialPos - 1;

            int contUpward = 0; 

            // cout<<"Inserir pra tras: "<<inserirPraTras<<" size: "<<vehicleOrder[i].finalPos-vehicleOrder[i].initialPos<<endl;
            //Inserting jobs leftward ( except for the immediatly close)
            while( inserirPraTras-- >0){
                // cout<<"e eu entrei\n";
                vector<int> newvector ( vehicleOrder[i].finalPos - vehicleOrder[i].initialPos +1, -1);
                set<int> jobsIn;

                int contInserir = 0;
                
                newvector[ contUpward ] = configuration[j].id; //Inserting job in the required place
                jobsIn.insert( configuration[j].id );
                // cout<<"Eu ja!! inseri "<<configuration[j].id<<"na posicao "<<contUpward<<endl;
                for(int k= vehicleOrder[i].initialPos; k<= vehicleOrder[i].finalPos; k++){
                    // cout<<"Posicao "<<k<<" do trabalho "<< configuration[k].id<<" ";
                    if( jobsIn.find( configuration[k].id ) == jobsIn.end()){
                        while( contInserir < newvector.size() && newvector[contInserir]!=-1){
                            contInserir++;
                        }
                        // cout<<"e eu ainda nao o inseri\n";
                        newvector[ contInserir ] = configuration[k].id;
                        jobsIn.insert( configuration[k].id );
                    }else{
                        // cout<<"e eu ja o inseri\n";
                    }
                }

                int contCopiar = 0;

                for(int k= vehicleOrder[i].initialPos; k<= vehicleOrder[i].finalPos; k++){
                    configuration[ k ].id = newvector[ contCopiar ];
                    contCopiar++;
                }

                deliveryTime = calculatingDeliveryTime(configuration,startVehicleTime,t,P,vehicleOrder,numTrabalhos);
                T = calculatingJobTardiness(deliveryTime,numTrabalhos,d);
                double newObj = objFVehicle(i, vehicleOrder,configuration, numVeiculos, numTrabalhos,t,w,T,F); 

                if( newObj < bestObj && (newObj-bestObj) < bestImprove) {

                    bestImprove= (newObj-bestObj);
                    improved = true;
                    bestConfig = configuration;            
                }

                contCopiar = 0;
                for(int k= vehicleOrder[i].initialPos; k<= vehicleOrder[i].finalPos; k++){
                    configuration[ k ].id = original[ contCopiar ];
                    contCopiar++;
                }


                contUpward++;
            }

            int ondeInserir = j - vehicleOrder[i].initialPos+ 1;
            int quantosInserir = j + 1 ;

            while(quantosInserir <= vehicleOrder[i].finalPos){

                // cout<<"Entrei pra inserir pra frente\n";
                vector<int> newvector( vehicleOrder[i].finalPos - vehicleOrder[i].initialPos +1, -1);
                set<int> jobsIn;


                newvector[ ondeInserir ] = configuration[j].id;
                jobsIn.insert( configuration[ j ].id );

                int contInserirVec = 0;

                for(int k= vehicleOrder[i].initialPos; k<= vehicleOrder[i].finalPos; k++){

                    if( jobsIn.find( configuration[k].id ) == jobsIn.end()){

                        while( contInserirVec< newvector.size() && newvector[ contInserirVec]!=-1){
                            contInserirVec++;
                        }
                        newvector[ contInserirVec ] = configuration[k].id;
                        jobsIn.insert( configuration[k].id );
                        contInserirVec++;
                    }
                }

                int contCopiar=0;
                for(int k= vehicleOrder[i].initialPos; k<= vehicleOrder[i].finalPos; k++){
                    configuration[ k ].id = newvector[ contCopiar ];
                    contCopiar++;
                }


                deliveryTime = calculatingDeliveryTime(configuration,startVehicleTime,t,P,vehicleOrder,numTrabalhos);
                T = calculatingJobTardiness(deliveryTime,numTrabalhos,d);
                double newObj = objFVehicle(i, vehicleOrder,configuration, numVeiculos, numTrabalhos,t,w,T,F); 

                 if( newObj < bestObj && (newObj-bestObj) < bestImprove) {

                    bestImprove= (newObj-bestObj);
                    improved = true;
                    bestConfig = configuration;            
                }


                contCopiar=0;
                for(int k= vehicleOrder[i].initialPos; k<= vehicleOrder[i].finalPos; k++){
                    configuration[ k ].id = original[ contCopiar ];
                    contCopiar++;
                }

                quantosInserir++;
                ondeInserir++;
            }

        }
    }

    if( improved ){

    	#ifdef VIEW_DECREASE
        cout<<"Vz3 BI: "<<bestImprove<<endl;
        #endif

        #ifdef VIEW_OBJ_DECREASE
        cout<<"Vz3 BI: "<<iniObj + bestImprove<<endl;
        #endif

        configuration = bestConfig;
        return true;
    }else{
        return false;
    }
}

bool vizi4_FI(vector<data> &configuration, vector<vehicleLoaded> &vehicleOrder, int numTrabalhos, int numVeiculos, const vector<double> &w,
    const vector<int> &P, const vector<vector<int>> &t, const vector<int> &F , const vector<int> &d, const vector<int> &Q, const vector<int> &s){

    /* Inserting jobs in other vehicles (already in use and the others) */



    //Calculating the current car volume
    vector<int> currCap( numVeiculos, 0 );
    vector<bool> carInUse( numVeiculos, false); 



    for(int i=0;i< vehicleOrder.size(); i++){

        int whichVehicle = vehicleOrder[i].id;

    	carInUse[ whichVehicle ] = true;

        for(int j=vehicleOrder[i].initialPos; j<= vehicleOrder[i].finalPos; j++){
            currCap[ whichVehicle ] += s[ configuration[j].id ];
        }

    }

   //Teste
 	vector<int> startVehicleTime(numVeiculos,0);
    vector<int> deliveryTime = calculatingDeliveryTime(configuration,startVehicleTime,t,P,vehicleOrder,numTrabalhos);
    vector<int> T = calculatingJobTardiness(deliveryTime,numTrabalhos,d);

    double bestObj = objFunction(vehicleOrder,configuration, numVeiculos, numTrabalhos,t,w,T,F); 
    // bestObj += objFVehicle(tempBefore, vehicleOrder,configuration, numVeiculos, numTrabalhos,t,w,T,F); 

    for(int i=0; i<vehicleOrder.size(); i++){


        int carsBeforeMe = i;
        int carsAfterMe = vehicleOrder.size() - i - 1;


        int tempBefore = carsBeforeMe; 
        
        while(tempBefore > 0){
                
            --tempBefore;


            vector<int> randomPos (vehicleOrder[i].finalPos - vehicleOrder[i].initialPos + 1);

			int iniContJob = vehicleOrder[ i ].initialPos;

			for(int u=0;u<randomPos.size();u++){
				randomPos[ u ] = iniContJob++;
			}
			unsigned seed = 0;
			random_shuffle( randomPos.begin() , randomPos.end());


			for(int j: randomPos){
                //For each task in that car
            //continuar

                //IF the car before me can handle carrying this job, we calculate the benefit int puting the job on it
                if( (currCap[ vehicleOrder[ tempBefore ].id ] + s[ configuration[j].id ]) <= (Q[ vehicleOrder[tempBefore].id] )){

                    int range = vehicleOrder[ tempBefore].finalPos - vehicleOrder[ tempBefore ].initialPos + 2;

                    int idCarInsert = vehicleOrder[ tempBefore ].id;
                    


                    //Trying to put in each space avaiable on the other car
                    for(int z=0; z < range; z++  ){

                        vector<int> newcarried( range ,-1);

                        newcarried[ z ] = configuration[j].id;
                        int copy= vehicleOrder[ tempBefore ].initialPos;
                        for(int u=0; u< range; u++){
                            if(z==u)
                                continue;
                            newcarried[ u ] = configuration[ copy++ ].id ;
                        }
                     
                        vector<data> newconfig( configuration.size());



                        int copyConfig =0;
                        int contInsertNew = 0;

                        while( copyConfig < configuration.size()){

                            if( configuration[ copyConfig ].job && configuration[ copyConfig ].id == configuration[j].id){
                                //Job moved
                                copyConfig++;
                                continue;
                            }

                            if( !configuration[ copyConfig ].job && configuration[ copyConfig ].id == idCarInsert){
                                newconfig[ contInsertNew ] = configuration[ copyConfig ];
                                
                                copyConfig++;

                                contInsertNew++;

                                int insert = range;
                                int contUpward = 0; 

                                while(insert > 0 ){
                                    newconfig[ contInsertNew ].job = true;
                                    newconfig[ contInsertNew ].id = newcarried[ contUpward ];
                                    contUpward++;
                                    contInsertNew++;

                                    copyConfig++;

                                    insert--;
                                }
                                copyConfig--; //Deleting the extra job inserted

                                continue;

                            }

                            
                            newconfig[ contInsertNew ] = configuration[ copyConfig ];
                            contInsertNew++;
                            copyConfig++;                            

                        }

                        vector<vehicleLoaded> newVehicleOrder=generateVehicleOrder(newconfig,numVeiculos);//Generating info about vehicle transportation
                        

                        
                        //Creating a new configuration

                        vector<int> newStartVehicleTime(numVeiculos,0);
                        vector<int> newDeliveryTime = calculatingDeliveryTime(newconfig,newStartVehicleTime,t,P,newVehicleOrder,numTrabalhos);
                        vector<int> newT = calculatingJobTardiness(newDeliveryTime,numTrabalhos,d);

                        double newObj=0;
                        newObj = objFunction(newVehicleOrder,newconfig, numVeiculos, numTrabalhos,t,w,newT,F); 

                        if(newObj < bestObj && newObj!=0){

                            configuration = newconfig;
                            vehicleOrder = newVehicleOrder;

                            #ifdef VIEW_DECREASE
                            cout<<"VZ4 FI:"<<newObj-bestObj<<endl;
                            #endif

                            return true;
                            //Return true
                        }
                    }
                }
            }
        }

        int carAhead = i+1 ;
        
        while(carAhead < vehicleOrder.size()  ){

            vector<int> randomPos (vehicleOrder[i].finalPos - vehicleOrder[i].initialPos + 1);

			int iniContJob = vehicleOrder[ i ].initialPos;

			for(int u=0;u<randomPos.size();u++){
				randomPos[ u ] = iniContJob++;
			}
			unsigned seed = 0;
			random_shuffle( randomPos.begin() , randomPos.end());


			for(int j: randomPos){
                //For each task in that car
            //continuar

                //IF the car before me can handle carrying this job, we calculate the benefit int puting the job on it
                if( (currCap[ vehicleOrder[ carAhead ].id ] + s[ configuration[j].id ]) <= (Q[ vehicleOrder[carAhead].id] )){

                    int range = vehicleOrder[ carAhead].finalPos - vehicleOrder[ carAhead ].initialPos + 2;

                    int idCarInsert = vehicleOrder[ carAhead ].id;
                    


                    //Trying to put in each space avaiable on the other car
                    for(int z=0; z < range; z++  ){

                        vector<int> newcarried( range ,-1);

                        newcarried[ z ] = configuration[j].id;
                        int copy= vehicleOrder[ carAhead ].initialPos;
                        for(int u=0; u< range; u++){
                            if(z==u)
                                continue;
                            newcarried[ u ] = configuration[ copy++ ].id ;
                        }

          
                        vector<data> newconfig( configuration.size());


                        int copyConfig=0;
                        int contInsertNew = 0;

                        while( copyConfig < configuration.size()){

                            if( configuration[ copyConfig ].job && configuration[ copyConfig ].id == configuration[j].id){
                                //Job moved
                                copyConfig++;
                                continue;
                            }

                            if( !configuration[ copyConfig ].job && configuration[ copyConfig ].id == idCarInsert){
                                newconfig[ contInsertNew ] = configuration[ copyConfig ];
                                
                                copyConfig++;

                                contInsertNew++;

                                int insert = range;
                                int contUpward = 0; 

                                while(insert > 0 ){
                                    newconfig[ contInsertNew ].job = true;
                                    newconfig[ contInsertNew ].id = newcarried[ contUpward ];
                                    contUpward++;
                                    contInsertNew++;

                                    copyConfig++;

                                    insert--;
                                }
                                copyConfig--; //Deleting the extra job inserted

                                continue;

                            }

                            
                            newconfig[ contInsertNew ] = configuration[ copyConfig ];
                            contInsertNew++;
                            copyConfig++;                            

                        }

                        vector<vehicleLoaded> newVehicleOrder=generateVehicleOrder(newconfig,numVeiculos);//Generating info about vehicle transportation
                        //Creating a new configuration

                        vector<int> newStartVehicleTime(numVeiculos,0);
                        vector<int> newDeliveryTime = calculatingDeliveryTime(newconfig,newStartVehicleTime,t,P,newVehicleOrder,numTrabalhos);
                        vector<int> newT = calculatingJobTardiness(newDeliveryTime,numTrabalhos,d);

                        double newObj=0;
                        newObj = objFunction(newVehicleOrder,newconfig, numVeiculos, numTrabalhos,t,w,newT,F);             

                        if(newObj < bestObj && newObj!=0){

                            configuration = newconfig;
                            vehicleOrder = newVehicleOrder;


                            #ifdef VIEW_DECREASE
                             cout<<"VZ4_FI: "<<newObj-bestObj<<endl;
                             #endif

                            return true;
                        }
                    }
                }
            }
            ++carAhead;
        }
    }

    
    for(int j=0; j<numVeiculos; j++){

    	// cout<<"Tentando inserir nos carros vazios\n";

    	if(!carInUse[ j ]){

    		for(int i=0;i<vehicleOrder.size();i++){

		    	vector<int> startVehicleTime(numVeiculos,0);
		        vector<int> deliveryTime = calculatingDeliveryTime(configuration,startVehicleTime,t,P,vehicleOrder,numTrabalhos);
		        vector<int> T = calculatingJobTardiness(deliveryTime,numTrabalhos,d);

		        double bestObj = objFunction(vehicleOrder,configuration, numVeiculos, numTrabalhos,t,w,T,F); 


				//For each task in this car...


		        vector<int> randomPos (vehicleOrder[i].finalPos - vehicleOrder[i].initialPos + 1);

				int iniContJob = vehicleOrder[ i ].initialPos;

				for(int u=0;u<randomPos.size();u++){
					randomPos[ u ] = iniContJob++;
				}
				unsigned seed = 0;
				random_shuffle( randomPos.begin() , randomPos.end());


				for(int k: randomPos){

					if( Q[ j ]>= s[ configuration[k].id ]){

						vector<data> newConfig ( configuration.size());

						int insert = 0;
						for(int z=0; z<configuration.size(); z++){

							if( !configuration[z].job && configuration[z].id== j){ //The new car to insert each job

								newConfig[insert++]= configuration[z];
								newConfig[ insert ].job = true;
								newConfig[ insert ].id = configuration[k].id;
								++insert;
							}else if( configuration[z].job && configuration[z].id == configuration[k].id){
								continue;
							}else{
								newConfig[ insert++ ] = configuration[z];
							}
						}

						//Verifying if the obj function does improve with this change

		                vector<vehicleLoaded> newVehicleOrder=generateVehicleOrder(newConfig,numVeiculos);//Generating info about vehicle transportation
		    			vector<int> newStartVehicleTime(numVeiculos,0);
			        	vector<int> newDeliveryTime = calculatingDeliveryTime(newConfig,newStartVehicleTime,t,P,newVehicleOrder,numTrabalhos);
			        	vector<int> newT = calculatingJobTardiness(newDeliveryTime,numTrabalhos,d);

			        	double newObj = objFunction(newVehicleOrder,newConfig, numVeiculos, numTrabalhos,t,w,newT,F);

			        	if(newObj < bestObj){
			        		vehicleOrder = newVehicleOrder;
			        		configuration = newConfig;

			        		// cout<<"CONFIG EMPTY: ";
			        		// for(data &u:configuration){
             //                	if(u.job){
             //                		cout<<"J"<<u.id<<" ";
             //                	}else{
             //                		cout<<"V"<<u.id<<" ";
             //                	}
             //                }cout<<endl;
                            #ifdef VIEW_DECREASE
			        		 cout<<"VZ4 FI:"<<newObj-bestObj<<endl;
                             #endif
                             
			        		return true;
			        	}

					}


    			}

    		}

    	}
    }
    
	
    return false;
}

bool vizi4_BI(vector<data> &configuration, vector<vehicleLoaded> &vehicleOrder, int numTrabalhos, int numVeiculos, const vector<double> &w,
    const vector<int> &P, const vector<vector<int>> &t, const vector<int> &F , const vector<int> &d, const vector<int> &Q, const vector<int> &s){

    /* Inserting jobs in other vehicles (already in use and the others) */



    //Calculating the current car volume
    vector<int> currCap( numVeiculos, 0 );
    vector<bool> carInUse( numVeiculos, false); 

 //Teste
    vector<int> startVehicleTime(numVeiculos,0);
    vector<int> deliveryTime = calculatingDeliveryTime(configuration,startVehicleTime,t,P,vehicleOrder,numTrabalhos);
    vector<int> T = calculatingJobTardiness(deliveryTime,numTrabalhos,d);

	double bestObj = objFunction(vehicleOrder,configuration, numVeiculos, numTrabalhos,t,w,T,F); 
    double iniObj = bestObj;
    double improved = false;
    vector<data> bestConfig = configuration;
    vector<vehicleLoaded> bestVOrder = vehicleOrder;

    for(int i=0;i< vehicleOrder.size(); i++){

        int whichVehicle = vehicleOrder[i].id;

        carInUse[ whichVehicle ] = true;

        for(int j=vehicleOrder[i].initialPos; j<= vehicleOrder[i].finalPos; j++){
            currCap[ whichVehicle ] += s[ configuration[j].id ];
        }

    }

  
     

    for(int i=0; i<vehicleOrder.size(); i++){


        int carsBeforeMe = i;
        int carsAfterMe = vehicleOrder.size() - i - 1;


        int tempBefore = carsBeforeMe; 
        
        while(tempBefore > 0){
                
            --tempBefore;


            for(int j= vehicleOrder[i].initialPos; j<= vehicleOrder[i].finalPos; j++){
                //For each task in that car
            //continuar

                //IF the car before me can handle carrying this job, we calculate the benefit int puting the job on it
                if( (currCap[ vehicleOrder[ tempBefore ].id ] + s[ configuration[j].id ]) <= (Q[ vehicleOrder[tempBefore].id] )){

                    int range = vehicleOrder[ tempBefore].finalPos - vehicleOrder[ tempBefore ].initialPos + 2;

                    int idCarInsert = vehicleOrder[ tempBefore ].id;
                    


                    //Trying to put in each space avaiable on the other car
                    for(int z=0; z < range; z++  ){

                        vector<int> newcarried( range ,-1);

                        newcarried[ z ] = configuration[j].id;
                        int copy= vehicleOrder[ tempBefore ].initialPos;
                        for(int u=0; u< range; u++){
                            if(z==u)
                                continue;
                            newcarried[ u ] = configuration[ copy++ ].id ;
                        }
                     
                        vector<data> newconfig( configuration.size());



                        int copyConfig =0;
                        int contInsertNew = 0;

                        while( copyConfig < configuration.size()){

                            if( configuration[ copyConfig ].job && configuration[ copyConfig ].id == configuration[j].id){
                                //Job moved
                                copyConfig++;
                                continue;
                            }

                            if( !configuration[ copyConfig ].job && configuration[ copyConfig ].id == idCarInsert){
                                newconfig[ contInsertNew ] = configuration[ copyConfig ];
                                
                                copyConfig++;

                                contInsertNew++;

                                int insert = range;
                                int contUpward = 0; 

                                while(insert > 0 ){
                                    newconfig[ contInsertNew ].job = true;
                                    newconfig[ contInsertNew ].id = newcarried[ contUpward ];
                                    contUpward++;
                                    contInsertNew++;

                                    copyConfig++;

                                    insert--;
                                }
                                copyConfig--; //Deleting the extra job inserted

                                continue;

                            }

                            
                            newconfig[ contInsertNew ] = configuration[ copyConfig ];
                            contInsertNew++;
                            copyConfig++;                            

                        }

                        vector<vehicleLoaded> newVehicleOrder=generateVehicleOrder(newconfig,numVeiculos);//Generating info about vehicle transportation
                        

                        
                        //Creating a new configuration

                        vector<int> newStartVehicleTime(numVeiculos,0);
                        vector<int> newDeliveryTime = calculatingDeliveryTime(newconfig,newStartVehicleTime,t,P,newVehicleOrder,numTrabalhos);
                        vector<int> newT = calculatingJobTardiness(newDeliveryTime,numTrabalhos,d);

                        double newObj=0;
                        newObj = objFunction(newVehicleOrder,newconfig, numVeiculos, numTrabalhos,t,w,newT,F); 

                        if(newObj < bestObj && newObj!=0){


                            bestConfig = newconfig;
                            bestVOrder = newVehicleOrder;

                            improved = true;
                            bestObj = newObj;

                            // cout<<"VZ4_Back:"<<newObj-bestObj<<endl;
                        }
                    }
                }
            }
        }

        int carAhead = i+1 ;
        
        while(carAhead < vehicleOrder.size()  ){


            for(int j= vehicleOrder[i].initialPos; j<= vehicleOrder[i].finalPos; j++){
                //For each task in that car
            //continuar

                //IF the car before me can handle carrying this job, we calculate the benefit int puting the job on it
                if( (currCap[ vehicleOrder[ carAhead ].id ] + s[ configuration[j].id ]) <= (Q[ vehicleOrder[carAhead].id] )){

                    int range = vehicleOrder[ carAhead].finalPos - vehicleOrder[ carAhead ].initialPos + 2;

                    int idCarInsert = vehicleOrder[ carAhead ].id;
                    


                    //Trying to put in each space avaiable on the other car
                    for(int z=0; z < range; z++  ){

                        vector<int> newcarried( range ,-1);

                        newcarried[ z ] = configuration[j].id;
                        int copy= vehicleOrder[ carAhead ].initialPos;
                        for(int u=0; u< range; u++){
                            if(z==u)
                                continue;
                            newcarried[ u ] = configuration[ copy++ ].id ;
                        }

          
                        vector<data> newconfig( configuration.size());


                        int copyConfig=0;
                        int contInsertNew = 0;

                        while( copyConfig < configuration.size()){

                            if( configuration[ copyConfig ].job && configuration[ copyConfig ].id == configuration[j].id){
                                //Job moved
                                copyConfig++;
                                continue;
                            }

                            if( !configuration[ copyConfig ].job && configuration[ copyConfig ].id == idCarInsert){
                                newconfig[ contInsertNew ] = configuration[ copyConfig ];
                                
                                copyConfig++;

                                contInsertNew++;

                                int insert = range;
                                int contUpward = 0; 

                                while(insert > 0 ){
                                    newconfig[ contInsertNew ].job = true;
                                    newconfig[ contInsertNew ].id = newcarried[ contUpward ];
                                    contUpward++;
                                    contInsertNew++;

                                    copyConfig++;

                                    insert--;
                                }
                                copyConfig--; //Deleting the extra job inserted

                                continue;

                            }

                            
                            newconfig[ contInsertNew ] = configuration[ copyConfig ];
                            contInsertNew++;
                            copyConfig++;                            

                        }

                        vector<vehicleLoaded> newVehicleOrder=generateVehicleOrder(newconfig,numVeiculos);//Generating info about vehicle transportation
                        //Creating a new configuration

                        vector<int> newStartVehicleTime(numVeiculos,0);
                        vector<int> newDeliveryTime = calculatingDeliveryTime(newconfig,newStartVehicleTime,t,P,newVehicleOrder,numTrabalhos);
                        vector<int> newT = calculatingJobTardiness(newDeliveryTime,numTrabalhos,d);

                        double newObj=0;
                        newObj = objFunction(newVehicleOrder,newconfig, numVeiculos, numTrabalhos,t,w,newT,F);             

                        if(newObj < bestObj && newObj!=0){

                            bestConfig = newconfig;
                            bestVOrder = newVehicleOrder;
                            improved = true;
                            bestObj = newObj;
                        }
                    }
                }
            }
            ++carAhead;
        }
    }

    
    for(int j=0; j<numVeiculos; j++){

        // cout<<"Tentando inserir nos carros vazios\n";

        if(!carInUse[ j ]){

            for(int i=0;i<vehicleOrder.size();i++){

                vector<int> startVehicleTime(numVeiculos,0);
                vector<int> deliveryTime = calculatingDeliveryTime(configuration,startVehicleTime,t,P,vehicleOrder,numTrabalhos);
                vector<int> T = calculatingJobTardiness(deliveryTime,numTrabalhos,d);

                //For each task in this car...

                for(int k= vehicleOrder[i].initialPos; k<= vehicleOrder[i].finalPos; k++){

                    if( Q[ j ]>= s[ configuration[k].id ]){

                        vector<data> newConfig ( configuration.size());

                        int insert = 0;
                        for(int z=0; z<configuration.size(); z++){

                            if( !configuration[z].job && configuration[z].id== j){ //The new car to insert each job

                                newConfig[insert++]= configuration[z];
                                newConfig[ insert ].job = true;
                                newConfig[ insert ].id = configuration[k].id;
                                ++insert;
                            }else if( configuration[z].job && configuration[z].id == configuration[k].id){
                                continue;
                            }else{
                                newConfig[ insert++ ] = configuration[z];
                            }
                        }

                        //Verifying if the obj function does improve with this change

                        vector<vehicleLoaded> newVehicleOrder=generateVehicleOrder(newConfig,numVeiculos);//Generating info about vehicle transportation
                        vector<int> newStartVehicleTime(numVeiculos,0);
                        vector<int> newDeliveryTime = calculatingDeliveryTime(newConfig,newStartVehicleTime,t,P,newVehicleOrder,numTrabalhos);
                        vector<int> newT = calculatingJobTardiness(newDeliveryTime,numTrabalhos,d);

                        double newObj = objFunction(newVehicleOrder,newConfig, numVeiculos, numTrabalhos,t,w,newT,F);

                        if(newObj < bestObj){

                            bestConfig = newConfig;
                            bestVOrder = newVehicleOrder;

                            improved = true;
                            bestObj = newObj;
                            // cout<<"VZ4_Empty:"<<newObj-bestObj<<endl;
                        }

                    }


                }

            }

        }
    }
    
    if( bestObj < iniObj){

        configuration = bestConfig;
        vehicleOrder= bestVOrder;//Generating info about vehicle transportation

        #ifdef VIEW_DECREASE
        cout<<"Vz4 BI: "<<bestObj-iniObj<<endl;
        #endif

        #ifdef VIEW_OBJ_DECREASE
        cout<<"Vz4 BI: "<<bestObj<<endl;
        #endif

        return true;
    }else{
        return false;
    }
}

void VND(int numTrabalhos, int numVeiculos, const vector<double> &w,const vector<int> &P, const vector<vector<int>> &t, 
   const vector<int> &F , const vector<int> &d, const vector<int> &Q, const vector<int> &s){

   vector<data> initialConfig = generateValidRandomConfig(numTrabalhos,numVeiculos,Q,s);

    vector<vehicleLoaded> vehicleOrder=generateVehicleOrder(initialConfig,numVeiculos);//Generating info about vehicle transportation
    vector<int> startVehicleTime(numVeiculos,0);//Calculating Delivery time(D) and Starting time(Sk)
    vector<int> deliveryTime = calculatingDeliveryTime(initialConfig,startVehicleTime,t,P,vehicleOrder,numTrabalhos);
    vector<int> jobTardiness = calculatingJobTardiness(deliveryTime,numTrabalhos,d);//Generating Job Tardiness (T) of each job (O(N))
    double iniObj= objFunction(vehicleOrder,initialConfig,numVeiculos,numTrabalhos,t,w,jobTardiness,F);
    

    #ifdef PRINT_VND_IMPROVE
    cout<<"\n Started VND at obj: "<<iniObj<<"\n\n";
    #endif

    vector<int> neighbors = { 1,2,3,4};


    random_shuffle( neighbors.begin(), neighbors.end());

    int it = 0;
    
    while( it < 3 ){

    	//1 corrigida, 2 e 3 parecem tb. Analisar a 4

        int whichNeighbor = neighbors[ it ];

        bool improved = false;

        if( whichNeighbor == 1){
            improved = vizi1_BI(initialConfig, vehicleOrder, numTrabalhos, numVeiculos, w, P, t, F, d);

        }else if( whichNeighbor == 2){
            improved = vizi2_BI(initialConfig, vehicleOrder, numTrabalhos, numVeiculos, w, P, t, F, d ,Q, s);

        }else if( whichNeighbor == 3){
            improved = vizi3_BI(initialConfig, vehicleOrder, numTrabalhos, numVeiculos, w, P, t, F, d);

        }else if(whichNeighbor ==4){
            improved = vizi4_BI(initialConfig, vehicleOrder, numTrabalhos, numVeiculos, w, P, t, F, d ,Q, s);
        }

        if( improved ){
            it = 0;
            random_shuffle( neighbors.begin(), neighbors.end());
        }else{
            it++;
        }

    }
    
    vehicleOrder=generateVehicleOrder(initialConfig,numVeiculos);//Generating info about vehicle transportation
    deliveryTime = calculatingDeliveryTime(initialConfig,startVehicleTime,t,P,vehicleOrder,numTrabalhos);
    jobTardiness = calculatingJobTardiness(deliveryTime,numTrabalhos,d);//Generating Job Tardiness (T) of each job (O(N))
    
    double finalObj= objFunction(vehicleOrder,initialConfig,numVeiculos,numTrabalhos,t,w,jobTardiness,F);

    #ifdef PRINT_VND_IMPROVE
    cout<<"Finished VND at obj: "<<finalObj<<"\n";
    #endif

    printConfig(finalObj,"VND - iniRandom",initialConfig, Q, s);

}
void graspV1(int numTrabalhos, int numVeiculos, const vector<double> &w,const vector<int> &P, const vector<vector<int>> &t, 
	const vector<int> &F , const vector<int> &d, const vector<int> &Q, const vector<int> &s, int nbiter){


		double bestObj;
		vector<data> bestConfig;
		int iterBestFound=0;
		for(int i=0; i< nbiter; i++){

		    vector<data> initialConfig = generateValidRandomConfig(numTrabalhos,numVeiculos,Q,s);

		    vector<vehicleLoaded> vehicleOrder=generateVehicleOrder(initialConfig,numVeiculos);//Generating info about vehicle transportation
		    vector<int> startVehicleTime(numVeiculos,0);//Calculating Delivery time(D) and Starting time(Sk)
		    vector<int> deliveryTime = calculatingDeliveryTime(initialConfig,startVehicleTime,t,P,vehicleOrder,numTrabalhos);
		    vector<int> jobTardiness = calculatingJobTardiness(deliveryTime,numTrabalhos,d);//Generating Job Tardiness (T) of each job (O(N))

		    double resultObjFunction= objFunction(vehicleOrder,initialConfig,numVeiculos,numTrabalhos,t,w,jobTardiness,F);


		    if( i == 0 ){
		    	bestObj = resultObjFunction;
		    	bestConfig = initialConfig;
		    	// cout<<" Started with: "<<bestObj<<endl;
		    }

		    bool generalImprove = true;
		    while(generalImprove){

		     	bool imp1,imp2,imp3,imp4;

		     	imp1 = vizi1_FI(initialConfig, vehicleOrder, numTrabalhos, numVeiculos, w, P, t, F, d );
		     	imp2 = vizi2_FI(initialConfig, vehicleOrder, numTrabalhos, numVeiculos, w, P, t, F, d , Q, s);
		     	imp3 = vizi3_FI(initialConfig, vehicleOrder, numTrabalhos, numVeiculos, w, P, t, F, d );
		     	imp4 = vizi4_FI(initialConfig, vehicleOrder, numTrabalhos, numVeiculos, w, P, t, F, d , Q, s);

		     	generalImprove = (imp1 || imp2 || imp3 || imp4);
		    }
				
		    vehicleOrder=generateVehicleOrder(initialConfig,numVeiculos);//Generating info about vehicle transportation
		  	deliveryTime = calculatingDeliveryTime(initialConfig,startVehicleTime,t,P,vehicleOrder,numTrabalhos);
		   	jobTardiness = calculatingJobTardiness(deliveryTime,numTrabalhos,d);//Generating Job Tardiness (T) of each job (O(N))


		    double newObj= objFunction(vehicleOrder,initialConfig,numVeiculos,numTrabalhos,t,w,jobTardiness,F);
		    if( newObj < bestObj){
		    	iterBestFound = i;
		    	bestObj = newObj;
		    	bestConfig = initialConfig;
		    }
		}

        printConfig(bestObj,"Grasp rand100x ",bestConfig, Q, s);
        // cout<<"Stop getting better at: "<<iterBestFound<<" iteration"<<endl<<endl;

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
 
    while(in>>numInstancia){
 
        in>>varMi>>sigmaDois;
        in>>numTrabalhos;
        in>>numVeiculos;
         
 
        char instancia[50];
        sprintf(instancia, "%d_%d_%d_%.1lf_%.1lf", numInstancia, numTrabalhos, numVeiculos, varMi, sigmaDois);
        cout<<instancia<<endl;
        vector<int> F(numVeiculos); //custo do veiculo
        vector<int> Q(numVeiculos); //capacidade do veiculo
      
        //0- Declaracao de variaveis e vetores constantes
        vector<vector<int> > t(numTrabalhos+1,vector<int>(numTrabalhos+1,0)); //tempo da viagem
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
        for(int i=0;i<numTrabalhos;i++)
            in>>w[i];
        for(int i=0;i<numVeiculos;i++)
            in>>Q[i];
        for(int i=0;i<numVeiculos;i++)
            in>>F[i];
        for(int i=0;i<=numTrabalhos;i++){
            for(int j=0;j<=numTrabalhos;j++){
                in>>t[i][j];
            }
        }
    
        /*
   
        //Generating RANDOM solution
        vector<data> initialConfig = generateValidRandomConfig(numTrabalhos,numVeiculos,Q,s);
 
        vector<vehicleLoaded> vehicleOrder=generateVehicleOrder(initialConfig,numVeiculos);//Generating info about vehicle transportation
        vector<int> startVehicleTime(numVeiculos,0);//Calculating Delivery time(D) and Starting time(Sk)
        vector<int> deliveryTime = calculatingDeliveryTime(initialConfig,startVehicleTime,t,P,vehicleOrder,numTrabalhos);
        vector<int> jobTardiness = calculatingJobTardiness(deliveryTime,numTrabalhos,d);//Generating Job Tardiness (T) of each job (O(N))
 
        double resultObjFunction= objFunction(vehicleOrder,initialConfig,numVeiculos,numTrabalhos,t,w,jobTardiness,F);


        bool imp1 = true;
         while(imp1){
             imp1 = vizi1_FI(initialConfig, vehicleOrder, numTrabalhos, numVeiculos, w, P, t, F, d );
            
         }
        bool imp2 = true;
        while(imp2){
            imp2 = vizi2_FI(initialConfig, vehicleOrder, numTrabalhos, numVeiculos, w, P, t, F, d ,Q, s);
        }
        
        bool imp3 = true;
        while(imp3){
            imp3 = vizi3_FI(initialConfig, vehicleOrder, numTrabalhos, numVeiculos, w, P, t, F, d );
        }
        
        bool imp4 = true;
        while(imp4){
            imp4 = vizi4_FI(initialConfig, vehicleOrder, numTrabalhos, numVeiculos, w, P, t, F, d , Q, s);
        }
        
 		
        vehicleOrder=generateVehicleOrder(initialConfig,numVeiculos);//Generating info about vehicle transportation
      	deliveryTime = calculatingDeliveryTime(initialConfig,startVehicleTime,t,P,vehicleOrder,numTrabalhos);
       	jobTardiness = calculatingJobTardiness(deliveryTime,numTrabalhos,d);//Generating Job Tardiness (T) of each job (O(N))
 

        double newObj= objFunction(vehicleOrder,initialConfig,numVeiculos,numTrabalhos,t,w,jobTardiness,F);
        printConfig(newObj,"Random - dr0",initialConfig, Q, s);

        */

        // graspV1(numTrabalhos, numVeiculos, w, P, t, F, d , Q, s, 100);

        VND(numTrabalhos, numVeiculos, w, P, t, F, d , Q, s);


        /*
       
        //ATC Configuration! Dispatching Rule 1:
        vector<data> atcConfig = generateATC(s, numTrabalhos, numVeiculos, w, P, d, F, Q);
 
        vehicleOrder = generateVehicleOrder(atcConfig,numVeiculos);
        deliveryTime = calculatingDeliveryTime(atcConfig,startVehicleTime,t,P,vehicleOrder,numTrabalhos);
        jobTardiness = calculatingJobTardiness(deliveryTime,numTrabalhos,d);
 
        double resultAtcFunction = objFunction(vehicleOrder,atcConfig,numVeiculos,numTrabalhos,t,w,jobTardiness,F);
        printConfig(resultAtcFunction,"ATC - dr1",atcConfig, Q, s);
        //cout<<"After local search (route): "<< resultAtcFunction <<endl<<endl;
 
 
 
 
        //WMDD Configuration! Dispatching Rule 2:
        vector<data> wmddConfig = generateWMDD(s, numTrabalhos, numVeiculos, w, P, d, F, Q);
 
        vehicleOrder = generateVehicleOrder(wmddConfig,numVeiculos);
        deliveryTime = calculatingDeliveryTime(wmddConfig,startVehicleTime,t,P,vehicleOrder,numTrabalhos);
        jobTardiness = calculatingJobTardiness(deliveryTime,numTrabalhos,d);
         
        double resultWMDDFunction = objFunction(vehicleOrder,wmddConfig,numVeiculos,numTrabalhos,t,w,jobTardiness,F);
 
        printConfig(resultWMDDFunction,"WMDD - dr2",wmddConfig, Q, s);
        // cout<<"After local search (route): "<< resultWMDDFunction <<endl;
 
      //WMDD Configuration! Dispatching Rule 3:
        vector<data> weddConfig = generateWEDD(s, numTrabalhos, numVeiculos, w, P, d, F, Q);
 
        vehicleOrder = generateVehicleOrder(weddConfig,numVeiculos);
        deliveryTime = calculatingDeliveryTime(weddConfig,startVehicleTime,t,P,vehicleOrder,numTrabalhos);
        jobTardiness = calculatingJobTardiness(deliveryTime,numTrabalhos,d);
         
        double resultWEDDFunction = objFunction(vehicleOrder,weddConfig,numVeiculos,numTrabalhos,t,w,jobTardiness,F);
 
        printConfig(resultWEDDFunction,"WEDD - dr3",weddConfig, Q, s);
        //cout<<"After local search (route): "<< resultWEDDFunction <<endl;
        cout<<endl;
 
        */
    }
    in.close();
 
}
