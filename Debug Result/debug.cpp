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

    cout<<"Job Tardiness: "<<endl;
    for(const int &u: jobTardiness){
    	cout<<u<<" ";
    }cout<<endl;
 	cout<<"Penalty Weight: "<<endl;
 	for(const double &u:weight){
 		cout<<u<<" ";
 	}cout<<endl;

 	cout<<"1 - Obj. Travel costs: "<<travelCosts<<endl;
 	cout<<"2 - Obj. Vehicle costs: "<<vehicleCosts<<endl;
 	cout<<"3 - Obj. Penalty costs: "<<penaltyCosts<<endl;
    return (travelCosts+vehicleCosts+penaltyCosts) ;
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
 

int main(){
    srand(time(NULL));
    int njobs;
    int ncars;

    //1- Reading data from file
    string fileName;
    float varMi,sigmaUm,sigmaDois;
    int numInstancia;
    cout<<"Input INSTANCE of result: ";


    getline(cin,fileName);
    ifstream in(fileName);
 
 	in>>numInstancia;
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


    in.close();

    cout<<"Enter manually the result like V1J0J2V0J3:\n";
    string line;
    getline(cin,line);

    vector<data> result;
    for(int i=0; i<line.size();i++){
    	if(line[ i ]=='V'){
    		data in;
    		in.job=false;
    		in.id = line[ i+1 ]-48;
    		result.push_back( in );
    		i++;
    	}else if(line[ i ]== 'J'){
    		data in;
    		in.job=true;
    		in.id = line[ i+1 ]-48;
    		result.push_back( in );
    		i++;
    	}
    }
    
    for(data i: result){
    	if( i.job){
    		cout<<"J"<<i.id +1<<" ";
    	}else{
    		cout<<"V"<<i.id +1<<" ";
    	}
    }cout<<endl;
    cout<<"Test (1) Weight and Vector Data: "<< validConfig( result, Q, s, njobs, ncars)<<endl;

    vector<vehicleLoaded> vehicleOrder=generateVehicleOrder(result,ncars);//Generating info about vehicle transportation
    vector<int> startVehicleTime(ncars,0);//Calculating Delivery time(D) and Starting time(Sk)
    vector<int> deliveryTime = calculatingDeliveryTime(result,startVehicleTime,t,P,vehicleOrder,njobs);
    vector<int> jobTardiness = calculatingJobTardiness(deliveryTime,njobs,d);//Generating Job Tardiness (T) of each job (O(N))
    
    double iniObj= objFunction(vehicleOrder,result,ncars,njobs,t,w,jobTardiness,F);
    

    cout<<" Obj Function for each Vehicle: "<<endl;
    for(int i=0; i<vehicleOrder.size(); i++){
    	cout<< objFVehicle(i,vehicleOrder,result,ncars,njobs,t,w,jobTardiness,F)<<" ";
    	if(i!= vehicleOrder.size()-1)
    		cout<<"+";
    }cout<<endl;

 
}