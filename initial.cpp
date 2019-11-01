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
#define INF 987654321
using namespace std;
  
// #define DEBUG_MODE
 
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
    cout<<setw(13)<<fObj;
 
    int numJobs= jobSize.size();
 
    string procOrder;
 
    for(int j=0;j<config.size();j++){
        if( config[j].job ){
            procOrder+= to_string( config[j].id );
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
            deliveryOrder+= to_string( config[i].id)+" ";
        }else{
            deliveryOrder+= " ["+ to_string(config[i].id) + "] ";
        }
    }
    for(int i=1; i<deliveryOrder.size()-1;i++){
        if( deliveryOrder[i-1]>='0' &&  deliveryOrder[i-1]<='9' && deliveryOrder[i+1]>='0' && deliveryOrder[i+1]<='9' ){
            deliveryOrder[i] = '-';
        }
    }
    cout<<setw(numJobsFixo*(5.5))<<deliveryOrder<<endl;
}
 
void interLocalSearch( double &currObjValue, vector<data> &configuration, int nbiter, const vector<vector<int>> &time, const vector<vehicleLoaded> &vehicleOrder,
    const vector<int> &procTime, int numTrabalhos, int numVeiculos, const vector<double> &weight, const vector<int> &vCost,
    const vector<int> &dueDate){
    /*For this local search in the same car, we need to recalculate
        the job tardiness and the delivery time due to the new delivery scheme */
 
    //Saving the initial solution and objetive value
    vector<data> bestSolution = configuration;
    double bestObjValue = currObjValue;
 
    for( int it=0; it<nbiter; it++){
 
        //Current version: Swapping (0,1*carriedJobs +1) pairs in each car.
 
        for(int i=0; i< vehicleOrder.size(); i++){
 
            int finalPos = vehicleOrder[i].finalPos;
            int initialPos = vehicleOrder[i].initialPos;
 
            set<pair<int,int>> setSwap;
 
            if( (finalPos - initialPos) == 0){
 
                continue; //There is just only one job carried, no need to swap
 
            }
            int swapNumber= 0.1 *(vehicleOrder[i].finalPos-vehicleOrder[i].initialPos +1) +1;
 
            while(swapNumber--){
                int jobA = (rand()% (finalPos-initialPos)) + initialPos;
                int jobB = (rand()% (finalPos-initialPos)) + initialPos;
 
                pair<int,int> verify = make_pair(jobA,jobB);
 
                if( setSwap.find(verify)!=setSwap.end() ){ //This swap was already made
 
                    swapNumber++;
 
                }else{
 
                    //So we do the swap in the jobs carried by the vehicle
                    int tempId= configuration[jobA].id;
                    configuration[jobA].id = configuration[jobB].id;
                    configuration[jobB].id = tempId;
 
                    setSwap.insert( verify );
                    verify = make_pair( jobB, jobA ); //Saving simetric swap
                    setSwap.insert( verify );
                }
 
            }
 
        }
 
        vector<int> startVehicleTime(numVeiculos,0);
        vector<int> deliveryTime = calculatingDeliveryTime(configuration,startVehicleTime,time,procTime,vehicleOrder,numTrabalhos);
        vector<int> jobTardiness = calculatingJobTardiness(deliveryTime,numTrabalhos,dueDate);
        double resultFunction = objFunction(vehicleOrder,configuration,numVeiculos, numTrabalhos, time, weight, jobTardiness, vCost);
         
        if( resultFunction < bestObjValue ){ //Saving the best solution
 
            bestSolution = configuration;
            bestObjValue = resultFunction;
        }
 
    }
    configuration = bestSolution;
    currObjValue = bestObjValue;
}
 
bool ls1IntraVehicle(vector<data> &configuration, vector<vehicleLoaded> &vehicleOrder, int numTrabalhos, int numVeiculos, const vector<double> &w,
	const vector<int> &P, const vector<vector<int>> &t, const vector<int> &F , const vector<int> &d){
    //BEST IMPROVEMENT

	bool generalImprove = false; 


    vector<int> startVehicleTime(numVeiculos,0);
	vector<int> deliveryTime = calculatingDeliveryTime(configuration,startVehicleTime,t,P,vehicleOrder,numTrabalhos);
    vector<int> T = calculatingJobTardiness(deliveryTime,numTrabalhos,d);
	

	for(int i=0;i<vehicleOrder.size();i++){
 
		double currObj = objFVehicle(i, vehicleOrder,configuration, numVeiculos, numTrabalhos,t,w,T,F); 
		// Calculate the obj function value for each car

		double bestImpObj = currObj;
		vector<data> bestImpConfig = configuration;
		bool internalImprove = false;

		if( vehicleOrder[i].initialPos == vehicleOrder[i].finalPos ){
			continue; //There is no more than one job in the vehicle
		}
		for(int j=vehicleOrder[i].initialPos; j<vehicleOrder[i].finalPos; j++){

			for(int k= (j+1); k<=vehicleOrder[i].finalPos; k++){

				swap( configuration[ j ], configuration[ k ]);
				
				deliveryTime = calculatingDeliveryTime(configuration,startVehicleTime,t,P,vehicleOrder,numTrabalhos);
        		T = calculatingJobTardiness(deliveryTime,numTrabalhos,d);

				double newObj = objFVehicle(i, vehicleOrder,configuration, numVeiculos, numTrabalhos,t,w,T,F); 

				if(newObj < bestImpObj){

					bestImpObj = newObj; 
					bestImpConfig = configuration;
					generalImprove = true;
					internalImprove = true;

				}
				//Swap back the jobs
				swap( configuration[ j ], configuration[ k ]);

			}

		}
		if( internalImprove){
			configuration = bestImpConfig;
			cout<<"Viz.1. => Melhora de "<< bestImpObj -  currObj <<endl;
		}
	}

	return generalImprove;
}

bool ls2InterVehicle(vector<data> &configuration, vector<vehicleLoaded> &vehicleOrder, int numTrabalhos, int numVeiculos, const vector<double> &w,
	const vector<int> &P, const vector<vector<int>> &t, const vector<int> &F , const vector<int> &d, const vector<int> &Q, const vector<int> &s){

	//FIRST IMPROVEMENT of BEST IMPROVEMENT in each 2 cars

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

		
		double bestObj = objFVehicle(i, vehicleOrder,configuration, numVeiculos, numTrabalhos,t,w,T,F); 
			   bestObj = objFVehicle(i+1, vehicleOrder,configuration, numVeiculos, numTrabalhos,t,w,T,F); 

		double currObj = bestObj;

		vector<data> bestConfig = configuration;

		for(int j= vehicleOrder[i].initialPos; j<= vehicleOrder[i].finalPos; j++ ){

			for(int k= vehicleOrder[i+1].initialPos; k<= vehicleOrder[i+1].finalPos; k++){

				//Checking if the capacity of each car will be respected

				if( (currCap[ vehicleOrder[ i ].id]- s[ configuration[j].id ]+ s[ configuration[k].id ]) <= Q[ vehicleOrder[i].id]
					&& (currCap[ vehicleOrder[ i+1 ].id] - s[configuration[k].id]+ s[ configuration[j].id]) <= Q[ vehicleOrder[i+1].id] ){

					swap( configuration[j], configuration[k]);

					deliveryTime = calculatingDeliveryTime(configuration,startVehicleTime,t,P,vehicleOrder,numTrabalhos);
    				T = calculatingJobTardiness(deliveryTime,numTrabalhos,d);
					
					double newDoubleConfig = objFVehicle(i, vehicleOrder,configuration, numVeiculos, numTrabalhos,t,w,T,F); 
			   		newDoubleConfig = objFVehicle(i+1, vehicleOrder,configuration, numVeiculos, numTrabalhos,t,w,T,F); 

			   		if( newDoubleConfig < bestObj ){

			   			bestObj = newDoubleConfig;
			   			bestConfig = configuration;

			   		}
					swap( configuration[j], configuration[k]);

				}
			}
		}
		if (bestObj < currObj){

			configuration = bestConfig;
			cout<<"Vz.2. => Melhora de "<<bestObj-currObj<<endl;
			return true;

		}

	}

	return false;
}

bool ls3IntraVehicle(vector<data> &configuration, vector<vehicleLoaded> &vehicleOrder, int numTrabalhos, int numVeiculos, const vector<double> &w,
	const vector<int> &P, const vector<vector<int>> &t, const vector<int> &F , const vector<int> &d){

    //FIRST IMPROVEMENT

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
		// cout<<"original: ";
		// for(int u:original){
		// 	cout<<u<<" ";
		// }
		// cout<<endl;
		for(int j= vehicleOrder[i].initialPos; j< vehicleOrder[i].finalPos; j++){

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

					cout<<"Vz.3. => Melhoria de "<<newObj-bestObj<<endl;
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

					cout<<"Vz.3. => Melhoria de "<<newObj-bestObj<<endl;
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

bool ls4InterVehicle(vector<data> &configuration, vector<vehicleLoaded> &vehicleOrder, int numTrabalhos, int numVeiculos, const vector<double> &w,
    const vector<int> &P, const vector<vector<int>> &t, const vector<int> &F , const vector<int> &d, const vector<int> &Q, const vector<int> &s){

    //Calculating the current car volume
    vector<int> currCap( numVeiculos, 0 );
    for(int i=0;i< vehicleOrder.size(); i++){

        int whichVehicle = vehicleOrder[i].id;

        for(int j=vehicleOrder[i].initialPos; j<= vehicleOrder[i].finalPos; j++){
            currCap[ whichVehicle ] += s[ configuration[j].id ];
        }

    }

    for(int i=0; i<vehicleOrder.size(); i++){

        for(int j= vehicleOrder[i].initialPos; j<= vehicleOrder[j].finalPos; j++){

            //continuar

        }
    }


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
 
   
        //Generating RANDOM solution
        vector<data> initialConfig = generateValidRandomConfig(numTrabalhos,numVeiculos,Q,s);
 
        vector<vehicleLoaded> vehicleOrder=generateVehicleOrder(initialConfig,numVeiculos);//Generating info about vehicle transportation
        vector<int> startVehicleTime(numVeiculos,0);//Calculating Delivery time(D) and Starting time(Sk)
        vector<int> deliveryTime = calculatingDeliveryTime(initialConfig,startVehicleTime,t,P,vehicleOrder,numTrabalhos);
        vector<int> jobTardiness = calculatingJobTardiness(deliveryTime,numTrabalhos,d);//Generating Job Tardiness (T) of each job (O(N))
 
        double resultObjFunction= objFunction(vehicleOrder,initialConfig,numVeiculos,numTrabalhos,t,w,jobTardiness,F);
        printConfig(resultObjFunction,"Random - dr0",initialConfig, Q, s);
        // interLocalSearch(resultObjFunction, initialConfig, 200, t, vehicleOrder,P, numTrabalhos, numVeiculos, w,F,d );
        // cout<<"After local search (route): "<< resultObjFunction <<endl<<endl;
 
        int tries=30000;

        while(tries--){
            bool any = false;
            if(ls3IntraVehicle(initialConfig, vehicleOrder, numTrabalhos, numVeiculos, w, P, t, F, d )
                || ls1IntraVehicle(initialConfig, vehicleOrder, numTrabalhos, numVeiculos, w, P, t, F, d ) 
                || ls2InterVehicle(initialConfig, vehicleOrder, numTrabalhos, numVeiculos, w, P, t, F, d , Q, s)){
                any=true;
            }

            if(any)
                tries=30000;
        }
        // ls3IntraVehicle(initialConfig, vehicleOrder, numTrabalhos, numVeiculos, w, P, t, F, d )<<endl;

        // ls1IntraVehicle(initialConfig, vehicleOrder, numTrabalhos, numVeiculos, w, P, t, F, d )<<endl;

        // ls2InterVehicle(initialConfig, vehicleOrder, numTrabalhos, numVeiculos, w, P, t, F, d , Q, s)<<endl;



      	deliveryTime = calculatingDeliveryTime(initialConfig,startVehicleTime,t,P,vehicleOrder,numTrabalhos);
       jobTardiness = calculatingJobTardiness(deliveryTime,numTrabalhos,d);//Generating Job Tardiness (T) of each job (O(N))
 
        double newObj= objFunction(vehicleOrder,initialConfig,numVeiculos,numTrabalhos,t,w,jobTardiness,F);

        cout<<"New Obj. F. is "<<newObj<<endl;




        /*
        #ifdef DEBUG_MODE //Testing the delivery time and the Starting time
             
            cout<<"Job sizes: "<<endl;
            for(int i=0;i<numTrabalhos;i++){
                cout<<"Job "<<i<<" size: "<<s[i]<<endl;
            }
 
            cout<<"Vehicle cap: "<<endl;
            for(int i=0;i<numVeiculos;i++){
                cout<<"Vehicle "<<i<<" cap: "<<Q[i]<<endl;
            }
 
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
        #endif
        //ATC Configuration! Dispatching Rule 1:
        vector<data> atcConfig = generateATC(s, numTrabalhos, numVeiculos, w, P, d, F, Q);
 
        vehicleOrder = generateVehicleOrder(atcConfig,numVeiculos);
        deliveryTime = calculatingDeliveryTime(atcConfig,startVehicleTime,t,P,vehicleOrder,numTrabalhos);
        jobTardiness = calculatingJobTardiness(deliveryTime,numTrabalhos,d);
 
        double resultAtcFunction = objFunction(vehicleOrder,atcConfig,numVeiculos,numTrabalhos,t,w,jobTardiness,F);
        printConfig(resultAtcFunction,"ATC - dr1",atcConfig, Q, s);
        //interLocalSearch(resultAtcFunction, atcConfig, 200, t, vehicleOrder,P, numTrabalhos, numVeiculos, w,F,d );
        //cout<<"After local search (route): "<< resultAtcFunction <<endl<<endl;
 
 
 
 
        //WMDD Configuration! Dispatching Rule 2:
        vector<data> wmddConfig = generateWMDD(s, numTrabalhos, numVeiculos, w, P, d, F, Q);
 
        vehicleOrder = generateVehicleOrder(wmddConfig,numVeiculos);
        deliveryTime = calculatingDeliveryTime(wmddConfig,startVehicleTime,t,P,vehicleOrder,numTrabalhos);
        jobTardiness = calculatingJobTardiness(deliveryTime,numTrabalhos,d);
         
        double resultWMDDFunction = objFunction(vehicleOrder,wmddConfig,numVeiculos,numTrabalhos,t,w,jobTardiness,F);
 
        printConfig(resultWMDDFunction,"WMDD - dr2",wmddConfig, Q, s);
        // interLocalSearch(resultWMDDFunction, wmddConfig, 200, t, vehicleOrder,P, numTrabalhos, numVeiculos, w,F,d );
        // cout<<"After local search (route): "<< resultWMDDFunction <<endl;
 
      //WMDD Configuration! Dispatching Rule 3:
        vector<data> weddConfig = generateWEDD(s, numTrabalhos, numVeiculos, w, P, d, F, Q);
 
        vehicleOrder = generateVehicleOrder(weddConfig,numVeiculos);
        deliveryTime = calculatingDeliveryTime(weddConfig,startVehicleTime,t,P,vehicleOrder,numTrabalhos);
        jobTardiness = calculatingJobTardiness(deliveryTime,numTrabalhos,d);
         
        double resultWEDDFunction = objFunction(vehicleOrder,weddConfig,numVeiculos,numTrabalhos,t,w,jobTardiness,F);
 
        printConfig(resultWEDDFunction,"WEDD - dr3",weddConfig, Q, s);
        //interLocalSearch(resultWEDDFunction, wmddConfig, 200, t, vehicleOrder,P, numTrabalhos, numVeiculos, w,F,d );
        //cout<<"After local search (route): "<< resultWEDDFunction <<endl;
        cout<<endl;
 
        */
    }
    in.close();
 
}
