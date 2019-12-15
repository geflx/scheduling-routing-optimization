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
#include <unordered_map>

#define EPS 1e-5

/* --- Defines to better a view of decreasing and debugging --- */

#define SHEETS_MODE
// #define VIEW_ILS_VND_IMPROVE
// #define VIEW_ILS_RVND_IMPROVE
// #define VIEW_OBJ_DECREASE
// #define VIEW_DECREASE
// #define PRINT_VND_IMPROVE
// #define PRINT_RVND_IMPROVE
// #define DEBUG_MODE

/*  --- Debugging words used to better found warnings ---

    1-Search for "Deletable" to cleanup code
    2-Search for "Danger" to look for invalid erros to fix
    3-Search for "Optimize" to improve methods
    4-Search "Caution" to search for unitialized variables (but tested)
*/

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

double objFunction(const vector<vehicleLoaded> &vehicleOrder, const vector<data> &configuration,int ncars, int njobs, const vector<vector<int> > &time,
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

    for(int i=0;i<njobs;i++){

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

double objFVehicle(int i, const vector<vehicleLoaded> &vehicleOrder, const vector<data> &configuration,int ncars, int njobs, const vector<vector<int> > &time,
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
    const vector<vehicleLoaded> &vehicleOrder,int njobs){
    vector<int> deliveryTime(njobs,0);
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

vector<vehicleLoaded> generateVehicleOrder(const vector<data> &config,int ncars){
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

vector<int> calculatingJobTardiness(const vector<int> &deliveryTime,int njobs, const vector<int> &dueDate){
    vector<int> T(njobs);
    for(int i=0;i<njobs;i++){
        T[i]=deliveryTime[i]-dueDate[i];

    	if(T[i]<0)
    		T[i]=0;
    }
    return T;
}

vector<data> generateRandomConfig(int njobs, int ncars){
    vector<data> configuration (njobs+ (ncars));
    int configSize = configuration.size();
    vector<bool> visitedConfig(configSize,false);
    vector<bool> visitedCar(ncars,false);


    int insertedVehicles=0;
    //Generating vehicles random positions
    while(insertedVehicles<ncars){
        int randomPos= rand()%configSize;
        while(visitedConfig[randomPos]) {
            randomPos=rand()%configSize;
        }
        if(!visitedConfig[0]){ //Putting in the config the first car to be used in position[0] of the vector.
            randomPos= rand()%ncars;
            configuration[0].job=false;
            configuration[0].id=randomPos;
            visitedConfig[0]=true;
            visitedCar[randomPos]=true;
            insertedVehicles++;
        }else{
            int randomVeic= rand()%ncars;
            while(visitedCar[randomVeic]) {
                randomVeic=rand()%ncars;
            }
            configuration[randomPos].job = false; //It is a vehicle, not a job.
            configuration[randomPos].id = randomVeic; //Identification of vehicle.
            visitedCar[randomVeic]=true;
            visitedConfig[randomPos]=true;
            insertedVehicles++;
        }
    }
    //Generating random positions for jobs
    for(int i=0;i<njobs;i++){
        int randomPos= rand()%configSize;
        while(visitedConfig[randomPos])
            randomPos=rand()%configSize;
        configuration[randomPos].job = true; //Yes, it is a job!
        configuration[randomPos].id = i; //Identification of jobs.
        visitedConfig[randomPos]=true;
    }
    return configuration;
}

vector<data> generateValidRandomConfig(int njobs, int ncars, const vector<int> &vehicleCap, const vector<int> &jobSizes){


    vector<data> configuration (njobs+ (ncars));
    bool validConfig=false;

    while(!validConfig){

        validConfig=true;

        int configSize = configuration.size();
        vector<bool> visitedConfig(configSize,false);
        vector<bool> visitedCar(ncars,false);


        int insertedVehicles=0;
        //Generating vehicles random positions
        while(insertedVehicles<ncars){
            int randomPos= rand()%configSize;
            while(visitedConfig[randomPos]) {
                randomPos=rand()%configSize;
            }
            if(!visitedConfig[0]){ //Putting in the config the first car to be used in position[0] of the vector.
                randomPos= rand()%ncars;
                configuration[0].job=false;
                configuration[0].id=randomPos;
                visitedConfig[0]=true;
                visitedCar[randomPos]=true;
                insertedVehicles++;
            }else{
                int randomVeic= rand()%ncars;
                while(visitedCar[randomVeic]) {
                    randomVeic=rand()%ncars;
                }
                configuration[randomPos].job = false; //It is a vehicle, not a job.
                configuration[randomPos].id = randomVeic; //Identification of vehicle.
                visitedCar[randomVeic]=true;
                visitedConfig[randomPos]=true;
                insertedVehicles++;
            }
        }
        //Generating random positions for jobs
        for(int i=0;i<njobs;i++){
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

vector<data> generateATC( const vector<int> &jobSize, int njobs, int ncars, const vector<double> &weight,
    const vector<int> &procTime, const vector<int> &dueDate, const vector<int> &carPrices, const vector<int> &carCap) {
    //Calculating processing time average
    double procTimeAvg=0;
    bool success= true; // If we generate a valid configuration

    for(int i=0;i<njobs;i++){

        procTimeAvg+=procTime[i];

    }
    procTimeAvg /= njobs;

    //Variables to generate processing order of jobs:
    vector<data> jobOrder(njobs);
    vector<bool> visitedJob(njobs,false);
    int acumulatedD=0;

    // cout<<"numtrbs"<<njobs<<endl;
    for(int i=0;i<njobs;i++){

        //Create priority queue of Dispatching Rule (ATC)
        priority_queue<pair<double, int> > atc;

        for(int j=0;j<njobs;j++){

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
    vector<data> vehicleDispatching(ncars);

    priority_queue<pair<int,int> > vehiclePrice; //We will select the cars in the cheapest order.

    for(int i=0; i<ncars; i++){

        vehiclePrice.push( make_pair( (-1) * carPrices[i], i ) ); //Ascending order ( *(-1) )

    }

    vector<data> configuration( njobs + ncars );
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
        while(insertedItems < configSize && jobToInsert<njobs && (currentVolume + jobSize[ jobOrder[jobToInsert].id ] <= carCap[ currentCar ]) ){


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

vector<data> generateWMDD( const vector<int> &jobSize, int njobs, int ncars, const vector<double> &weight,
    const vector<int> &procTime, const vector<int> &dueDate, const vector<int> &carPrices, const vector<int> &carCap) {

    //Variables to generate processing order of jobs:
    vector<data> jobOrder(njobs);
    vector<bool> visitedJob(njobs,false);
    int acumulatedD=0;

    for(int i=0;i<njobs;i++){

        //Create priority queue of Dispatching Rule (WMDD)
        priority_queue<pair<double, int> > wmdd;

        for(int j=0;j<njobs;j++){

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
    vector<data> vehicleDispatching(ncars);

    priority_queue<pair<int,int> > vehiclePrice; //We will select the cars in the cheapest order.

    for(int i=0; i<ncars; i++){

        vehiclePrice.push( make_pair( (-1) * carPrices[i], i ) ); //Ascending order ( *(-1) )

    }

    vector<data> configuration( njobs + ncars );
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
         while(insertedItems < configSize && jobToInsert<njobs && (currentVolume + jobSize[ jobOrder[jobToInsert].id ] <= carCap[ currentCar ]) ){


            configuration[ insertedItems ].job = true;
            configuration[ insertedItems ].id = jobOrder[ jobToInsert ].id;
            jobToInsert++;
            insertedItems++;

            currentVolume+= jobSize [  jobOrder[jobToInsert-1].id ];
        }

    }

    return configuration;
}

vector<data> generateWEDD( const vector<int> &jobSize, int njobs, int ncars, const vector<double> &weight,
    const vector<int> &procTime, const vector<int> &dueDate, const vector<int> &carPrices, const vector<int> &carCap) {

    //Variables to generate processing order of jobs:
    vector<data> jobOrder(njobs);
    vector<bool> visitedJob(njobs,false);

    for(int i=0;i<njobs;i++){

        //Create priority queue of Dispatching Rule (WEDD)
        priority_queue<pair<double, int> > wedd;

        for(int j=0;j<njobs;j++){

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
    vector<data> vehicleDispatching(ncars);

    priority_queue<pair<int,int> > vehiclePrice; //We will select the cars in the cheapest order.

    for(int i=0; i<ncars; i++){

        vehiclePrice.push( make_pair( (-1) * carPrices[i], i ) ); //Ascending order ( *(-1) )

    }

    vector<data> configuration( njobs + ncars );
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
         while(insertedItems < configSize && jobToInsert<njobs && (currentVolume + jobSize[ jobOrder[jobToInsert].id ] <= carCap[ currentCar ]) ){


            configuration[ insertedItems ].job = true;
            configuration[ insertedItems ].id = jobOrder[ jobToInsert ].id;
            jobToInsert++;
            insertedItems++;

            currentVolume+= jobSize [  jobOrder[jobToInsert-1].id ];
        }

    }

    return configuration;
}

bool nbhood1_FI(vector<data> &configuration, vector<vehicleLoaded> &vehicleOrder, int njobs, int ncars, const vector<double> &w,
	const vector<int> &P, const vector<vector<int>> &t, const vector<int> &F , const vector<int> &d){

    /* Change vehicle routing inside a vehicle .*/

    vector<int> startVehicleTime(ncars,0);
	vector<int> deliveryTime = calculatingDeliveryTime(configuration,startVehicleTime,t,P,vehicleOrder,njobs);
    vector<int> T = calculatingJobTardiness(deliveryTime,njobs,d);

	for(int i=0;i<vehicleOrder.size();i++){

		// Calculate the obj function value for each car

		double bestImpObj = objFVehicle(i, vehicleOrder,configuration, ncars, njobs,t,w,T,F);

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

				deliveryTime = calculatingDeliveryTime(configuration,startVehicleTime,t,P,vehicleOrder,njobs);
        		T = calculatingJobTardiness(deliveryTime,njobs,d);

				double newObj = objFVehicle(i, vehicleOrder,configuration, ncars, njobs,t,w,T,F);

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

bool nbhood1_BI(vector<data> &configuration, vector<vehicleLoaded> &vehicleOrder, int njobs, int ncars, const vector<double> &w,
    const vector<int> &P, const vector<vector<int>> &t, const vector<int> &F , const vector<int> &d){

    /* Change vehicle routing inside a vehicle .*/

    vector<int> startVehicleTime(ncars,0);
    vector<int> deliveryTime = calculatingDeliveryTime(configuration,startVehicleTime,t,P,vehicleOrder,njobs);
    vector<int> T = calculatingJobTardiness(deliveryTime,njobs,d);


    vector<data> bestConfig;
    bool improved = false;
    double biggestImprove = 0;

    double initialVObj = objFunction(vehicleOrder,configuration, ncars, njobs,t,w,T,F);
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

                deliveryTime = calculatingDeliveryTime(configuration,startVehicleTime,t,P,vehicleOrder,njobs);
                T = calculatingJobTardiness(deliveryTime,njobs,d);

                double newObj = objFunction(vehicleOrder,configuration, ncars, njobs,t,w,T,F);

                if(newObj < (bestObj - EPS)){
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
      // cout<<"Foi de "<<initialVObj<<" para "<<bestObj<<endl;
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

bool nbhood2_FI(vector<data> &configuration, vector<vehicleLoaded> &vehicleOrder, int njobs, int ncars, const vector<double> &w,
	const vector<int> &P, const vector<vector<int>> &t, const vector<int> &F , const vector<int> &d, const vector<int> &Q, const vector<int> &s){

	/* Job swap between vehicles. */

	vector<int> currCap( ncars, 0 );
	for(int i=0;i< vehicleOrder.size(); i++){

		int whichVehicle = vehicleOrder[i].id;

		for(int j=vehicleOrder[i].initialPos; j<= vehicleOrder[i].finalPos; j++){
			currCap[ whichVehicle ] += s[ configuration[j].id ];
		}

	}

	vector<int> startVehicleTime(ncars,0);
	vector<int> deliveryTime = calculatingDeliveryTime(configuration,startVehicleTime,t,P,vehicleOrder,njobs);
    vector<int> T = calculatingJobTardiness(deliveryTime,njobs,d);

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
				double bestObj = objFVehicle(i, vehicleOrder,configuration, ncars, njobs,t,w,T,F);
			    bestObj += objFVehicle(next, vehicleOrder,configuration, ncars, njobs,t,w,T,F);
				*/
				deliveryTime = calculatingDeliveryTime(configuration,startVehicleTime,t,P,vehicleOrder,njobs);
				T = calculatingJobTardiness(deliveryTime,njobs,d);

				double bestObj = objFunction(vehicleOrder,configuration, ncars, njobs,t,w,T,F);

				for(int k= vehicleOrder[next].initialPos; k<= vehicleOrder[next].finalPos; k++){

					//Checking if the capacity of each car will be respected

					if( (currCap[ vehicleOrder[ i ].id]- s[ configuration[j].id ]+ s[ configuration[k].id ]) <= Q[ vehicleOrder[i].id]
						&& (currCap[ vehicleOrder[ next ].id] - s[configuration[k].id]+ s[ configuration[j].id]) <= Q[ vehicleOrder[next].id] ){

						swap( configuration[j], configuration[k]);

						// deliveryTime = calculatingDeliveryTime(configuration,startVehicleTime,t,P,vehicleOrder,njobs);
						// T = calculatingJobTardiness(deliveryTime,njobs,d);



						deliveryTime = calculatingDeliveryTime(configuration,startVehicleTime,t,P,vehicleOrder,njobs);
					    T = calculatingJobTardiness(deliveryTime,njobs,d);



					 	/* Otimizar mais tarde.
						double newDoubleConfig = objFVehicle(i, vehicleOrder,configuration, ncars, njobs,t,w,T,F);
				   		newDoubleConfig += objFVehicle(next, vehicleOrder,configuration, ncars, njobs,t,w,T,F);
						*/

				   		double newDoubleConfig = objFunction(vehicleOrder,configuration, ncars, njobs,t,w,T,F);

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

bool nbhood2_BI(vector<data> &configuration, vector<vehicleLoaded> &vehicleOrder, int njobs, int ncars, const vector<double> &w,
    const vector<int> &P, const vector<vector<int>> &t, const vector<int> &F , const vector<int> &d, const vector<int> &Q, const vector<int> &s){

    /* Job swap between vehicles. */

    vector<int> currCap( ncars, 0 );
    for(int i=0;i< vehicleOrder.size(); i++){

        int whichVehicle = vehicleOrder[i].id;

        for(int j=vehicleOrder[i].initialPos; j<= vehicleOrder[i].finalPos; j++){
            currCap[ whichVehicle ] += s[ configuration[j].id ];
        }

    }

    vector<int> startVehicleTime(ncars,0);
    vector<int> deliveryTime = calculatingDeliveryTime(configuration,startVehicleTime,t,P,vehicleOrder,njobs);
    vector<int> T = calculatingJobTardiness(deliveryTime,njobs,d);


    double bestObj = objFunction(vehicleOrder,configuration, ncars, njobs,t,w,T,F);
    double iniObj = bestObj;
    bool improved = false;
    vector<data> bestConfig = configuration;

    for(int i=0; i< vehicleOrder.size()-1; i++){


        for(int j= vehicleOrder[i].initialPos; j<=vehicleOrder[i].finalPos; j++){


            for(int next= i+1; next<vehicleOrder.size(); next++){



                /* Otimizar mais tarde.
                double bestObj = objFVehicle(i, vehicleOrder,configuration, ncars, njobs,t,w,T,F);
                bestObj += objFVehicle(next, vehicleOrder,configuration, ncars, njobs,t,w,T,F);
                */
                deliveryTime = calculatingDeliveryTime(configuration,startVehicleTime,t,P,vehicleOrder,njobs);
                T = calculatingJobTardiness(deliveryTime,njobs,d);


                for(int k= vehicleOrder[next].initialPos; k<= vehicleOrder[next].finalPos; k++){

                    //Checking if the capacity of each car will be respected

                    if( (currCap[ vehicleOrder[ i ].id]- s[ configuration[j].id ]+ s[ configuration[k].id ]) <= Q[ vehicleOrder[i].id]
                        && (currCap[ vehicleOrder[ next ].id] - s[configuration[k].id]+ s[ configuration[j].id]) <= Q[ vehicleOrder[next].id] ){

                        swap( configuration[j], configuration[k]);

                        // deliveryTime = calculatingDeliveryTime(configuration,startVehicleTime,t,P,vehicleOrder,njobs);
                        // T = calculatingJobTardiness(deliveryTime,njobs,d);



                        deliveryTime = calculatingDeliveryTime(configuration,startVehicleTime,t,P,vehicleOrder,njobs);
                        T = calculatingJobTardiness(deliveryTime,njobs,d);



                        /* Otimizar mais tarde.
                        double newDoubleConfig = objFVehicle(i, vehicleOrder,configuration, ncars, njobs,t,w,T,F);
                        newDoubleConfig += objFVehicle(next, vehicleOrder,configuration, ncars, njobs,t,w,T,F);
                        */

                        double newDoubleConfig = objFunction(vehicleOrder,configuration, ncars, njobs,t,w,T,F);

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

bool nbhood3_FI(vector<data> &configuration, vector<vehicleLoaded> &vehicleOrder, int njobs, int ncars, const vector<double> &w,
	const vector<int> &P, const vector<vector<int>> &t, const vector<int> &F , const vector<int> &d){

    /* Inserting jobs inside a vehicle */

	vector<int> startVehicleTime(ncars,0);
	vector<int> deliveryTime = calculatingDeliveryTime(configuration,startVehicleTime,t,P,vehicleOrder,njobs);
    vector<int> T = calculatingJobTardiness(deliveryTime,njobs,d);

	for(int i=0; i< vehicleOrder.size();i++){

		if( vehicleOrder[i].initialPos == vehicleOrder[i].finalPos){
			continue;
		}
		double bestObj = objFVehicle(i, vehicleOrder,configuration, ncars, njobs,t,w,T,F);

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

				deliveryTime = calculatingDeliveryTime(configuration,startVehicleTime,t,P,vehicleOrder,njobs);
    			T = calculatingJobTardiness(deliveryTime,njobs,d);
				double newObj = objFVehicle(i, vehicleOrder,configuration, ncars, njobs,t,w,T,F);
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


				deliveryTime = calculatingDeliveryTime(configuration,startVehicleTime,t,P,vehicleOrder,njobs);
    			T = calculatingJobTardiness(deliveryTime,njobs,d);
				double newObj = objFVehicle(i, vehicleOrder,configuration, ncars, njobs,t,w,T,F);

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

bool nbhood3_BI(vector<data> &configuration, vector<vehicleLoaded> &vehicleOrder, int njobs, int ncars, const vector<double> &w,
    const vector<int> &P, const vector<vector<int>> &t, const vector<int> &F , const vector<int> &d){

    /* Inserting jobs inside a vehicle */

    vector<int> startVehicleTime(ncars,0);
    vector<int> deliveryTime = calculatingDeliveryTime(configuration,startVehicleTime,t,P,vehicleOrder,njobs);
    vector<int> T = calculatingJobTardiness(deliveryTime,njobs,d);


    double bestImprove = 0;
    bool improved = false;
    vector<data> bestConfig = configuration;

   double iniObj = objFunction(vehicleOrder,configuration, ncars, njobs,t,w,T,F);


    for(int i=0; i< vehicleOrder.size();i++){

        if( vehicleOrder[i].initialPos == vehicleOrder[i].finalPos){
            continue;
        }
        double bestObj = objFVehicle(i, vehicleOrder,configuration, ncars, njobs,t,w,T,F);

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

                deliveryTime = calculatingDeliveryTime(configuration,startVehicleTime,t,P,vehicleOrder,njobs);
                T = calculatingJobTardiness(deliveryTime,njobs,d);
                double newObj = objFVehicle(i, vehicleOrder,configuration, ncars, njobs,t,w,T,F);

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


                deliveryTime = calculatingDeliveryTime(configuration,startVehicleTime,t,P,vehicleOrder,njobs);
                T = calculatingJobTardiness(deliveryTime,njobs,d);
                double newObj = objFVehicle(i, vehicleOrder,configuration, ncars, njobs,t,w,T,F);

                 if( (newObj < (bestObj-EPS)) && (newObj-bestObj-EPS) < bestImprove) {

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

bool nbhood4_FI(vector<data> &configuration, vector<vehicleLoaded> &vehicleOrder, int njobs, int ncars, const vector<double> &w,
    const vector<int> &P, const vector<vector<int>> &t, const vector<int> &F , const vector<int> &d, const vector<int> &Q, const vector<int> &s){

    /* Inserting jobs in other vehicles (already in use and the others) */



    //Calculating the current car volume
    vector<int> currCap( ncars, 0 );
    vector<bool> carInUse( ncars, false);



    for(int i=0;i< vehicleOrder.size(); i++){

        int whichVehicle = vehicleOrder[i].id;

    	carInUse[ whichVehicle ] = true;

        for(int j=vehicleOrder[i].initialPos; j<= vehicleOrder[i].finalPos; j++){
            currCap[ whichVehicle ] += s[ configuration[j].id ];
        }

    }

   //Teste
 	vector<int> startVehicleTime(ncars,0);
    vector<int> deliveryTime = calculatingDeliveryTime(configuration,startVehicleTime,t,P,vehicleOrder,njobs);
    vector<int> T = calculatingJobTardiness(deliveryTime,njobs,d);

    double bestObj = objFunction(vehicleOrder,configuration, ncars, njobs,t,w,T,F);
    // bestObj += objFVehicle(tempBefore, vehicleOrder,configuration, ncars, njobs,t,w,T,F);

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

                        vector<vehicleLoaded> newVehicleOrder=generateVehicleOrder(newconfig,ncars);//Generating info about vehicle transportation



                        //Creating a new configuration

                        vector<int> newStartVehicleTime(ncars,0);
                        vector<int> newDeliveryTime = calculatingDeliveryTime(newconfig,newStartVehicleTime,t,P,newVehicleOrder,njobs);
                        vector<int> newT = calculatingJobTardiness(newDeliveryTime,njobs,d);

                        double newObj=0;
                        newObj = objFunction(newVehicleOrder,newconfig, ncars, njobs,t,w,newT,F);

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

                        vector<vehicleLoaded> newVehicleOrder=generateVehicleOrder(newconfig,ncars);//Generating info about vehicle transportation
                        //Creating a new configuration

                        vector<int> newStartVehicleTime(ncars,0);
                        vector<int> newDeliveryTime = calculatingDeliveryTime(newconfig,newStartVehicleTime,t,P,newVehicleOrder,njobs);
                        vector<int> newT = calculatingJobTardiness(newDeliveryTime,njobs,d);

                        double newObj=0;
                        newObj = objFunction(newVehicleOrder,newconfig, ncars, njobs,t,w,newT,F);

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


    for(int j=0; j<ncars; j++){

    	// cout<<"Tentando inserir nos carros vazios\n";

    	if(!carInUse[ j ]){

    		for(int i=0;i<vehicleOrder.size();i++){

		    	vector<int> startVehicleTime(ncars,0);
		        vector<int> deliveryTime = calculatingDeliveryTime(configuration,startVehicleTime,t,P,vehicleOrder,njobs);
		        vector<int> T = calculatingJobTardiness(deliveryTime,njobs,d);

		        double bestObj = objFunction(vehicleOrder,configuration, ncars, njobs,t,w,T,F);


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

		                vector<vehicleLoaded> newVehicleOrder=generateVehicleOrder(newConfig,ncars);//Generating info about vehicle transportation
		    			vector<int> newStartVehicleTime(ncars,0);
			        	vector<int> newDeliveryTime = calculatingDeliveryTime(newConfig,newStartVehicleTime,t,P,newVehicleOrder,njobs);
			        	vector<int> newT = calculatingJobTardiness(newDeliveryTime,njobs,d);

			        	double newObj = objFunction(newVehicleOrder,newConfig, ncars, njobs,t,w,newT,F);

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

bool nbhood4_BI(vector<data> &configuration, vector<vehicleLoaded> &vehicleOrder, int njobs, int ncars, const vector<double> &w,
    const vector<int> &P, const vector<vector<int>> &t, const vector<int> &F , const vector<int> &d, const vector<int> &Q, const vector<int> &s){

    /* Inserting jobs in other vehicles (already in use and the others) */



    //Calculating the current car volume
    vector<int> currCap( ncars, 0 );
    vector<bool> carInUse( ncars, false);

 //Teste
    vector<int> startVehicleTime(ncars,0);
    vector<int> deliveryTime = calculatingDeliveryTime(configuration,startVehicleTime,t,P,vehicleOrder,njobs);
    vector<int> T = calculatingJobTardiness(deliveryTime,njobs,d);

	double bestObj = objFunction(vehicleOrder,configuration, ncars, njobs,t,w,T,F);
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

                        vector<vehicleLoaded> newVehicleOrder=generateVehicleOrder(newconfig,ncars);//Generating info about vehicle transportation



                        //Creating a new configuration

                        vector<int> newStartVehicleTime(ncars,0);
                        vector<int> newDeliveryTime = calculatingDeliveryTime(newconfig,newStartVehicleTime,t,P,newVehicleOrder,njobs);
                        vector<int> newT = calculatingJobTardiness(newDeliveryTime,njobs,d);

                        double newObj=0;
                        newObj = objFunction(newVehicleOrder,newconfig, ncars, njobs,t,w,newT,F);

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



                                //corrigir
                                while(insert > 0  ){ //Danger: Gambiarra 1 pro cont
                                    // cerr<<"contInsertNet "<<contInsertNew<<" e posso "<<newconfig.size()<<endl;
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

                            if( contInsertNew> configuration.size() || copyConfig> configuration.size())
                                break;
                            newconfig[ contInsertNew ] = configuration[ copyConfig ];
                            contInsertNew++;
                            copyConfig++;

                        }

                        vector<vehicleLoaded> newVehicleOrder=generateVehicleOrder(newconfig,ncars);//Generating info about vehicle transportation
                        //Creating a new configuration

                        vector<int> newStartVehicleTime(ncars,0);
                        vector<int> newDeliveryTime = calculatingDeliveryTime(newconfig,newStartVehicleTime,t,P,newVehicleOrder,njobs);
                        vector<int> newT = calculatingJobTardiness(newDeliveryTime,njobs,d);

                        double newObj=0;
                        newObj = objFunction(newVehicleOrder,newconfig, ncars, njobs,t,w,newT,F);

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


    for(int j=0; j<ncars; j++){

        // cout<<"Tentando inserir nos carros vazios\n";

        if(!carInUse[ j ]){

            for(int i=0;i<vehicleOrder.size();i++){

                vector<int> startVehicleTime(ncars,0);
                vector<int> deliveryTime = calculatingDeliveryTime(configuration,startVehicleTime,t,P,vehicleOrder,njobs);
                vector<int> T = calculatingJobTardiness(deliveryTime,njobs,d);

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

                        vector<vehicleLoaded> newVehicleOrder=generateVehicleOrder(newConfig,ncars);//Generating info about vehicle transportation
                        vector<int> newStartVehicleTime(ncars,0);
                        vector<int> newDeliveryTime = calculatingDeliveryTime(newConfig,newStartVehicleTime,t,P,newVehicleOrder,njobs);
                        vector<int> newT = calculatingJobTardiness(newDeliveryTime,njobs,d);

                        double newObj = objFunction(newVehicleOrder,newConfig, ncars, njobs,t,w,newT,F);

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

//Swap entire vehicle (with all its jobs) with another ones
bool nbhood5(char bestOrFirst, vector<data> &config, vector<vehicleLoaded> &vehicleOrder, int njobs, int ncars, const vector<double> &w,
    const vector<int> &P, const vector<vector<int>> &t, const vector<int> &F , const vector<int> &d){


	vector<int> startVehicleTime(ncars,0);
    vector<int> deliveryTime = calculatingDeliveryTime(config,startVehicleTime,t,P,vehicleOrder,njobs);
    vector<int> T = calculatingJobTardiness(deliveryTime,njobs,d);

	double iniOfEverything = objFunction(vehicleOrder,config, ncars, njobs,t,w,T,F);
    double iniObj = iniOfEverything;
    bool improvedBest = false;

    //Caution, data not initialized
    vector<data> bestConfig;
    vector<vehicleLoaded> bestVOrder;

	for(int i=0;i< vehicleOrder.size()-1; i++){

		int nbJobs = vehicleOrder[i].finalPos - vehicleOrder[i].initialPos +1 ;
		vector<data> tempOut ( nbJobs + 1); // to fit vehicle

		tempOut[ 0 ] = config[ vehicleOrder[i].initialPos-1 ]; //copy vehicle
		for(int i1=1; i1<=nbJobs; i1++) //Copying configuration
			tempOut[ i1 ] = config[ vehicleOrder[i].initialPos +(i1-1) ];

		int vehicleOut = config[ vehicleOrder[i].initialPos-1 ].id; //get id of external vehicle

		for(int j=i+1; j<vehicleOrder.size(); j++){

			int nbJobsIn = vehicleOrder[j].finalPos - vehicleOrder[j].initialPos +1 ;
			vector<data> tempIn ( nbJobsIn + 1); // to fit vehicle

			tempIn[ 0 ] = config[ vehicleOrder[j].initialPos-1 ]; //copy vehicle
			for(int i1=1; i1<=nbJobsIn; i1++) //Copying configuration
				tempIn[ i1 ] = config[ vehicleOrder[j].initialPos +(i1-1)  ];

			int vehicleIn = config[ vehicleOrder[j].initialPos-1 ].id;


			// SWAP!

			int copy=0;
			int insert=0;

			vector<data> newconfig( config.size());


			while(copy < config.size()){


				if( !config[copy].job && config[copy].id!=vehicleOut && config[copy].id!=vehicleIn){

					int next = copy+1;

					newconfig[ insert++ ] = config[copy]; //copy vehicle

					while( next < config.size() && config[next].job){
						newconfig[ insert ] = config[ next ];
						next++; insert++;
					}
					copy = next;
				}else if( !config[copy].job && config[copy].id== vehicleOut){

					int cont=0;
					while( cont<=nbJobsIn ){
						newconfig[ insert ]= tempIn[ cont ];

						insert++; cont++;
					}
					//copy get size of entire vehicle out
					copy += nbJobs+1;

				}else if( !config[ copy].job && config[copy].id == vehicleIn){

					int cont=0;
					while( cont<=nbJobs ){
						newconfig[ insert ]= tempOut[ cont ];

						insert++; cont++;
					}
					//copy get size of entire vehicle out
					copy += nbJobsIn+1;
				}
			}



			//Checking new config OBJ!
    		vector<vehicleLoaded> newVehicleOrder=generateVehicleOrder(newconfig,ncars);
    		vector<int> newStartVehicleTime(ncars,0);
    		vector<int> newDeliveryTime = calculatingDeliveryTime(newconfig,newStartVehicleTime,t,P,newVehicleOrder,njobs);
    		vector<int> newT = calculatingJobTardiness(newDeliveryTime,njobs,d);

			double newObj = objFunction(newVehicleOrder,newconfig, ncars, njobs,t,w,newT,F);

			if( newObj < iniObj){


					if( bestOrFirst == 'F'){
	                    #ifdef VIEW_DECREASE
						cout<<"VZ5 FI: "<<newObj-iniObj<<endl;
	                    #endif
						#ifdef VIEW_OBJ_DECREASE
						cout<<"VZ5 FI: "<<newObj<<endl;
	                    #endif
						vehicleOrder = newVehicleOrder;
						config = newconfig;
						return true;
					}else{
						iniObj = newObj;
						bestConfig = newconfig;
						bestVOrder = newVehicleOrder;
						improvedBest = true;
					}
				}
		}
	}


	if( improvedBest && bestOrFirst == 'B'){

		config = bestConfig;
		vehicleOrder = bestVOrder;

		#ifdef VIEW_DECREASE
		cout<<"VZ5 BI: "<<iniObj- iniOfEverything<<endl;
        #endif
		#ifdef VIEW_OBJ_DECREASE
		cout<<"VZ5 BI: "<<iniObj<<endl;
        #endif
        return true;

	}
	return false;
}

//Insert entire vehicle (with all its jobs) in  another positions, except for the immediately before
bool nbhood6(char bestOrFirst, vector<data> &config, vector<vehicleLoaded> &vehicleOrder, int njobs, int ncars, const vector<double> &w,
    const vector<int> &P, const vector<vector<int>> &t, const vector<int> &F , const vector<int> &d){


	vector<int> startVehicleTime(ncars,0);
    vector<int> deliveryTime = calculatingDeliveryTime(config,startVehicleTime,t,P,vehicleOrder,njobs);
    vector<int> T = calculatingJobTardiness(deliveryTime,njobs,d);

	double iniOfEverything = objFunction(vehicleOrder,config, ncars, njobs,t,w,T,F);
    double iniObj = iniOfEverything;
    bool improvedBest = false;

    //Caution, data not initialized
    vector<data> bestConfig;
    vector<vehicleLoaded> bestVOrder;


    //Saving original car positions in the received config
    vector<int> carPos(ncars,-1);


    int contcar=0;
    // cout<<"Csize: "<<config.size()<<endl;
    for(int i=0;i<config.size(); i++){
    	if( !config[i].job){
    		carPos[contcar] = config[i].id;
    		contcar++;
    	}
    }

    unordered_set<int> inUse; //vehicles in use
	unordered_map<int,int> locate; //first == vehicle , second == position in array


    for(int i=0; i<vehicleOrder.size(); i++){
    	inUse.insert( vehicleOrder[i].id );
    	locate[ vehicleOrder[i].id ] = vehicleOrder[i].initialPos;
    }


   //  //Inserting by foward swapping cars adj cars!
    for(int i=0; i<ncars-1; i++){
    	//We will only insert cars that are in use!!!

    	if( inUse.find( carPos[i] ) == inUse.end())
    		continue; //not in use

    	vector<int> newCarPos = carPos;

    	for(int j=i+1; j<ncars; j++){

    		int carJ= newCarPos[j];
    		swap( newCarPos[i], newCarPos[j]);

    		//Generating this solution if car j is in use!
    		if( inUse.find( carJ) == inUse.end())
    			continue; //continue swapping, because this swap will not increase the obj function

    		//Generating the new solution

    		vector<data> newconfig( config.size() );
    		int insert = 0;

    		for(int k=0; k< ncars; k++){

    			if( inUse.find( newCarPos[k]) == inUse.end()){ //car not in use
    				newconfig[ insert ].job = false;
    				newconfig[ insert++].id = newCarPos[k];
    			}else{ //car is in use
    				newconfig[ insert ].job= false;
    				newconfig[ insert++ ].id = newCarPos[k];

    				int copy = locate[ newCarPos[k] ];
    				while( copy<config.size() && config[ copy ].job){
    					newconfig[ insert ].job = true;
    					newconfig[ insert++ ].id = config[copy].id;
    					copy++;
    				}
    			}
    		}

    		//Checking new config OBJ!
    		vector<vehicleLoaded> newVehicleOrder=generateVehicleOrder(newconfig,ncars);
    		vector<int> newStartVehicleTime(ncars,0);
    		vector<int> newDeliveryTime = calculatingDeliveryTime(newconfig,newStartVehicleTime,t,P,newVehicleOrder,njobs);
    		vector<int> newT = calculatingJobTardiness(newDeliveryTime,njobs,d);

			double newObj = objFunction(newVehicleOrder,newconfig, ncars, njobs,t,w,newT,F);

			if( newObj < iniObj){

					if( bestOrFirst == 'F'){
	                    #ifdef VIEW_DECREASE
						cout<<"VZ6 FI: "<<newObj-iniObj<<endl;
	                    #endif
						#ifdef VIEW_OBJ_DECREASE
						cout<<"VZ6 FI: "<<newObj<<endl;
	                    #endif
						vehicleOrder = newVehicleOrder;
						config = newconfig;
						return true;
					}else{
						iniObj = newObj;
						bestConfig = newconfig;
						bestVOrder = newVehicleOrder;
						improvedBest = true;
					}
			}
    	}
    }

    //Swap backward except for the immediatly before
    for(int i=ncars-1; i>0; i--){
    	//We will only insert cars that are in use!!!

    	if( inUse.find( carPos[i] ) == inUse.end())
    		continue; //not in use

    	vector<int> newCarPos = carPos;

    	for(int j=i-1; j>=0; j--){

    		int carJ= newCarPos[j];
    		swap( newCarPos[i], newCarPos[j]);

    		//Generating this solution if car j is in use!
    		if( j==(i-1) || inUse.find( carJ) == inUse.end())
    			continue; //continue swapping, because this swap will not increase the obj function

    		//Generating the new solution

    		vector<data> newconfig( config.size() );
    		int insert = 0;

    		for(int k=0; k< ncars; k++){

    			if( inUse.find( newCarPos[k]) == inUse.end()){ //car not in use
    				newconfig[ insert ].job = false;
    				newconfig[ insert++].id = newCarPos[k];
    			}else{ //car is in use
    				newconfig[ insert ].job= false;
    				newconfig[ insert++ ].id = newCarPos[k];

    				int copy = locate[ newCarPos[k] ];
    				while( copy<config.size() && config[ copy ].job){
    					newconfig[ insert ].job = true;
    					newconfig[ insert++ ].id = config[copy].id;
    					copy++;
    				}
    			}
    		}

    		//Checking new config OBJ!
    		vector<vehicleLoaded> newVehicleOrder=generateVehicleOrder(newconfig,ncars);
    		vector<int> newStartVehicleTime(ncars,0);
    		vector<int> newDeliveryTime = calculatingDeliveryTime(newconfig,newStartVehicleTime,t,P,newVehicleOrder,njobs);
    		vector<int> newT = calculatingJobTardiness(newDeliveryTime,njobs,d);

			double newObj = objFunction(newVehicleOrder,newconfig, ncars, njobs,t,w,newT,F);

			if( newObj < iniObj){

					if( bestOrFirst == 'F'){
	                    #ifdef VIEW_DECREASE
						cout<<"VZ6 FI.F: "<<newObj-iniObj<<endl;
	                    #endif
						#ifdef VIEW_OBJ_DECREASE
						cout<<"VZ6 FI.F: "<<newObj<<endl;
	                    #endif
						vehicleOrder = newVehicleOrder;
						config = newconfig;
						return true;
					}else{
						iniObj = newObj;
						bestConfig = newconfig;
						bestVOrder = newVehicleOrder;
						improvedBest = true;
					}
			}
    	}
    }

	if( improvedBest && bestOrFirst == 'B'){

		config = bestConfig;
		vehicleOrder = bestVOrder;

		#ifdef VIEW_DECREASE
		cout<<"VZ6 BI: "<<iniObj- iniOfEverything<<endl;
	       #endif
		#ifdef VIEW_OBJ_DECREASE
		cout<<"VZ6 BI: "<<iniObj<<endl;
	       #endif
	       return true;

	}
	return false;
}

//pair< ValueObjFunction, validSolution>
pair<double, vector<data>> RVND(bool reuse, int njobs, int ncars, const vector<double> &w,const vector<int> &P, const vector<vector<int>> &t,
   const vector<int> &F , const vector<int> &d, const vector<int> &Q, const vector<int> &s, vector<data> &initialConfig){

	if( !reuse ){
        initialConfig = generateValidRandomConfig(njobs,ncars,Q,s);
    }

	vector<int> interRoute = { 2, 4, 5, 6};
	random_shuffle( interRoute.begin(), interRoute.end());
	int it=0;

	vector<vehicleLoaded> vehicleOrder=generateVehicleOrder(initialConfig,ncars);//Generating info about vehicle transportation
    vector<int> startVehicleTime(ncars,0);//Calculating Delivery time(D) and Starting time(Sk)
    vector<int> deliveryTime = calculatingDeliveryTime(initialConfig,startVehicleTime,t,P,vehicleOrder,njobs);
    vector<int> jobTardiness = calculatingJobTardiness(deliveryTime,njobs,d);//Generating Job Tardiness (T) of each job (O(N))

    double bestObj = objFunction(vehicleOrder,initialConfig,ncars,njobs,t,w,jobTardiness,F);
    double iniOfEverything = bestObj;
    bool generalImprove = false;

    //Caution Unitialized data
    vector<data> bestConfig;

	while( it < 4){

		int neighbor = interRoute[ it ];
		bool improved = false;
    // cerr<<"Delete - Entrei no vizinho "<<neighbor<<"\n";
		switch(neighbor){
			case 2:
            	improved = nbhood2_BI(initialConfig, vehicleOrder, njobs, ncars, w, P, t, F, d, Q, s);
			case 4:
            	improved = nbhood4_BI(initialConfig, vehicleOrder, njobs, ncars, w, P, t, F, d ,Q, s);

			case 5:
            	improved = nbhood5('B',initialConfig, vehicleOrder, njobs, ncars, w, P, t, F, d);
			case 6:
            	improved = nbhood6('B',initialConfig, vehicleOrder, njobs, ncars, w, P, t, F, d);
		}
    // cerr<<"Delete - Sai do vizinho "<<neighbor<<"\n";

		if( improved ){
			generalImprove = true;

      // cerr<<"Delete - Entrei na intra LS\n";
			while( true ){ //intra route
				if( nbhood1_BI(initialConfig, vehicleOrder, njobs, ncars, w, P, t, F, d)){
          // cerr<<"Delete - Melhorei na nh1 continue!!\n";
					continue;
				}
				if( nbhood3_BI(initialConfig, vehicleOrder, njobs, ncars, w, P, t, F, d)){
          // cerr<<"Delete - Melhorei na nh3 continue!!\n";

					continue;
				}
				break;
			}
      // cerr<<"Delete - Sai da intra LS\n";


			//Caution: it is needed to recalculate vehicleORder? I guess not.
			#ifdef PRINT_RVND_IMPROVE
				vector<int> newstartVehicleTime(ncars,0);//Calculating Delivery time(D) and Starting time(Sk)
	    		vector<int> newdeliveryTime = calculatingDeliveryTime(initialConfig,newstartVehicleTime,t,P,vehicleOrder,njobs);
	    		vector<int> newjobTardiness = calculatingJobTardiness(newdeliveryTime,njobs,d);//Generating Job Tardiness (T) of each job (O(N))

	    		double newObj= objFunction(vehicleOrder,initialConfig,ncars,njobs,t,w,newjobTardiness,F);
	    		cout<< newObj - bestObj;
	    		bestObj = newObj;
    		#endif


			bestConfig = initialConfig;
			it = 0;
			random_shuffle( interRoute.begin(), interRoute.end());

		}else{
			it++;
		}
	}

	if( generalImprove)
		initialConfig = bestConfig;

	vehicleOrder=generateVehicleOrder(initialConfig,ncars);//Generating info about vehicle transportation
    deliveryTime = calculatingDeliveryTime(initialConfig,startVehicleTime,t,P,vehicleOrder,njobs);
    jobTardiness = calculatingJobTardiness(deliveryTime,njobs,d);//Generating Job Tardiness (T) of each job (O(N))
    double result= objFunction(vehicleOrder,initialConfig,ncars,njobs,t,w,jobTardiness,F);


	return {result, initialConfig};
}

//pair< ValueObjFunction, validSolution>
pair<double, vector<data>> VND(bool reuse, int njobs, int ncars, const vector<double> &w,const vector<int> &P, const vector<vector<int>> &t,
   const vector<int> &F , const vector<int> &d, const vector<int> &Q, const vector<int> &s, vector<data> &initialConfig){


    //Checking if it is needed to generate a new one or if the current solution will be used
    if( !reuse ){
        initialConfig = generateValidRandomConfig(njobs,ncars,Q,s);
    }

    vector<vehicleLoaded> vehicleOrder=generateVehicleOrder(initialConfig,ncars);//Generating info about vehicle transportation
    vector<int> startVehicleTime(ncars,0);//Calculating Delivery time(D) and Starting time(Sk)
    vector<int> deliveryTime = calculatingDeliveryTime(initialConfig,startVehicleTime,t,P,vehicleOrder,njobs);
    vector<int> jobTardiness = calculatingJobTardiness(deliveryTime,njobs,d);//Generating Job Tardiness (T) of each job (O(N))

    double iniObj= objFunction(vehicleOrder,initialConfig,ncars,njobs,t,w,jobTardiness,F);


    #ifdef PRINT_VND_IMPROVE
    cout<<"\n Started VND at obj: "<<iniObj<<"\n\n";
    #endif

    vector<int> neighbors = {1,2,3,4,5,6};


    random_shuffle( neighbors.begin(), neighbors.end());

    int it = 0;

    while( it < 6 ){

    	//1 corrigida, 2 e 3 parecem tb. Analisar a 4

        int whichNeighbor = neighbors[ it ];

        bool improved = false;

        if( whichNeighbor == 1){
            improved = nbhood1_BI(initialConfig, vehicleOrder, njobs, ncars, w, P, t, F, d);

        }else if( whichNeighbor == 2){
            improved = nbhood2_BI(initialConfig, vehicleOrder, njobs, ncars, w, P, t, F, d ,Q, s);

        }else if( whichNeighbor == 3){
            improved = nbhood3_BI(initialConfig, vehicleOrder, njobs, ncars, w, P, t, F, d);

        }else if(whichNeighbor ==4){
            improved = nbhood4_BI(initialConfig, vehicleOrder, njobs, ncars, w, P, t, F, d ,Q, s);
        }else if(whichNeighbor == 5){
            improved = nbhood5('B',initialConfig, vehicleOrder, njobs, ncars, w, P, t, F, d);
        }else if(whichNeighbor == 6){
            improved = nbhood6('B',initialConfig, vehicleOrder, njobs, ncars, w, P, t, F, d);
        }

        if( improved ){
            it = 0;
            random_shuffle( neighbors.begin(), neighbors.end());
        }else{
            it++;
        }

    }

    vehicleOrder=generateVehicleOrder(initialConfig,ncars);//Generating info about vehicle transportation
    deliveryTime = calculatingDeliveryTime(initialConfig,startVehicleTime,t,P,vehicleOrder,njobs);
    jobTardiness = calculatingJobTardiness(deliveryTime,njobs,d);//Generating Job Tardiness (T) of each job (O(N))

    double finalObj= objFunction(vehicleOrder,initialConfig,ncars,njobs,t,w,jobTardiness,F);

    #ifdef PRINT_VND_IMPROVE
    cout<<"Finished VND at obj: "<<finalObj<<"\n";
    #endif


    return {finalObj, initialConfig};

}

void graspV1(int njobs, int ncars, const vector<double> &w,const vector<int> &P, const vector<vector<int>> &t,
	const vector<int> &F , const vector<int> &d, const vector<int> &Q, const vector<int> &s, int nbiter){


		double bestObj;
		vector<data> bestConfig;
		int iterBestFound=0;
		for(int i=0; i< nbiter; i++){

		    vector<data> initialConfig = generateValidRandomConfig(njobs,ncars,Q,s);

		    vector<vehicleLoaded> vehicleOrder=generateVehicleOrder(initialConfig,ncars);//Generating info about vehicle transportation
		    vector<int> startVehicleTime(ncars,0);//Calculating Delivery time(D) and Starting time(Sk)
		    vector<int> deliveryTime = calculatingDeliveryTime(initialConfig,startVehicleTime,t,P,vehicleOrder,njobs);
		    vector<int> jobTardiness = calculatingJobTardiness(deliveryTime,njobs,d);//Generating Job Tardiness (T) of each job (O(N))

		    double resultObjFunction= objFunction(vehicleOrder,initialConfig,ncars,njobs,t,w,jobTardiness,F);


		    if( i == 0 ){
		    	bestObj = resultObjFunction;
		    	bestConfig = initialConfig;
		    	// cout<<" Started with: "<<bestObj<<endl;
		    }

		    bool generalImprove = true;
		    while(generalImprove){

		     	bool imp1,imp2,imp3,imp4;

		     	imp1 = nbhood1_FI(initialConfig, vehicleOrder, njobs, ncars, w, P, t, F, d );
		     	imp2 = nbhood2_FI(initialConfig, vehicleOrder, njobs, ncars, w, P, t, F, d , Q, s);
		     	imp3 = nbhood3_FI(initialConfig, vehicleOrder, njobs, ncars, w, P, t, F, d );
		     	imp4 = nbhood4_FI(initialConfig, vehicleOrder, njobs, ncars, w, P, t, F, d , Q, s);

		     	generalImprove = (imp1 || imp2 || imp3 || imp4);
		    }

		    vehicleOrder=generateVehicleOrder(initialConfig,ncars);//Generating info about vehicle transportation
		  	deliveryTime = calculatingDeliveryTime(initialConfig,startVehicleTime,t,P,vehicleOrder,njobs);
		   	jobTardiness = calculatingJobTardiness(deliveryTime,njobs,d);//Generating Job Tardiness (T) of each job (O(N))


		    double newObj= objFunction(vehicleOrder,initialConfig,ncars,njobs,t,w,jobTardiness,F);
		    if( newObj < bestObj){
		    	iterBestFound = i;
		    	bestObj = newObj;
		    	bestConfig = initialConfig;
		    }
		}

        // cout<<"Stop getting better at: "<<iterBestFound<<" iteration"<<endl<<endl;

}

bool validConfig( const vector<data> &config, const vector<int> &capacities, const vector<int> &jobSize, int njobs, int ncars ){


    int walk = 0;
    bool valid = true;

    unordered_set<int> jobsIn;
    unordered_set<int> carIn;

    for(int i=0; i<config.size(); i++){
    	if( config[i].id == -1)
    		return false;
    	if( config[i].job ){
    		if( jobsIn.find( config[i].id )== jobsIn.end() ){
    			jobsIn.insert( config[i].id );
    		}else{
    			return false; // job repeated
    		}
    		if( config[i].id<0 || config[i].id >=njobs)
    			return false;
    	}else{
    		if( carIn.find( config[i].id )== carIn.end() ){
    			carIn.insert( config[i].id );
    		}else{
    			return false; // car repeated
    		}

    		if( config[i].id<0 || config[i].id >= ncars)
    			return false;
    	}
    }

    while( walk < config.size() && valid){

        int walkAhead = walk;
        if( config[ walk ].job == false ){

            walkAhead = walk +1;

            if( walkAhead >= config.size() ){
                break;
            }

            int carCapacity = capacities[ config[ walk].id ];

            int accCap = 0;

            while( walkAhead<config.size() && config[ walkAhead ].job ){

                accCap += jobSize[ config[ walkAhead].id ];

                if( accCap > carCapacity){
                    valid=false;
                    break;
                }

                ++walkAhead;
            }

        }

        walk = walkAhead;
    }

    return valid;
}

void printConfig( double fObj, const char* message, const vector<data> &config, const vector<int> &carCap, const vector<int> &jobSize,
                  int njobs, int ncars, const vector<int> &processTime, const vector<vector<int>> &time, const vector<int> &dueDate,
                  const vector<double> &weight, const vector<int> &carPrices){


    vector<vehicleLoaded> vehicleOrder = generateVehicleOrder( config, ncars);
    vector<int> startVehicleTime(ncars,0);
    vector<int> deliveryTime = calculatingDeliveryTime(config ,startVehicleTime,time,processTime,vehicleOrder,njobs);
    vector<int> T = calculatingJobTardiness(deliveryTime,njobs,dueDate);

    if(fObj == 0) //Calculate
        fObj = objFunction(vehicleOrder,config, ncars, njobs,time,weight,T,carPrices);

    // The program will alert the dev and abort if the config to print is invalid.
    if( validConfig( config, carCap, jobSize, jobSize.size(), carCap.size()) == false){

        cerr<<"Tried to print INVALID Solution. Aborting... at Method: "<<message<<endl;
        exit(0);
    }



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

vector<int> carryWeight( vector<data> &config, const vector<int> &jobSize, int ncars){

    //Danger: confirmar se funciona
    vector<int> carry (ncars, 0);

    int walk= 0;

    while( walk < config.size()){

        int walkAhead = walk;
        if( config[ walk ].job == false ){

            walkAhead = walk +1;

            if( walkAhead >= config.size() ){
                break;
            }

            int accCap = 0;

            while( walkAhead<config.size() && config[ walkAhead ].job ){

                accCap += jobSize[ config[ walkAhead].id ];
                ++walkAhead;
            }
            carry[ config[ walk].id ] = accCap;

        }

        walk = walkAhead;
    }
    return carry;
}

vector<data> perturb ( vector<data> &solution, const vector<int> &capacities, const vector<int> &jobSize, int njobs, int ncars){

    int mode[3] = { 1,2 };
    double intensity[11]= { 0.5, 0.6, 0.7, 0.8, 0.9, 1.0, 1.1, 1.2, 1.3, 1.4, 1.5};

    double sizePerturb = njobs * intensity[ rand()%11 ];

    //rand()%3
    int whichMode = mode[ rand()%2 ];

    vector<int> carrying = carryWeight( solution, jobSize, ncars);

    vector<int> jobPos (njobs, -1);
    for(int i=0; i<solution.size(); i++)
        if( solution[i].job)
            jobPos[ solution[i].id ] = i;

    vector<int> dad( njobs, -1 );


    vector<vehicleLoaded> order = generateVehicleOrder( solution, ncars);

    //Saves the position of vehicle in the vehicleOrder array
    vector<int> posiVehicleOrder (ncars,-1);

    //Getting the car id of each job carried
    for(int i=0;i< order.size();i++){

        posiVehicleOrder[ order[i].id ]= i;

        for(int j=order[i].initialPos; j<=order[i].finalPos; j++){

        	if( solution[j].job){
	            int whichJob = solution[j].id;
	            dad[ whichJob ] = order[i].id;
	        }
        }
    }

    //Variable to not get stuck successivily while trying to move 2 jobs between cars
    int randomTries = 0;

    switch( whichMode ){

        case 1:
            /*Perturbation Swap(1,1) Permutation between a customer k from a route r1 and a customer l, from a route r2
                Only valid moves are allowed */


            for(int i=0; i<sizePerturb; i++){

                if(randomTries > 2*njobs){
                    // cout<<"Caution - Force break!! Tried njobs*2 times to swap between cars and get invalid move. \n";
                    break;
                }
                int first = rand()%njobs;
                int sec = rand()%njobs;

                //Optimize - adding a new data structure
                while( dad[first] == dad[sec]){
                    first = rand()%njobs;
                }
                int posiFirst = jobPos [ first ];
                int posiSec = jobPos [ sec ];

                int carFirst = dad[ first ];
                int carSec = dad[ sec ];


                if( capacities[carFirst]>= (carrying[carFirst]- jobSize[ first ]+ jobSize[ sec])
                &&   (capacities[carSec]>= (carrying[carSec]-jobSize[sec]+jobSize[first]) )){

                    //Updating car's carried weight
                    carrying[carFirst]= carrying[carFirst]- jobSize[ first ]+ jobSize[ sec];
                    carrying[carSec]= carrying[carSec]- jobSize[ sec ]+ jobSize[ first ];

                    //Updating carrying vehicles' id
                    swap( dad[ first ], dad[ sec ]);

                    //Swapping and updating
                    swap( solution[ posiFirst ], solution[ posiSec] );
                    swap( jobPos[ first ] , jobPos[ sec]);

                    randomTries = 0;

                }else{
                    i--; //Swap again
                    randomTries++;
                }


            }

         case 2:
             for(int i=0; i<sizePerturb; i++){

                 if(randomTries > 2*njobs){
                     // cout<<"Caution - Force break!! Tried njobs*2 times to swap between cars and get invalid move. \n";
                     break;
                 }
                 int first = rand()%njobs;
                 int sec = rand()%njobs;

                 //Optimize - adding a new data structure
                 while( dad[first] == dad[sec]){
                     first = rand()%njobs;
                 }
                 // cerr<<"Dad first and sec "<<dad[first]<<" "<<dad[sec]<<endl;

                 // int posiFirst = jobPos [ first ];
                 // int posiSec = jobPos [ sec ];

                 int carFirst = dad[ first ];
                 int carSec = dad[ sec ];


                 if( capacities[carFirst]>= (carrying[carFirst]- jobSize[ first ]+ jobSize[ sec])
                 &&   (capacities[carSec]>= (carrying[carSec]-jobSize[sec]+jobSize[first]) )){

                     //Updating car's carried weight
                     carrying[carFirst]= carrying[carFirst]- jobSize[ first ]+ jobSize[ sec];
                     carrying[carSec]= carrying[carSec]- jobSize[ sec ]+ jobSize[ first ];

                     //Updating carrying vehicles' id
                     swap( dad[ first ], dad[ sec ]);

                    //Create an array for both of vehicles
                    int carFirstPosiOrder = posiVehicleOrder[ carFirst ];
                    int carSecPosiOrder = posiVehicleOrder[ carSec ];

                    int beg1= order[ carFirstPosiOrder ].initialPos;
                    int beg2= order[ carSecPosiOrder ].initialPos;

                    int end1= order[ carFirstPosiOrder ].finalPos;
                    int end2= order[ carSecPosiOrder ].finalPos;

                    // cerr<<"posi v order "<<carFirstPosiOrder<<" "<<carSecPosiOrder<<" and v order size: "<<order.size()<<endl;
                    // cerr<<" posi first:: "<<beg1<<" "<<end1<<"   posi sec:: "<<beg2<<" "<<end2<<endl;
                    int sizeFirst = end1 - beg1 + 1;
                    int sizeSec = end2 - beg2 + 1;

                    // cerr<<"Size f e sec "<<sizeFirst<<" "<<sizeSec<<endl;
                    vector<int> jobsFirst (sizeFirst, -1);
                    vector<int> jobsSec (sizeSec, -1);

                    // cerr<<"size first e sec "<<sizeFirst<<" "<<sizeSec<<endl;
                    //Generating random positions to insert job in vehicle
                    int insertOne = rand()%sizeFirst;
                    int insertTwo = rand()%sizeSec;



                    // cerr<<"Fuck\n";
                    //Generating new array of jobs of vehicles!

                    jobsFirst[ insertOne ] = sec;
                    jobsSec[ insertTwo ] = first;

                    int in1=0;
                    int in2=0;
                    for(int k=beg1; k<=end1; k++){

                        if( jobsFirst[ in1 ]!= -1)
                            in1++;

                        if( solution[k].job && solution[k].id==first){
                            continue;
                        }else{
                            jobsFirst[ in1++ ] = solution[k].id;
                        }
                    }

                    for(int k=beg2; k<=end2; k++){

                        if( jobsSec[ in2 ]!= -1)
                            in2++;

                        if( solution[k].job && solution[k].id==sec){
                            continue;
                        }else{
                            jobsSec[ in2++ ] = solution[k].id;
                        }
                    }

                    // cout<<"Delete- JOB FIRST    ";
                    // for(int k: jobsFirst){
                    //     cout<<k<<" ";
                    // }cout<<endl;
                    //
                    // // cout<<"Delete- JOB SEC    ";
                    // for(int k: jobsSec){
                    //     cout<<k<<" ";
                    // }cout<<endl;
                    //Inserting the new sequences in solution
                    int walk = 0;
                    while( walk < solution.size()){

                        int walkAhead = walk + 1;

                        if( solution[ walk ].job == false){

                            if( solution[walk].id == carFirst){
                                int copyfirst = 0;
                                while( walkAhead < solution.size() && solution[ walkAhead ].job == true){
                                    solution[ walkAhead++ ].id = jobsFirst[ copyfirst++ ];
                                }

                            }else if( solution[walk].id == carSec ){
                                int copySec = 0;
                                while( walkAhead < solution.size() &&  solution[ walkAhead ].job == true){
                                    solution[ walkAhead++ ].id = jobsSec[ copySec++ ];
                                }

                            }else{ // some other car
                                while( walkAhead < solution.size() && solution[ walkAhead ].job == true){
                                    walkAhead++;
                                }
                            }

                        }
                        walk = walkAhead;
                   }

                     randomTries = 0;

                 }else{
                     i--; //Swap again
                     randomTries++;
                 }
             }

        // case 3:

    }

    return solution;
}

pair<double, vector<data>> ILS_VND(int njobs, int ncars, const vector<double> &w,const vector<int> &P, const vector<vector<int>> &t,
    const vector<int> &F , const vector<int> &d, const vector<int> &Q, const vector<int> &s, int maxIter, int maxIterIls){


    int nbValidSolutions = 0;
    vector< vector<data> > validSolutions;

    // Generating initial greedy solutions
    vector<data> atcConfig = generateATC(s, njobs, ncars, w, P, d, F, Q);
    vector<data> weddConfig = generateWEDD(s, njobs, ncars, w, P, d, F, Q);
    vector<data> wmddConfig = generateWMDD(s, njobs, ncars, w, P, d, F, Q);

    //Danger: Verify ATC!!!
    // Verifying if they are really valid
    if( validConfig( atcConfig, Q, s, njobs, ncars)){
        validSolutions.push_back( atcConfig );
        nbValidSolutions++;
    }
    if( validConfig( wmddConfig, Q, s, njobs, ncars)){
        validSolutions.push_back( wmddConfig);
        nbValidSolutions++;
    }
    if( validConfig( weddConfig, Q, s, njobs, ncars)){
        validSolutions.push_back( weddConfig);
        nbValidSolutions++;
    }

    int contGetSolution = 0;

    double bestResult = INF;
    vector<data> bestSolution;


    for(int a=0; a<maxIter; a++){

        vector<data> solution;
        pair< double, vector<data> > callVnd;

        //Getting the greedy solutions if there is one avaiable

        bool justCalledVnd = true;
        if( contGetSolution < nbValidSolutions ){

            solution = validSolutions[ contGetSolution ];

            callVnd = VND( true, njobs, ncars, w, P, t, F, d , Q, s, solution);

            contGetSolution++;
        }else{
        //Otherwise we generate a random one
            callVnd = VND( false, njobs, ncars, w, P, t, F, d , Q, s, solution);
         }

        for(int b=0; b< maxIterIls; b++){

            if( !justCalledVnd ){
                callVnd = VND( true, njobs, ncars, w, P, t, F, d , Q, s, solution);
            }else{
                justCalledVnd = false;
            }

            if( callVnd.first < bestResult){

                #ifdef VIEW_ILS_VND_IMPROVE
                    if(bestResult != INF ) // Printing only the improves caused by ILS
                        cout<<"// ILS_Improve:   "<<callVnd.first-bestResult <<" //\n";
                #endif
                bestResult = callVnd.first;
                bestSolution = callVnd.second;

                b=0; //Reseting ILS!!
            }
            solution = perturb( solution, Q, s, njobs, ncars);
        }
    }
 	if( validConfig( bestSolution, Q, s, njobs, ncars))
    	return {bestResult,bestSolution};
    else
    	return {-1,bestSolution};
}

pair<double, vector<data>> ILS_RVND(int njobs, int ncars, const vector<double> &w,const vector<int> &P, const vector<vector<int>> &t,
    const vector<int> &F , const vector<int> &d, const vector<int> &Q, const vector<int> &s, int maxIter, int maxIterIls){


    int nbValidSolutions = 0;
    vector< vector<data> > validSolutions;

    // Generating initial greedy solutions
    vector<data> atcConfig = generateATC(s, njobs, ncars, w, P, d, F, Q);
    vector<data> weddConfig = generateWEDD(s, njobs, ncars, w, P, d, F, Q);
    vector<data> wmddConfig = generateWMDD(s, njobs, ncars, w, P, d, F, Q);

    //Danger: Verify ATC!!!
    // Verifying if they are really valid
    if( validConfig( atcConfig, Q, s, njobs, ncars)){
        validSolutions.push_back( atcConfig );
        nbValidSolutions++;
    }
    if( validConfig( wmddConfig, Q, s, njobs, ncars)){
        validSolutions.push_back( wmddConfig);
        nbValidSolutions++;
    }
    if( validConfig( weddConfig, Q, s, njobs, ncars)){
        validSolutions.push_back( weddConfig);
        nbValidSolutions++;
    }

    int contGetSolution = 0;

    double bestResult = INF;
    vector<data> bestSolution;


    for(int a=0; a<maxIter; a++){

        // cout<<"Delete - Iter: "<<a<<endl;
        vector<data> solution;
        pair< double, vector<data> > callRVnd;

        //Getting the greedy solutions if there is one avaiable

        bool justCalledRVnd = true;
        if( contGetSolution < nbValidSolutions ){

            solution = validSolutions[ contGetSolution ];

            callRVnd = RVND( true, njobs, ncars, w, P, t, F, d , Q, s, solution);

            contGetSolution++;
        }else{
        //Otherwise we generate a random one
            callRVnd = RVND( false, njobs, ncars, w, P, t, F, d , Q, s, solution);
         }

        for(int b=0; b< maxIterIls; b++){

        	// cerr<<"B vale.\n";
            if( !justCalledRVnd ){
              // cerr<<"Entrei RVND.\n";

                callRVnd = RVND( true, njobs, ncars, w, P, t, F, d , Q, s, solution);
                // cerr<<"Sai RVND.\n";

            }else{
                justCalledRVnd = false;
            }

            if( callRVnd.first < bestResult){

                #ifdef VIEW_ILS_RVND_IMPROVE
                    if(bestResult != INF ) // Printing only the improves caused by ILS
                        cout<<"// ILS_Improve:   "<<callRVnd.first-bestResult <<" //\n";
                #endif
                bestResult = callRVnd.first;
                bestSolution = callRVnd.second;

                b=0; //Reseting ILS!!
            }
            // cerr<<"Delete - Entrei perturb.\n";
            solution = perturb( solution, Q, s, njobs, ncars);
            // cerr<<"Delete- Sai da perturb.\n";

        }
    }

    if( validConfig( bestSolution, Q, s, njobs, ncars))
    	return {bestResult,bestSolution};
    else
    	return {-1,bestSolution};
}

//pair< BOOL, solution > indicating if it is valid
pair< bool, vector<data> > crossOver( const vector<data> &f1, const vector<data> &f2,const vector<int> &capacities, const vector<int> &jobSize, int njobs, int ncars){

    //generating random interval to 2-point crossOver
    // cerr<<" Del - Started crossing\n ";

    int a = rand()%(f1.size());
    int b = rand() %(f1.size());

    while( a==b || a==(f1.size()-1) || (a==0) || (b==0) || (a>b)){
        a = rand()%(f1.size());
        b = rand() %(f1.size());
    }
    // cout<<"Del - A: "<<a<<" B: "<<b<<endl;
    unordered_set<int> jobsCross;
    unordered_set<int> carsCross;
    for(int i=a; i<=b; i++){ // i <= b?
        if( f1[i].job){
            jobsCross.insert( f1[i].id );
        }else{
            carsCross.insert( f1[i].id);
        }
    }

    // cout<<"Delete - I will start to copy\n";
    vector<data> son( f1.size() );

    vector<data> neworder;

    for(int i=0;i<f2.size();i++){
        if( f2[i].job){
            if( jobsCross.find( f2[i].id )!= jobsCross.end() ){ //job is int our cross
                 data temp;
                 temp.job = true;
                 temp.id = f2[i].id;
                 neworder.push_back( temp );
            }
        }else{
            if( carsCross.find( f2[i].id )!= carsCross.end() ){ //job is int our cross
                 data temp;
                 temp.job = false;
                 temp.id = f2[i].id;
                 neworder.push_back( temp );
            }
        }
    }

    int insert = 0;
    int copy = 0;

    while( insert< f1.size()){
        if( insert<a || insert>b){
            son[ insert ] = f1[ insert ];
        }else{
            son[ insert ] = neworder[ copy++ ];
        }
        insert++;
    }

    // for(data i: son){
    //     if( i.job){
    //         cout<<"J"<<i.id<<" ";
    //     }else{
    //         cout<<"V"<<i.id<<" ";
    //     }
    // }cout<<endl;

    // cout<<"Delete - I finished copying \n";

    // cout<<"Delete - start checking if it is valid\n";
    bool isvalid = validConfig( son, capacities, jobSize, njobs, ncars );
    // cout<<"Delete - finished checking if it is valid\n";

    // cerr<<"Del -  Finished crossing\n ";
    return { isvalid, son};
}

pair<double, vector<data>> fastLocalSearch(bool reuse, int njobs, int ncars, const vector<double> &w,const vector<int> &P, const vector<vector<int>> &t,
   const vector<int> &F , const vector<int> &d, const vector<int> &Q, const vector<int> &s, vector<data> &initialConfig){


    //Caution - there is something missing?
    vector<vehicleLoaded> vehicleOrder=generateVehicleOrder(initialConfig,ncars);//Generating info about vehicle transportation

    nbhood1_BI(initialConfig, vehicleOrder, njobs, ncars, w, P, t, F, d);
    nbhood2_BI(initialConfig, vehicleOrder, njobs, ncars, w, P, t, F, d ,Q, s);
    nbhood3_BI(initialConfig, vehicleOrder, njobs, ncars, w, P, t, F, d);
    nbhood4_BI(initialConfig, vehicleOrder, njobs, ncars, w, P, t, F, d ,Q, s);
    nbhood5('B',initialConfig, vehicleOrder, njobs, ncars, w, P, t, F, d);
    nbhood6('B',initialConfig, vehicleOrder, njobs, ncars, w, P, t, F, d);


    vehicleOrder=generateVehicleOrder(initialConfig,ncars);//Generating info about vehicle transportation
    vector<int> startVehicleTime(ncars,0);//Calculating Delivery time(D) and Starting time(Sk)
    vector<int> deliveryTime = calculatingDeliveryTime(initialConfig,startVehicleTime,t,P,vehicleOrder,njobs);
    vector<int> jobTardiness = calculatingJobTardiness(deliveryTime,njobs,d);//Generating Job Tardiness (T) of each job (O(N))
    double finalObj= objFunction(vehicleOrder,initialConfig,ncars,njobs,t,w,jobTardiness,F);

    return {finalObj, initialConfig};

}

pair<double, vector<data>> genAlgo1(int njobs, int ncars, const vector<double> &w,const vector<int> &P, const vector<vector<int>> &t,
    const vector<int> &F , const vector<int> &d, const vector<int> &Q, const vector<int> &s, int popSize){


        vector< double > popObj( popSize , -1);
        vector< vector<data> > pop( popSize );

        vector< data > bestConfig;
        double bestObj = INF;

        for(int i=0; i< popSize; i++){
            vector<data> trash; // will be use as a puppet to call function generateRandom
            pair<double, vector<data>> callVnd = VND( false, njobs, ncars, w, P, t, F, d , Q, s, trash);
            pop[ i ] = callVnd.second;
            popObj[ i ] = callVnd.first;

            if( popObj[ i ] < bestObj ){
                bestObj = popObj[ i ];
                bestConfig = pop[ i ];
            }
        }

        int maxIter = 8 * (njobs + ncars);
        int contIter = 0;
        while( contIter++ < maxIter){

            int newPopSize = 0;
            vector<vector<data>> newPop( popSize );
            vector< double> newPopObj( popSize, -1 );

            //It is (popSize-1) to carry the best solution
            while( newPopSize < (popSize-1)){

                int father1 = -1;
                int father2 = -1;

                // cout<<"Delete - gett new fathers\n";
                // Generatin fathers 1 and 2 by tournament
                while( father1 == father2 ){
                    int fatherLeft1 = rand()%popSize;
                    int fatherLeft2 = rand()%popSize;
                    while ( fatherLeft1 == fatherLeft2) //Need to be DIF
                        fatherLeft1 = rand()%popSize;

                    if( popObj[ fatherLeft1 ] < popObj[ fatherLeft2 ]){
                        father1 = fatherLeft1;
                    }else{
                        father1 = fatherLeft2;
                    }

                    int fatherRight1 = rand()%popSize;
                    int fatherRight2 = rand()%popSize;
                    while ( fatherRight1 == fatherRight2) //Need to be DIF
                        fatherRight1 = rand()%popSize;

                    if( popObj[ fatherRight1 ] < popObj[ fatherRight2 ]){
                        father2 = fatherRight1;
                    }else{
                        father2 = fatherRight2;
                    }
                }
                // cout<<"Delete - STOP getting new fathers\n";

                pair<bool,vector<data>> son1 = crossOver( pop[ father1 ], pop[ father2 ], Q, s,  njobs, ncars);
                pair<bool,vector<data>> son2 = crossOver( pop[ father1 ], pop[ father2 ], Q, s,  njobs, ncars);

                //new person to be inserted in new population
                vector<data> newPerson;

                //IF Both sons are feasible
                if( son1.first && son2.first){
                       newPop[ newPopSize ] = son1.second;
                       newPop[ newPopSize ] = son2.second;

                //When just one son is feasible
                }else if( son1.first ){

                    newPop[ newPopSize ] = son1.second;
                    // newPopObj[ newPopSize ] = resSon1;

                }else if( son2.first ){
                    newPop[ newPopSize ] = son2.second;

                }else{ //both sons are infeasible

                    //pick one father at random
                    int randomPickFather = rand()%2;

                    if( randomPickFather == 0){
                        newPop[ newPopSize ] = pop[ father1 ];
                        // newPopObj[ newPopSize ] = popObj[ father1 ];

                    }else{ // == 1
                        newPop[ newPopSize ] = pop[ father2 ];
                        // newPopObj[ newPopSize ] = popObj[ father2 ];
                    }

                    //Do mutation in father picked


                }

                newPopSize++;
            }

            //Putting in the last position the best solution
            newPop[ newPopSize ] = bestConfig;
            newPopObj[ newPopSize ] = bestObj;

            //Fast Local Search in new population
            // AND calculating object function!!

            for(int i=0;i< popSize; i++){
                pair<double, vector<data>> callFastLocalSearch = fastLocalSearch(true,njobs,ncars,w,P,t,F,d,Q,s, newPop[ i ]);
                pop[ i ] = callFastLocalSearch.second;
                popObj[ i ] =callFastLocalSearch.first;

                if( popObj[ i ] < bestObj){
                    bestObj = popObj[ i];
                    bestConfig = pop[ i ];
                }
            }
        }

    return { bestObj, bestConfig};
}

int main(){
    srand(time(NULL));
    int njobs;
    int ncars;

    //1- Reading data from file
    string fileName;
    float varMi,sigmaUm,sigmaDois;
    int numInstancia;
    cout<<"Input file name + .ext to open: ";


    getline(cin,fileName);
    ifstream in(fileName);

    #ifdef SHEETS_MODE
    cout<<"Input sheets name to output: ";
    string sheetsName;
    cin>>sheetsName;
    ofstream sheets(sheetsName);
    #endif

    while(in>>numInstancia){

        in>>varMi>>sigmaDois;
        in>>njobs;
        in>>ncars;


        char instancia[50];
        sprintf(instancia, "%d_%d_%d_%.1lf_%.1lf", numInstancia, njobs, ncars, varMi, sigmaDois);
        cout<<instancia<<endl;
        vector<int> F(ncars); //Car's cost
        vector<int> Q(ncars); //Car's capacity

        // Variables
        vector<vector<int> > t(njobs+1,vector<int>(njobs+1,0)); //Time travels
        vector<int> P(njobs); //Processing time
        vector<double> w(njobs); //Penalty weight
        vector<int> d(njobs); //Due date
        vector<int> s(njobs); //Job size

        for(int i=0;i<njobs;i++)
            in>>P[i];
        for(int i=0;i<njobs;i++)
            in>>d[i];
        for(int i=0;i<njobs;i++)
            in>>s[i];
        for(int i=0;i<njobs;i++)
            in>>w[i];
        for(int i=0;i<ncars;i++)
            in>>Q[i];
        for(int i=0;i<ncars;i++)
            in>>F[i];
        for(int i=0;i<=njobs;i++){
            for(int j=0;j<=njobs;j++){
                in>>t[i][j];
            }
        }

        // graspV1(njobs, ncars, w, P, t, F, d , Q, s, 100);

        // vector<data> solution;
        // pair<double, vector<data>> callVnd = VND(false, njobs, ncars, w, P, t, F, d , Q, s, solution);


        // cout<<"Get with vnd"<<callVnd.first<<endl;
        // ILS_VND( njobs, ncars, w, P, t, F, d, Q, s, 10, 100);

        // cout<<"Delete- Entrei Rvnd..."<<endl;
        // time_t time1;
        // time(&time1);
        // pair<double,vector<data>> ils = ILS_RVND( njobs, ncars, w, P, t, F, d, Q, s, 30, 100);
        // // cout<<"Delete- Sai da Rvnd..."<<endl;
        // time_t time1end;
        // time(&time1end);
        // double diff1 = difftime(time1end,time1);

        // printConfig( ils.first, "Ils Rvnd", ils.second, Q, s, njobs, ncars, P, t, d, w, F);

        // cout<<"Delete- Entrei vnd..."<<endl;
        time_t time2;
        time(&time2);
        pair<double,vector<data>> ils2 = ILS_VND( njobs, ncars, w, P, t, F, d, Q, s, 20, 100);
        // cout<<"Delete- Sai da vnd..."<<endl;
        time_t time2end;
        time(&time2end);
        double diff2 = difftime(time2end,time2);

        printConfig( ils2.first, "Now Ils Vnd", ils2.second, Q, s, njobs, ncars, P, t, d, w, F);

        time_t time3;
        time(&time3);
        pair<double,vector<data>> ga1 = genAlgo1(njobs, ncars, w, P, t, F, d, Q, s, 50);
        time_t time3end;
        time(&time3end);
        double diff3 = difftime(time3end,time3);
        printConfig( ga1.first, "GA1 ", ga1.second, Q, s, njobs, ncars, P, t, d, w, F);


        #ifdef SHEETS_MODE
        // sheets<<ils.first<<" "<<ils2.first<<" "<<ga1.first<<" "<<diff1<<" "<<diff2<<" "<<diff3<<"\n";
        sheets<<ils2.first<<" "<<ga1.first<<" "<<diff2<<" "<<diff3<<"\n";
        #endif

    }
    sheets.close();
    in.close();

}
