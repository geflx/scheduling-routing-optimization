#ifndef CORELS_H
#define CORELS_H

    using namespace std;

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

    double objFunction(const vector<vehicleLoaded> &vehicleOrder, const vector<data> &configuration,int ncars, int njobs, const vector<vector<int> > &time,
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
        for(int i=0; i<njobs; i++)
            penaltyCosts+= jobTardiness[i]* weight[i];
        
        return (travelCosts+vehicleCosts+penaltyCosts);
    }

    double objFVehicle(int i, const vector<vehicleLoaded> &vehicleOrder, const vector<data> &configuration,int ncars, int njobs, const vector<vector<int> > &time,
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

    vector<int > calculatingDeliveryTime(const vector<data> &config, vector<int> &startVehicleTime, const vector<vector<int> > &time,const vector<int> &procTime,
        const vector<vehicleLoaded> &vehicleOrder,int njobs){

        vector<int> deliveryTime(njobs, 0);
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

    vector<vehicleLoaded> generateVehicleOrder(const vector<data> &config,int ncars){

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

    vector<int> calculatingJobTardiness(const vector<int> &deliveryTime,int njobs, const vector<int> &dueDate){

        vector<int> T(njobs);

        for(int i=0; i<njobs; i++){

            T[i] = deliveryTime[i] - dueDate[i];

            if(T[i]<0)
                T[i]=0;
        }

        return T;
    }

    bool validConfig(const vector<data> &config, const vector<int> &capacities, const vector<int> &jobSize, int njobs, int ncars){

    int walk = 0;

    unordered_set<int> jobsIn;
    unordered_set<int> carIn;

    for(int i=0; i<config.size(); i++){

    	if( config[i].id == -1){

            cout << "Invalid ID (-1).\n";
    		return false;

        }
        
    	if( config[i].job ){

    		if(jobsIn.find(config[i].id) == jobsIn.end()){

    			jobsIn.insert(config[i].id);

            }else{

                cout << "Repeated job. \n";
    			return false;

            }
            // Job ID out of boundaries.
    		if( config[i].id<0 || config[i].id >= njobs){

                cout << "Job doesn't exist. \n";
    			return false;
            }

    	}else{

    		if(carIn.find(config[i].id ) == carIn.end()){

    			carIn.insert( config[i].id );

            }else{
                cout << "Repeated vehicle. \n";
    			return false; // Repeated vehicle.

            }

            // Vehicle ID out of boundaries.
    		if( config[i].id<0 || config[i].id >= ncars){

                cout << "Vehicle doesn't exist. \n";
    			return false;

            }
    	}
    }

    while(walk < config.size()){

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
                if( accCap > carCapacity){
                    
                    cout << "Overlapping vehicle capacity. \n";
                    return false;

                }
                
                ++walkAhead;
            }
        }
        walk = walkAhead;
    }

    return true;
}

    void checkS(const Solution &S){
        
        for(int i=0; i < (S.M[0].size()); i++){
            cout << "V" << S.M[0][i] << " ";
        }
        cout << endl;

        for(int i=0; i < (S.M[1].size()); i++){
            cout << "J" << S.M[1][i] << " ";
        }
        cout << endl;
    }

    void checkD(const vector<data> &v){
        
        for(data i: v){

            if(i.job)
                cout << "J";
            else
                cout << "V";

            cout << i.id << " ";
        }
        cout << endl;

    }
   
    // TODO check code
    vector<data> solution_to_data(const Solution &S, int N, int K){

        // Convert information from "Data" type to "Solution" type.

        vector<data> settings(N+K);

        unordered_map<int, int> jobDestiny;
        
        vector<int> vehiOrder;
        unordered_set<int> visited;

        // Get Job and Vehicles configuration.
        for(int i=0; i<N; i++){

            int k = S.M[0][i]; // Vehicle 'k'.

            jobDestiny[ i ] = k;

            if(visited.find( k ) == visited.end()){

                vehiOrder.push_back( k );
                visited.insert( k );

            }
        }

        // Insert in vehiOrder unused vehicles.
        for(int k=0; k<K; k++){

            if(visited.find( k ) == visited.end())
                vehiOrder.push_back(k);
        }


        unordered_map<int, vector<int>> vehiCarrySorted;

        // Set Job delivery order accordingly to S.M[1] sequence.
        for(int i=0; i<N; i++){

            int job = S.M[1][i];
            int k = jobDestiny[job]; 

            vehiCarrySorted[k].push_back(job);
        }

        int counter = 0;

        for(int i=0; i<K; i++){

            int k = vehiOrder[i];
            
            settings[counter].job = false;
            settings[counter].id = k;
            counter++;

            for(int job: vehiCarrySorted[k]){

                settings[counter].job = true;
                settings[counter].id = job;
                counter++;
            }
        }

        return settings;
    }

    Solution data_to_solution(const vector<data> &settings, int N, int K){

        // Convert information from "Data" type to "Solution" type.

        Solution S;
        S.alocate(N);

        // Counter and Vehicle ID.
        int cDown = 0;
        int k = -1;

        for(data i: settings){

            if( !i.job ){

                k = i.id;

            }else{

                assert(cDown < N);

                S.M[0][i.id] = k;
                S.M[1][cDown] = i.id;

                cDown++;                
            }
        }
        assert(S.M.size() == 2);
        return S;
    }

#endif