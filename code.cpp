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
/* --- Defines to better a view of decreasing and debugging , Uncomment to EXECUTE --- */

// #define SHEETS_MODE /* Create a file to print results divided by columns. */
// #define VIEW_OBJ_DECREASE /* View Objective Function decrease, with its entire value */
// #define VIEW_DECREASE /* View just the decrease of OBJ Function, the difference between new_Obj and old_Obj */
// #define PRINT_RVND_IMPROVE /* View improvement of every call of method */
// #define PRINT_RVND_CUSTOM_IMPROVE /*                    ''                   */
// #define VIEW_ILS_RVND_IMPROVE /*                 ''                   */
// #define VIEW_ILS_RVND_CUSTOM_IMPROVE /*          ''                   */
// #define DEBUG_MODE    /*                         ''                   */
//#define VIEW_RANDOM_TRIES /* Generate file divided by two columns showing the number of tries made by the 2 diff. random methods to generate solutions */

/*  --- Debugging words used to better found warnings ---

    1-Search for "Delete" to cleanup code
    2-Search for "Danger" to look for invalid erros to fix
    3-Search for "Optimize" to improve methods
    4-Search "Caution" to search for unitialized variables (but tested)

*/

#define INF 987654321
using namespace std;

//Variables to DECIDE which greedy method will be called in general greedy function
const string atc = "atc";
const string wmdd = "wmdd";
const string wedd = "wedd";

//Variables to TRACK number of tries to generate feasible random solution (in 2 diff. methods)
int tries_random_1, tries_random_2;

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

void quickConfigPrint( const vector<data> &config ){
    for(data i: config){
        if(i.job){
            cout<<"J"<<i.id+1<<" ";
        }else{
            cout<<"V"<<i.id+1<<" ";
        }
    }
    cout<<endl;
    return;
}

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


    while( !validConfig ){

        tries_random_1++;

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

bool validConfig( const vector<data> &config, const vector<int> &capacities, const vector<int> &jobSize, int njobs, int ncars ){


    int walk = 0;
    bool valid = true;

    unordered_set<int> jobsIn;
    unordered_set<int> carIn;


    ( config );

    for(int i=0; i<config.size(); i++){
    	if( config[i].id == -1){
            // cerr<<" not valid cause it has '-1' in some position of solution! "<<endl;
    		return false;
        }
    	if( config[i].job ){
    		if( jobsIn.find( config[i].id )== jobsIn.end() ){
    			jobsIn.insert( config[i].id );
    		}else{
                // cerr<<"  not valid cause it has a job repeated at least 2x! "<<endl;

    			return false; // job repeated
    		}
    		if( config[i].id<0 || config[i].id >=njobs)
    			return false;
    	}else{
    		if( carIn.find( config[i].id )== carIn.end() ){
    			carIn.insert( config[i].id );
    		}else{
                // cerr<<" not valid cause it has a vehicle(id) repeated at least 2x! "<<endl;
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
                    // cerr<<"Invalid because vehicle "<<config[walk].id+1<<" has cap. "<<carCapacity<<" and is carrying until now "<<accCap<<endl;
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

vector<data> randomConfigSequential(int njobs, int ncars, const vector<int> &vehicleCap, const vector<int> &jobSizes){

    //Generate a random config based on assigning random jobs to random cars until the last ones are full.
    bool valid = false;
    vector<data> solution( njobs + ncars );

    int cont_tries = 0;
    while( !valid ){

        tries_random_2++;

        //[Random] Shuffled  Jobs and Shuffled Cars
        vector<int> s_jobs( njobs );
        vector<int> s_cars( ncars );

        for(int i=0;i<njobs;i++)
            s_jobs[ i ] = i;
        for(int i=0;i<ncars;i++)
            s_cars[ i ] = i;

        random_shuffle( s_jobs.begin(), s_jobs.end() );
        random_shuffle( s_cars.begin(), s_cars.end() );

        int checkjob = 0;
        int checkinsert = 0;
        for(int i=0;i<ncars;i++){
            solution[ checkinsert ].job = false;
            solution[ checkinsert++ ].id = s_cars[ i ];
            int carry = 0;
            for(int j=checkjob; j<njobs; j++){
                if( (jobSizes[ s_jobs[ j ]]+carry) > vehicleCap[ s_cars[ i ]]){
                    break;
                }else{
                    solution[ checkinsert ].job = true;
                    solution[ checkinsert++ ].id = s_jobs[ j ];
                    carry += jobSizes[ s_jobs[ j ]];
                    checkjob = j + 1;
                }
            }

        }

        valid = validConfig( solution, vehicleCap, jobSizes, njobs, ncars);
    }

    return solution;

}

vector<data> generateGreedy(const string &which, const vector<int> &jobSize, int njobs, int ncars, const vector<double> &weight,
    const vector<int> &procTime, const vector<int> &dueDate, const vector<int> &carPrices, const vector<int> &carCap) {
    //Calculating processing time average
    double procTimeAvg=0;
    bool success= true; // If we generate a valid configuration

    for(int i=0;i<njobs;i++){

        procTimeAvg+=procTime[i];

    }
    procTimeAvg /= njobs;

     /* ------------ debug of parameters readed -------------

    cout<<"Jobs:" <<njobs<<" Cars: "<<ncars<<endl;
    cout<<"Sizes (P,d): "<<procTime.size()<<" "<<dueDate.size()<<" Sizes(F,Q): "<<carPrices.size()<<" "<<carCap.size()<<endl;
    cout<<"P: "; for(int i: procTime) cout<<i<<" "; cout<<endl;
    cout<<"d: "; for(int i: dueDate) cout<<i<<" "; cout<<endl;
    cout<<"F: "; for(int i: carPrices) cout<<i<<" "; cout<<endl;
    cout<<"Q: "; for(int i: carCap) cout<<i<<" "; cout<<endl;

     -------------- end of debugging ----------------------*/

    //Variables to generate processing order of jobs:
    vector<data> jobOrder(njobs);
    vector<bool> visitedJob(njobs,false);
    int acumulatedD=0;

    // cout<<"numtrbs"<<njobs<<endl;
    for(int i=0;i<njobs;i++){

        //Create priority queue of Dispatching Rule
        priority_queue<pair<double, int> > pq;

        for(int j=0;j<njobs;j++){

            if(visitedJob[j])
                continue;

            double value = 0;

            if(which == atc){
                value= (weight[j]/procTime[j]) * exp( (-1) * (max( dueDate[j]- procTime[j]- acumulatedD ,0 ))/ procTimeAvg );
                pq.push( make_pair( value,j) );
            }else if( which == wmdd){
                value=  (1 /weight[j])* max(procTime[j], dueDate[j] - acumulatedD) ;
                pq.push( make_pair( (-1) * value,j) );
            }else if( which == wedd){
                value=  ( dueDate[j]/weight[j]) ;
                pq.push( make_pair( (-1) * value,j) );
            }

        }

        jobOrder[i].id = ( pq.top() ).second;
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


    vector<int> carbyprice;
    while( !vehiclePrice.empty()){
        carbyprice.push_back(vehiclePrice.top().second);
        vehiclePrice.pop();
    }

    // cout<<"Job order: ";
    // for(data i: jobOrder){
    //     cout<<i.id<<" ";
    // }
    // cout<<endl;
    //
    // cout<<"Car by price: ";
    // for(int i: carbyprice){
    //     cout<<i<<" ";
    // }
    // cout<<endl;
    //Creating adj list of carried job ( sorted by car prices in vector below)
    vector<vector<int>> adjcarry( ncars );

    int job = 0;
    for( int i=0; i<ncars; i++){
        int cap = carCap[ carbyprice[ i ] ]; //getting id and cap

        int accCap = 0;
        while( job < njobs && (accCap + jobSize[ jobOrder[ job ].id ])<= cap ){
            adjcarry[ i ].push_back( jobOrder[ job ].id);
            accCap += jobSize[ jobOrder[ job ].id];
            job++;

        }
    }

    vector<data> configuration( njobs + ncars );
    int configSize = configuration.size();
    // cout<<"configsize "<<configSize<<endl;

    int insert = 0;
    for(int i=0; i<adjcarry.size(); i++){
        configuration[ insert ].job = false;
        configuration[ insert++ ].id = carbyprice[i];
        for(int j=0;j<adjcarry[i].size(); j++){
            configuration[ insert ].job = true;
            configuration[ insert++ ].id = adjcarry[i][j];
        }
    }
    return configuration;
}

// Swap sequential jobs inside a vehicle
bool nbhood1(char bestOrFirst, vector<data> &configuration, vector<vehicleLoaded> &vehicleOrder, int njobs, int ncars, const vector<double> &w,
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

                    if( bestOrFirst == 'F'){

                        return true;
                    }

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
        cout<<"Vz1: "<<bestObj-initialVObj<<endl;
        #endif

        #ifdef VIEW_OBJ_DECREASE
        cout<<"Vz1: "<<bestObj<<endl;
        #endif

        return true;
    }else{
        return false;
    }
}

// Swap jobs between vehicles
bool nbhood2(char bestOrFirst, vector<data> &configuration, vector<vehicleLoaded> &vehicleOrder, int njobs, int ncars, const vector<double> &w,
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
                            if( bestOrFirst == 'F' ){
                                return true;
                            }

                        }
                        swap( configuration[j], configuration[k]);

                    }
                }
            }
        }
    }

    if( improved ){

    	#ifdef VIEW_DECREASE
        cout<<"Vz2: "<<bestObj - iniObj<<endl;
        #endif

        #ifdef VIEW_OBJ_DECREASE
        cout<<"Vz2: "<<bestObj<<endl;
        #endif

        configuration = bestConfig;
        return true;
    }else{
        return false;
    }
}

// Inserting jobs in positions inside a vehicle
bool nbhood3(char bestOrFirst, vector<data> &configuration, vector<vehicleLoaded> &vehicleOrder, int njobs, int ncars, const vector<double> &w,
    const vector<int> &P, const vector<vector<int>> &t, const vector<int> &F , const vector<int> &d){

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

                    //If it is a First Improvement call
                    if( bestOrFirst == 'F'){
                        return true;
                    }

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

                    //If it is a First Improvement call
                    if( bestOrFirst == 'F'){
                        return true;
                    }
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
        cout<<"Vz3: "<<bestImprove<<endl;
        #endif

        #ifdef VIEW_OBJ_DECREASE
        cout<<"Vz3: "<<iniObj + bestImprove<<endl;
        #endif

        configuration = bestConfig;
        return true;
    }else{
        return false;
    }
}

// Inserting jobs in positions inside others vehicle
bool nbhood4(char bestOrFirst, vector<data> &configuration, vector<vehicleLoaded> &vehicleOrder, int njobs, int ncars, const vector<double> &w,
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

                            //If it is a call for First Improvement
                            if( bestOrFirst == 'F'){
                                configuration = newconfig;
                                vehicleOrder= newVehicleOrder;
                                return true;
                            }
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

                            //If it is a call for First Improvement
                            if( bestOrFirst == 'F'){
                                configuration = newconfig;
                                vehicleOrder= newVehicleOrder;
                                return true;
                            }

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

                            //If it is a call for First Improvement
                            if( bestOrFirst == 'F'){
                                configuration = newConfig;
                                vehicleOrder= newVehicleOrder;
                                return true;
                            }

                            bestConfig = newConfig;
                            bestVOrder = newVehicleOrder;

                            improved = true;
                            bestObj = newObj;
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
        cout<<"Vz4: "<<bestObj-iniObj<<endl;
        #endif

        #ifdef VIEW_OBJ_DECREASE
        cout<<"Vz4: "<<bestObj<<endl;
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

//2-opt
bool nbhood7(char bestOrFirst, vector<data> &config, vector<vehicleLoaded> &vehicleOrder, int njobs, int ncars, const vector<double> &w,
    const vector<int> &P, const vector<vector<int>> &t, const vector<int> &F , const vector<int> &d, const vector<int> &Q, const vector<int> &s){

        if( !validConfig ( config,Q,s,njobs,ncars)){
            return false;
        }
        vector<int> startVehicleTime(ncars,0);
        vector<int> deliveryTime = calculatingDeliveryTime(config,startVehicleTime,t,P,vehicleOrder,njobs);
        vector<int> T = calculatingJobTardiness(deliveryTime,njobs,d);

        vector<data> bestConfig;
        bool improved = false;
        double biggestImprove = 0;

        double initialObj = objFunction(vehicleOrder,config, ncars, njobs,t,w,T,F);
        double bestObj = initialObj;

        vector<int> carUsed;

        //The position of each car i will be referred to edgesByCar[ i ]
        vector<vector< pair<int, int>>> edgesByCar( ncars);

        vector<int> jobPos ( njobs );
        for(int i=0;i<config.size();i++){
            if( config[i].job){
                jobPos[ config[i].id ]= i;
            }
        }

        //Generating edges
        for(int i=0; i<vehicleOrder.size(); i++ ){
            int beg= vehicleOrder[i].initialPos; int end = vehicleOrder[i].finalPos;
            //Verifying if there is enough edges to do 2-opt in each car
            if( (end-beg)<=2 ){
                continue;
            }

            int carId = vehicleOrder[i].id;
            carUsed.push_back( carId );


            //We will skip the edges that connects to origin (this will be treated in nhood1)
            // edgesByCar[ carId ].push_back( {0, config[ beg ].id});
            // edgesByCar[ carId ].push_back( {config[ end ].id, 0});

            for(int j= beg; j< end; j++){
                edgesByCar[ carId ].push_back( {config[j].id, config[j+1].id});
            }
        }

        //Randomizing vehicles
        random_shuffle( carUsed.begin(), carUsed.end());
        for(int i=0; i<carUsed.size(); i++){

            int car = carUsed[ i ];
            random_shuffle( edgesByCar[ car ].begin(), edgesByCar[ car ].end());

            for(int j=0; j<edgesByCar[ car].size()-1; j++){

                for(int k=j+1; k<edgesByCar[ car ].size(); k++){
                    set<int> points;
                    int p1= edgesByCar[ car ][j].first;
                    int p2= edgesByCar[ car ][j].second;
                    int p3= edgesByCar[ car ][k].first;
                    int p4= edgesByCar[ car ][k].second;
                    points.insert(p1 );
                    points.insert(p2 );
                    points.insert(p3 );
                    points.insert(p4 );
                    if( points.size() < 4){
                        //Not valid, adjacent edges!
                        continue;
                    }
                    vector<data> newconfig = config;

                    swap( config[ jobPos[ p2 ]], config[ jobPos[ p3 ]] );

                    vector<int> newstartVehicleTime(ncars,0);
                    vector<int> newdeliveryTime = calculatingDeliveryTime(config,newstartVehicleTime,t,P,vehicleOrder,njobs);
                    vector<int> newT = calculatingJobTardiness(newdeliveryTime,njobs,d);

                    double newObj = objFunction(vehicleOrder,config, ncars, njobs,t,w,newT,F);

                    if( newObj < bestObj){

                        #ifdef VIEW_DECREASE
                            cout<<"Vz7: "<<newObj - bestObj<<endl;
                        #endif

                        #ifdef VIEW_OBJ_DECREASE
                            cout<<"Vz7: "<<newObj<<endl;
                        #endif

                        bestObj = newObj;
                        bestConfig = config;
                        improved = true;
                        if( bestOrFirst == 'F'){
                            return true;
                        }
                    }
                    // undoing
                    swap( config[ jobPos[ p2 ]], config[ jobPos[ p3 ]] );

                }
            }

        }

        if( bestOrFirst == 'F'){
            if(!improved){
                return false;
            }
        }else{
            if(improved){ // 'B'est improvement

            config = bestConfig;
            return true;

            }else{
                return false;
            }
        }

}

//pair< ValueObjFunction, validSolution>
pair<double, vector<data>> RVND_Custom(bool reuse, int njobs, int ncars, const vector<double> &w,const vector<int> &P, const vector<vector<int>> &t,
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

		switch(neighbor){
			case 2:
            	improved = nbhood2('B',initialConfig, vehicleOrder, njobs, ncars, w, P, t, F, d, Q, s);
			case 4:
            	improved = nbhood4('B',initialConfig, vehicleOrder, njobs, ncars, w, P, t, F, d ,Q, s);

			case 5:
            	improved = nbhood5('B',initialConfig, vehicleOrder, njobs, ncars, w, P, t, F, d);
			case 6:
            	improved = nbhood6('B',initialConfig, vehicleOrder, njobs, ncars, w, P, t, F, d);
		}


		if( improved ){
			generalImprove = true;

			while( true ){ //intra route
				if( nbhood1('B',initialConfig, vehicleOrder, njobs, ncars, w, P, t, F, d)){
					continue;
				}
				if( nbhood3('B',initialConfig, vehicleOrder, njobs, ncars, w, P, t, F, d)){

					continue;
				}
                if( nbhood7('B',initialConfig, vehicleOrder, njobs, ncars, w, P, t, F, d,Q,s)){

					continue;
				}
				break;
			}


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
pair<double, vector<data>> RVND(bool reuse, int njobs, int ncars, const vector<double> &w,const vector<int> &P, const vector<vector<int>> &t,
   const vector<int> &F , const vector<int> &d, const vector<int> &Q, const vector<int> &s, vector<data> &initialConfig){


    //Checking if it is needed to generate a new one or if the current solution will be used
    if( !reuse ){
        initialConfig = randomConfigSequential(njobs,ncars,Q,s);
    }

    vector<vehicleLoaded> vehicleOrder=generateVehicleOrder(initialConfig,ncars);//Generating info about vehicle transportation
    vector<int> startVehicleTime(ncars,0);//Calculating Delivery time(D) and Starting time(Sk)
    vector<int> deliveryTime = calculatingDeliveryTime(initialConfig,startVehicleTime,t,P,vehicleOrder,njobs);
    vector<int> jobTardiness = calculatingJobTardiness(deliveryTime,njobs,d);//Generating Job Tardiness (T) of each job (O(N))

    double iniObj= objFunction(vehicleOrder,initialConfig,ncars,njobs,t,w,jobTardiness,F);


    #ifdef PRINT_RVND_IMPROVE
    cout<<"\n Started RVND at obj: "<<iniObj<<"\n\n";
    #endif

    vector<int> neighbors = {1,2,3,4,5,6,7};


    random_shuffle( neighbors.begin(), neighbors.end());

    int it = 0;

    while( it < 7 ){

    	//1 corrigida, 2 e 3 parecem tb. Analisar a 4

        int whichNeighbor = neighbors[ it ];

        bool improved = false;

        if( whichNeighbor == 1){
            improved = nbhood1('B',initialConfig, vehicleOrder, njobs, ncars, w, P, t, F, d);

        }else if( whichNeighbor == 2){
            improved = nbhood2('B',initialConfig, vehicleOrder, njobs, ncars, w, P, t, F, d ,Q, s);

        }else if( whichNeighbor == 3){
            improved = nbhood3('B',initialConfig, vehicleOrder, njobs, ncars, w, P, t, F, d);

        }else if(whichNeighbor ==4){
            improved = nbhood4('B',initialConfig, vehicleOrder, njobs, ncars, w, P, t, F, d ,Q, s);
        }else if(whichNeighbor == 5){
            improved = nbhood5('B',initialConfig, vehicleOrder, njobs, ncars, w, P, t, F, d);
        }else if(whichNeighbor == 6){
            improved = nbhood6('B',initialConfig, vehicleOrder, njobs, ncars, w, P, t, F, d);
        }else if(whichNeighbor == 7){
            improved = nbhood7('B',initialConfig, vehicleOrder, njobs, ncars, w, P, t, F, d, Q, s);
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

    #ifdef PRINT_RVND_IMPROVE
    cout<<"Finished RVND at obj: "<<finalObj<<"\n";
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

		     	imp1 = nbhood1('F',initialConfig, vehicleOrder, njobs, ncars, w, P, t, F, d );
		     	imp2 = nbhood2('F',initialConfig, vehicleOrder, njobs, ncars, w, P, t, F, d , Q, s);
		     	imp3 = nbhood3('F',initialConfig, vehicleOrder, njobs, ncars, w, P, t, F, d );
		     	imp4 = nbhood4('F',initialConfig, vehicleOrder, njobs, ncars, w, P, t, F, d , Q, s);

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

//Soft transformation: swap two jobs between different vehicles
vector<data> mutation( vector<data> &solution, const vector<int> &capacities, const vector<int> &jobSize, int njobs, int ncars){

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

    while(true){

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
            break;
        }else{
            //Swap again
            randomTries++;
        }
    }
    return solution;
}

vector<data> perturb( vector<data> &solution, const vector<int> &capacities, const vector<int> &jobSize, int njobs, int ncars){

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

                int changeDadTries = 0;
                while( dad[first] == dad[sec]){

                    if(changeDadTries>100){ //It means we got stuck because ALL jobs are in just one vehicle
                       return solution;
                    }
                    changeDadTries++;
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
                 int changeDadTries = 0;
                 while( dad[first] == dad[sec]){

                     if(changeDadTries>100){ //It means we got stuck because ALL jobs are in just one vehicle
                        return solution;
                     }

                     first = rand()%njobs;
                     changeDadTries++;
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


                    //Generating new array of jobs of vehicles!

                    jobsFirst[ insertOne ] = sec;
                    jobsSec[ insertTwo ] = first;

                    int in1=0;
                    int in2=0;
                    for(int k=beg1; k<=end1; k++){

                        if(in1<(jobsFirst.size()-1) && jobsFirst[ in1 ]!= -1)
                            in1++;

                        if( solution[k].job && solution[k].id==first){
                            continue;
                        }else{
                            jobsFirst[ in1++ ] = solution[k].id;
                        }
                    }

                    for(int k=beg2; k<=end2; k++){


                        if( in2<(jobsSec.size()-1) && jobsSec[ in2 ]!= -1)
                            in2++;

                        if( solution[k].job && solution[k].id==sec){
                            continue;
                        }else{
                            jobsSec[ in2++ ] = solution[k].id;
                        }
                    }

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

pair<double, vector<data>> ils_rvnd(int njobs, int ncars, const vector<double> &w,const vector<int> &P, const vector<vector<int>> &t,
    const vector<int> &F , const vector<int> &d, const vector<int> &Q, const vector<int> &s, int maxIter, int maxIterIls){


    int nbValidSolutions = 0;
    vector< vector<data> > validSolutions;

    // Generating initial greedy solutions
    vector<data> atcConfig = generateGreedy(atc, s, njobs, ncars, w, P, d, F, Q);
    vector<data> weddConfig = generateGreedy(wedd, s, njobs, ncars, w, P, d, F, Q);
    vector<data> wmddConfig = generateGreedy(wmdd, s, njobs, ncars, w, P, d, F, Q);

    //Danger: Verify ATC!!!
    // Verifying if they are really valid

    // cerr<<" Trying ATC: \n";
    if( validConfig( atcConfig, Q, s, njobs, ncars)){
        validSolutions.push_back( atcConfig );
        nbValidSolutions++;
    }
    // cerr<<" Trying WMDD: \n";

    if( validConfig( wmddConfig, Q, s, njobs, ncars)){

        validSolutions.push_back( wmddConfig);
        nbValidSolutions++;
    }

    // cerr<<" Trying WEDD: \n";
    if( validConfig( weddConfig, Q, s, njobs, ncars)){

        validSolutions.push_back( weddConfig);
        nbValidSolutions++;
    }

    int contGetSolution = 0;

    double bestResult = INF;
    vector<data> bestSolution;


    for(int a=0; a<maxIter; a++){

        vector<data> solution;
        pair< double, vector<data> > callRvnd;

        //Getting the greedy solutions if there is one avaiable

        bool justCalledRvnd = true;
        if( contGetSolution < nbValidSolutions ){

            solution = validSolutions[ contGetSolution ];

            callRvnd = RVND( true, njobs, ncars, w, P, t, F, d , Q, s, solution);

            contGetSolution++;
        }else{
        //Otherwise we generate a random one
            callRvnd = RVND( false, njobs, ncars, w, P, t, F, d , Q, s, solution);
         }

        for(int b=0; b< maxIterIls; b++){

            if( !justCalledRvnd ){
                callRvnd = RVND( true, njobs, ncars, w, P, t, F, d , Q, s, solution);
            }else{
                justCalledRvnd = false;
            }

            if( callRvnd.first < bestResult){

                #ifdef VIEW_ILS_RVND_IMPROVE
                    if(bestResult != INF ) // Printing only the improves caused by ILS
                        cout<<"// ILS_Improve:   "<<callRvnd.first-bestResult <<" //\n";
                #endif
                bestResult = callRvnd.first;
                bestSolution = callRvnd.second;

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

pair<double, vector<data>> ils_rvnd_custom(int njobs, int ncars, const vector<double> &w,const vector<int> &P, const vector<vector<int>> &t,
    const vector<int> &F , const vector<int> &d, const vector<int> &Q, const vector<int> &s, int maxIter, int maxIterIls){


    int nbValidSolutions = 0;
    vector< vector<data> > validSolutions;

    // Generating initial greedy solutions
    vector<data> atcConfig = generateGreedy(atc, s, njobs, ncars, w, P, d, F, Q);
    vector<data> weddConfig = generateGreedy(wedd, s, njobs, ncars, w, P, d, F, Q);
    vector<data> wmddConfig = generateGreedy(wmdd, s, njobs, ncars, w, P, d, F, Q);

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
        pair< double, vector<data> > callRvndCustom;

        //Getting the greedy solutions if there is one avaiable

        bool justCalledRvndCustom = true;
        if( contGetSolution < nbValidSolutions ){

            solution = validSolutions[ contGetSolution ];

            callRvndCustom = RVND_Custom( true, njobs, ncars, w, P, t, F, d , Q, s, solution);

            contGetSolution++;
        }else{
        //Otherwise we generate a random one
            callRvndCustom = RVND_Custom( false, njobs, ncars, w, P, t, F, d , Q, s, solution);
         }

        for(int b=0; b< maxIterIls; b++){

            if( !justCalledRvndCustom ){

                callRvndCustom = RVND_Custom( true, njobs, ncars, w, P, t, F, d , Q, s, solution);

            }else{
                justCalledRvndCustom = false;
            }

            if( callRvndCustom.first < bestResult){

                #ifdef VIEW_ILS_RVND_CUSTOM_IMPROVE
                    if(bestResult != INF ) // Printing only the improves caused by ILS
                        cout<<"// ILS_Improve:   "<<callRvndCustom.first-bestResult <<" //\n";
                #endif
                bestResult = callRvndCustom.first;
                bestSolution = callRvndCustom.second;

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

//pair< BOOL, solution > indicating if it is valid
pair< bool, vector<data> > crossOver( const vector<data> &f1, const vector<data> &f2,const vector<int> &capacities, const vector<int> &jobSize, int njobs, int ncars){

    //generating random interval to 2-point crossOver

    int a = rand()%(f1.size());
    int b = rand() %(f1.size());

    while( a==b || a==(f1.size()-1) || (a==0) || (b==0) || (a>b)){
        a = rand()%(f1.size());
        b = rand() %(f1.size());
    }
    unordered_set<int> jobsCross;
    unordered_set<int> carsCross;
    for(int i=a; i<=b; i++){ // i <= b?
        if( f1[i].job){
            jobsCross.insert( f1[i].id );
        }else{
            carsCross.insert( f1[i].id);
        }
    }

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

    bool isvalid = validConfig( son, capacities, jobSize, njobs, ncars );

    return { isvalid, son};
}

//method using in G.A. to perform a fast improvement on solutions
pair<double, vector<data>> fastLocalSearch(bool reuse, int njobs, int ncars, const vector<double> &w,const vector<int> &P, const vector<vector<int>> &t,
   const vector<int> &F , const vector<int> &d, const vector<int> &Q, const vector<int> &s, vector<data> &initialConfig){


    //Caution - there is something missing?
    vector<vehicleLoaded> vehicleOrder=generateVehicleOrder(initialConfig,ncars);//Generating info about vehicle transportation

    nbhood1('B',initialConfig, vehicleOrder, njobs, ncars, w, P, t, F, d);
    nbhood2('B',initialConfig, vehicleOrder, njobs, ncars, w, P, t, F, d ,Q, s);
    nbhood3('B',initialConfig, vehicleOrder, njobs, ncars, w, P, t, F, d);
    nbhood4('B',initialConfig, vehicleOrder, njobs, ncars, w, P, t, F, d ,Q, s);
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
            pair<double, vector<data>> callRvnd = RVND( false, njobs, ncars, w, P, t, F, d , Q, s, trash);
            pop[ i ] = callRvnd.second;
            popObj[ i ] = callRvnd.first;

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

                    vector<data> afterMutation = mutation( newPop[ newPopSize ], Q, s, njobs, ncars);
                    //Do mutation in father picked

                    newPop[ newPopSize ] = afterMutation;

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

    // Reading parameters from file
    string fileName;
    float varMi,sigmaUm,sigmaDois;
    int numInstancia;
    cout<<"Input file name + .ext to open: ";


    getline(cin,fileName);
    ifstream in(fileName);

    //Define to print results in spreadsheet mode
    #ifdef SPREADSHEET_MODE
        cout<<"Input sheets name to output: ";
        string sheetsName;
        cin>>sheetsName;
        ofstream sheets(sheetsName);
    #endif

    int total_random_1 = 0;
    int total_random_2 = 0;

    #ifdef VIEW_RANDOM_TRIES
        cout<<"Input random_tries filename to output: ";
        string triesname;
        cin>>triesname;
        ofstream triesfile(triesname);
    #endif
    //Reading all instances in file (Default: 300 by file)
    while(in>>numInstancia){

        in>>varMi>>sigmaDois;
        in>>njobs;
        in>>ncars;
        char instancia[50];
        sprintf(instancia, "%d_%d_%d_%.1lf_%.1lf", numInstancia, njobs, ncars, varMi, sigmaDois);
        cout<<instancia<<endl;
        vector<int> F(ncars); //Car's cost
        vector<int> Q(ncars); //Car's capacity

        // Reading parameters
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
        // pair<double, vector<data>> callRvnd = RVND(false, njobs, ncars, w, P, t, F, d , Q, s, solution);


        tries_random_2 = 0;
        tries_random_1 = 0;


        //Calling for ILS Rvnd
        time_t time2;
        time(&time2);
        pair<double,vector<data>> ils2 = ils_rvnd( njobs, ncars, w, P, t, F, d, Q, s, 5, 10);

        time_t time2end;
        time(&time2end);
        double diff2 = difftime(time2end,time2);
        printConfig( ils2.first, "ils_rvnd", ils2.second, Q, s, njobs, ncars, P, t, d, w, F);


        #ifdef VIEW_RANDOM_TRIES


        vector<data> random1 = generateValidRandomConfig(njobs, ncars, Q,s);
        vector<data> random2 = randomConfigSequential(njobs, ncars, Q,s);
        triesfile<<tries_random_1<<" "<<tries_random_2<<endl;

        total_random_1 += tries_random_1;
        total_random_2 += tries_random_2;
        #endif

        //Calling for ILS Rvnd Custom ( Intra Route and Inter Route, from article)
        /*
        time_t time1;
        time(&time1);
        pair<double,vector<data>> ils = ils_rvnd_custom( njobs, ncars, w, P, t, F, d, Q, s, 30, 100);
        time_t time1end;
        time(&time1end);
        double diff1 = difftime(time1end,time1);
        printConfig( ils.first, "ils_rvnd_custom", ils.second, Q, s, njobs, ncars, P, t, d, w, F);

        //Calling for Genetic Algorithm Version 1
        time_t time3;
        time(&time3);
        pair<double,vector<data>> ga1 = genAlgo1(njobs, ncars, w, P, t, F, d, Q, s, 50);
        time_t time3end;
        time(&time3end);
        double diff3 = difftime(time3end,time3);
        printConfig( ga1.first, "gen_algo_1 ", ga1.second, Q, s, njobs, ncars, P, t, d, w, F);
*/

        #ifdef SPREADSHEET_MODE
            sheets<<ils.first<<" "<<ils2.first<<" "<<ga1.first<<" "<<diff1<<" "<<diff2<<" "<<diff3<<"\n";
        #endif

    }

    #ifdef SPREADSHEET_MODE
        sheets.close();
    #endif
    #ifdef VIEW_RANDOM_TRIES
        triesfile<<"TOTAL: "<<total_random_1<<" "<<total_random_2<<endl;
        triesfile.close();
    #endif

    in.close();

}
