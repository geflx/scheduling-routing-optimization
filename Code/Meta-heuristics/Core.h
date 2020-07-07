#ifndef CORE_H
#define CORE_H

    // Data Structures.
    #include <vector>
    #include <unordered_set>
    #include <unordered_map>
    #include <set>
    #include <queue>

    #include <iostream>
    #include <fstream>
    #include <iomanip>
    #include <cmath>
    #include <stdlib.h>  
    #include <time.h>    
    #include <stdio.h>
    #include <stdlib.h>
    #include <ctype.h>
    #include <algorithm>
    #include <chrono>
    #include <random>

    #define EPS 1e-5
    #define INF 987654321

    using namespace std;

    // Greedy Constructive Rules IDs.
    const string atc = "atc";
    const string wmdd = "wmdd";
    const string wedd = "wedd";

    struct data{
        bool job; // True: Job, False: Vehicle.
        int id;  // ID.

        data(){
            job = true; // Default: Is job.
            id = -1;
        }
    };

    struct vehicleLoaded{
        int id, initialPos, finalPos;
    };

    struct Execution{
        double value = -1;
        double time = -1;

        vector<data> vec;
    };

    // * Ident OK
    double objFunction(const vector<vehicleLoaded> &vehicleOrder, const vector<data> &configuration,int K, int N, const vector<vector<int> > &time,
        const vector<double> &weight,const vector<int> &jobTardiness, const vector<int> &carPrices){

        int travelCosts, vehicleCosts;
        double penaltyCosts;
        
        travelCosts = vehicleCosts = 0;
        penaltyCosts = 0.0;

        //Calculate travel costs.
        for(int i=0; i<vehicleOrder.size(); i++){

            int initialIndex = vehicleOrder[i].initialPos; 
            int whichJob = configuration[initialIndex].id;

            travelCosts += time[0][whichJob+1];  // Origin -> first job.

            int finalIndex = vehicleOrder[i].finalPos; 

            for(int j=initialIndex; j<finalIndex; j++){

                int jobA = configuration[j].id;
                int jobB = configuration[j+1].id;

                travelCosts += time[ jobA+1 ][ jobB+1];
            }

            whichJob = configuration[finalIndex].id;
            travelCosts += time[ whichJob+1][ 0 ] ; // Last job -> origin.
        }

        //Calculate vehicle's use prices.
        for(int k=0; k<vehicleOrder.size(); k++)
            vehicleCosts += carPrices[ vehicleOrder[k].id ];
        
        //Calculate penalty costs.
        for(int i=0; i<N; i++)
            penaltyCosts+= jobTardiness[i]* weight[i];
        
        return (travelCosts+vehicleCosts+penaltyCosts);
    }


    // * Ident OK
    double objFVehicle(int i, const vector<vehicleLoaded> &vehicleOrder, const vector<data> &configuration,int K, int N, const vector<vector<int> > &time,
        const vector<double> &weight,const vector<int> &jobTardiness, const vector<int> &carPrices){

        //Caution : need to recalculate the deliveryTime and JobTardiness before call.

        int travelCosts, vehicleCosts;
        double penaltyCosts;
        
        travelCosts = vehicleCosts = 0;
        penaltyCosts = 0.0;

        //Calculate Travel Costs.
        int initialIndex = vehicleOrder[i].initialPos; 
        int whichJob = configuration[initialIndex].id;

        travelCosts += time[0][whichJob+1]; // Cost of Origin -> First Job.

        int finalIndex = vehicleOrder[i].finalPos; 


        for(int j=initialIndex; j<finalIndex; j++){

            int jobA = configuration[j].id;
            int jobB = configuration[j+1].id;

            travelCosts += time[ jobA+1 ][ jobB+1];
        }

        whichJob= configuration[finalIndex].id;
        travelCosts+= time[ whichJob+1][ 0 ] ; // Cost of Last job -> origin.


        // Calculate SINGLE vehicle use costs.
        vehicleCosts += carPrices[ vehicleOrder[i].id ];

        // Calculate Penalty Costs.
        for(int j=initialIndex; j<=finalIndex; j++){

            int job = configuration[j].id;
            penaltyCosts += jobTardiness[job] * weight[job];

        }

        return (travelCosts+vehicleCosts+penaltyCosts) ;
    }

    // * Ident OK
    vector<int > calculatingDeliveryTime(const vector<data> &config, vector<int> &startVehicleTime, const vector<vector<int> > &time,const vector<int> &procTime,
        const vector<vehicleLoaded> &vehicleOrder,int N){

        vector<int> deliveryTime(N, 0);
        int acumulatedProcessTime = 0;

        //Calculate acumulate processing time.
        for(int i=0; i<vehicleOrder.size(); i++){

            int whichCar = vehicleOrder[i].id; 

            for(int j=vehicleOrder[i].initialPos; j<=vehicleOrder[i].finalPos;  j++){
                
                int whichJob = config[j].id;
                acumulatedProcessTime += procTime[ whichJob ];
                
            }
            startVehicleTime[whichCar] = acumulatedProcessTime;

            // Calculate travel distance between deliveries.
            int acumulatedDeliveryTime = 0;
            int lastJob;

            acumulatedDeliveryTime = time[ 0 ][config[vehicleOrder[i].initialPos].id +1];

            for(int j=vehicleOrder[i].initialPos; j<vehicleOrder[i].finalPos; j++){

                int whichJob = config[j].id;
                int nextJob = config[j+1].id;

                deliveryTime[ whichJob ] = acumulatedDeliveryTime + acumulatedProcessTime;
                acumulatedDeliveryTime += time[ whichJob+1 ][ nextJob+1];

            }
            deliveryTime[ config[vehicleOrder[i].finalPos].id  ] = acumulatedProcessTime + acumulatedDeliveryTime;
        }

        return deliveryTime;
    }

    // * Ident OK
    vector<vehicleLoaded> generateVehicleOrder(const vector<data> &config,int K){

        vector<vehicleLoaded> vehicleList;

        for(int i=0; i<config.size(); i++){

            if(!config[i].job){ 

                vehicleLoaded temp; 
                temp.id = config[i].id;

                int count = i+1;
                temp.initialPos = count;

                while(count<config.size() && config[count].job)
                    count++;
                
                temp.finalPos = count-1; 

                // Ignore if vehicle is empty.
                if(! (count == (i+1)) )
                    vehicleList.push_back(temp);
                
                i = count-1;
            }
        }

        return vehicleList;
    }

    // * Ident OK
    vector<int> calculatingJobTardiness(const vector<int> &deliveryTime,int N, const vector<int> &dueDate){

        vector<int> T(N);

        for(int i=0; i<N; i++){

            T[i] = deliveryTime[i] - dueDate[i];

            if(T[i]<0)
                T[i]=0;
        }

        return T;
    }

    vector<data> generateValidRandomConfig(int N, int K, const vector<int> &vehicleCap, const vector<int> &jobSizes){

        vector<data> configuration (N+ (K));
        bool validConfig=false;


        while( !validConfig ){

            validConfig=true;

            int configSize = configuration.size();
            vector<bool> visitedConfig(configSize,false);
            vector<bool> visitedCar(K,false);

            int insertedVehicles=0;
            //Generating vehicles random positions
            while(insertedVehicles<K){
                int randomPos= rand()%configSize;
                while(visitedConfig[randomPos]) {
                    randomPos=rand()%configSize;
                }
                if(!visitedConfig[0]){ //Putting in the config the first car to be used in position[0] of the vector.
                    randomPos= rand()%K;
                    configuration[0].job=false;
                    configuration[0].id=randomPos;
                    visitedConfig[0]=true;
                    visitedCar[randomPos]=true;
                    insertedVehicles++;
                }else{
                    int randomVeic= rand()%K;
                    while(visitedCar[randomVeic]) {
                        randomVeic=rand()%K;
                    }
                    configuration[randomPos].job = false; //It is a vehicle, not a job.
                    configuration[randomPos].id = randomVeic; //Identification of vehicle.
                    visitedCar[randomVeic]=true;
                    visitedConfig[randomPos]=true;
                    insertedVehicles++;
                }
            }
            //Generating random positions for jobs
            for(int i=0;i<N;i++){
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

    // * Ident OK
    bool validConfig(const vector<data> &config, const vector<int> &capacities, const vector<int> &jobSize, int N, int K){

        int walk = 0;
        bool valid = true;

        unordered_set<int> jobsIn;
        unordered_set<int> carIn;

        for(int i=0; i<config.size(); i++){

            // ID == -1.
            if( config[i].id == -1)
                return false;
            
            if( config[i].job ){

                if(jobsIn.find(config[i].id) == jobsIn.end())
                    jobsIn.insert(config[i].id);
                else
                    return false; // Repeated Job.

                // Job ID out of boundaries.
                if( config[i].id<0 || config[i].id >= N)
                    return false;

            }else{

                if(carIn.find(config[i].id ) == carIn.end())
                    carIn.insert( config[i].id );
                else
                    return false; // Repeated vehicle.

                // Vehicle ID out of boundaries.
                if( config[i].id<0 || config[i].id >= K)
                    return false;
            }
        }

        while(walk < config.size() && valid){

            int walkAhead = walk;

            if( config[walk].job == false ){

                walkAhead = walk + 1;

                if(walkAhead >= config.size())
                    break;            

                int carCapacity = capacities[config[walk].id];
                int accCap = 0;

                while(walkAhead<config.size() && config[ walkAhead ].job){

                    accCap += jobSize[ config[ walkAhead].id ];

                    // Vehicle overlaps its capacity.
                    if( accCap > carCapacity)
                        return false;
                    
                    ++walkAhead;
                }
            }
            walk = walkAhead;
        }

        return true;
    }

    vector<data> randomConfigSequential(int N, int K, const vector<int> &vehicleCap, const vector<int> &jobSizes){

        //Generate a random config based on assigning random jobs to random cars until the last ones are full.
        bool valid = false;
        vector<data> solution( N + K );

        int cont_tries = 0;
        while( !valid ){

            //[Random] Shuffled  Jobs and Shuffled Cars
            vector<int> s_jobs( N );
            vector<int> s_cars( K );

            for(int i=0;i<N;i++)
                s_jobs[ i ] = i;
            for(int i=0;i<K;i++)
                s_cars[ i ] = i;

            random_shuffle( s_jobs.begin(), s_jobs.end() );
            random_shuffle( s_cars.begin(), s_cars.end() );

            int checkjob = 0;
            int checkinsert = 0;
            for(int i=0;i<K;i++){
                solution[ checkinsert ].job = false;
                solution[ checkinsert++ ].id = s_cars[ i ];
                int carry = 0;
                for(int j=checkjob; j<N; j++){
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

            valid = validConfig( solution, vehicleCap, jobSizes, N, K);
        }
        return solution;
    }

    void printConfig(const Execution &S, const vector<int> &carCap, const vector<int> &jobSize,
                  int N, int K, const vector<int> &processTime, const vector<vector<int>> &time, const vector<int> &dueDate,
                  const vector<double> &weight, const vector<int> &carPrices, int instNumber, double mi, double delta){

        char instanceId[50];
        sprintf(instanceId, "%d_%d_%d_%.1lf_%.1lf", instNumber, N, K, mi, delta);
        cout << instanceId << " ";

        vector<vehicleLoaded> vehicleOrder = generateVehicleOrder( S.vec, K);
        vector<int> startVehicleTime(K, 0);
        vector<int> deliveryTime = calculatingDeliveryTime(S.vec ,startVehicleTime,time,processTime,vehicleOrder,N);
        vector<int> T = calculatingJobTardiness(deliveryTime,N,dueDate);

        int numJobsFixo = jobSize.size();

        if(numJobsFixo == 20)
            numJobsFixo -= 4;
        else if(numJobsFixo == 15)
            numJobsFixo -= 1;
        else if(numJobsFixo == 8)
            numJobsFixo += 2;
        
        cout << setw(13) << "Answer";
        cout << setw(13) << S.value <<"    ";

        int numJobs= jobSize.size();

        string procOrder;

        for(int j=0; j < S.vec.size();j++){
            if( S.vec[j].job ){
                procOrder+= to_string( S.vec[j].id +1);
                if(numJobs>1){
                    procOrder+="-";
                    numJobs--;
                }
            }
        }
        cout<<setw(numJobsFixo*3)<<procOrder;
        int j=0;

        string deliveryOrder;

        for(int i=0;i<S.vec.size();i++){
            if(S.vec[i].job){
                deliveryOrder+= to_string( S.vec[i].id+1)+" ";
            }else{
                deliveryOrder+= " ["+ to_string(S.vec[i].id+1) + "] ";
            }
        }
        for(int i=1; i<deliveryOrder.size()-1;i++){
            if( deliveryOrder[i-1]>='0' &&  deliveryOrder[i-1]<='9' && deliveryOrder[i+1]>='0' && deliveryOrder[i+1]<='9' ){
                deliveryOrder[i] = '-';
            }
        }
        cout<<setw(numJobsFixo*(5.5))<<deliveryOrder<<"\n\n";
    }

    vector<int> carryWeight(vector<data> &config, const vector<int> &jobSize, int K){

        //Danger: confirmar se funciona
        vector<int> carry (K, 0);

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

    void readInstance(ifstream &input, double &mi, double &delta, int &N, int &K,
                  vector<int> &P, vector<int> &d, vector<int> &s,
                  vector<double> &w, vector<int> &Q, vector<int> &F,
                  vector<vector<int>> &t)
    {
        input >> mi >> delta;
        
        input >> N >> K;

        P.resize(N); d.resize(N);
        s.resize(N); w.resize(N);

        Q.resize(K); F.resize(K);
        t.resize(N+1, vector<int>(N+1, 0));

        // Job Variables
        for(int i=0; i<N; i++) 
            input >> P[i];
        for(int i=0; i<N; i++) 
            input >> d[i];
        for(int i=0; i<N; i++) 
            input >> s[i];
        for(int i=0; i<N; i++) 
            input >> w[i];

        // Vehicle Variables
        for(int i=0; i<K; i++) 
            input >> Q[i];
        for(int i=0; i<K; i++) 
            input >> F[i];

        for(int i=0; i<= N; i++)
            for(int j=0; j<=N; j++)
                input >> t[i][j]; 
        
    }

    void getVariables(string &fileName, ifstream& input, int &metaheuristic, int &parameter1, int &parameter2){
        
        cout << "Input file name: ";
        getline(cin, fileName);

        input.open(fileName);

        cout << "Select ILS_RVND(1), ILS_RVND_2(2), GA_LS(3): ";
        cin >> metaheuristic;
        
        if(metaheuristic <= 2){
            
            cout << "Input restart times (maxIter): ";
            cin >> parameter1;

            cout << "Input max. iterations (maxIterIls): ";
            cin >> parameter2;

        }else if(metaheuristic == 3){

            cout << "Input Population Size (psize): ";
            cin >> parameter1;

        }
    }

#endif
