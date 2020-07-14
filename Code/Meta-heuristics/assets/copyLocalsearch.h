#ifndef LOCALSEARCH_H
#define LOCALSEARCH_H

    using namespace std;
    
    // * New Obj Function OK
    /*  Neighborhood 1: Swap sequential jobs inside a vehicle */
    bool nbhood1(char bestOrFirst, vector<data> &configuration, vector<vehicleLoaded> &vehicleOrder, int N, int K, const vector<double> &w,
        const vector<int> &P, const vector<vector<int>> &t, const vector<int> &F , const vector<int> &d){

        
        vector<int> startVehicleTime(K, 0);
        vector<int> deliveryTime = calculatingDeliveryTime(configuration,startVehicleTime,t,P,vehicleOrder,N);
        vector<int> T = calculatingJobTardiness(deliveryTime, N, d);

        vector<data> bestConfig;
        bool improved = false;
        double biggestImprove = 0;

        double initialVObj = objFunction(vehicleOrder,configuration, K, N, t, w, T, F);

        double bestObj = initialVObj;

        for(int i=0;i<vehicleOrder.size();i++){

            bool improvedCar = false;

            // IF There is just one job in vehicle, ignore ..
            if(vehicleOrder[i].initialPos == vehicleOrder[i].finalPos)
                continue; 
            
            for(int j=vehicleOrder[i].initialPos; j<vehicleOrder[i].finalPos; j++){

                for(int k= (j+1); k<=vehicleOrder[i].finalPos; k++){

                    swap( configuration[ j ], configuration[ k ]);

                    deliveryTime = calculatingDeliveryTime(configuration,startVehicleTime,t,P,vehicleOrder,N);
                    T = calculatingJobTardiness(deliveryTime, N, d);

                    double newObj = objFunction(vehicleOrder,configuration, K, N, t, w, T, F);

                    if(newObj < (bestObj - EPS)){

                        improved = true;
                        bestObj = newObj;
                        bestConfig = configuration;

                        if( bestOrFirst == 'F')
                            return true;
                    }
                    //Swap back the jobs
                    swap( configuration[ j ], configuration[ k ]);
                }
            }
        }

        if(improved){
            configuration = bestConfig;
            return true;
        }else{
            return false;
        }
    }

    /*  Neighborhood 2: Swap jobs between vehicles */
    bool nbhood2(char bestOrFirst, vector<data> &configuration, vector<vehicleLoaded> &vehicleOrder, int N, int K, const vector<double> &w,
        const vector<int> &P, const vector<vector<int>> &t, const vector<int> &F , const vector<int> &d, const vector<int> &Q, const vector<int> &s){

        vector<int> currCap(K, 0);

        for(int i=0; i<vehicleOrder.size(); i++){

            int whichVehicle = vehicleOrder[i].id;

            for(int j=vehicleOrder[i].initialPos; j<= vehicleOrder[i].finalPos; j++)
                currCap[ whichVehicle ] += s[ configuration[j].id ];
        }

        vector<int> startVehicleTime(K, 0);
        vector<int> deliveryTime = calculatingDeliveryTime(configuration,startVehicleTime,t,P,vehicleOrder,N);
        vector<int> T = calculatingJobTardiness(deliveryTime, N, d);


        double bestObj = objFunction(vehicleOrder,configuration, K, N, t, w, T, F);
        double iniObj = bestObj;
        bool improved = false;
        vector<data> bestConfig = configuration;

        for(int i=0; i<vehicleOrder.size()-1; i++){
            for(int j= vehicleOrder[i].initialPos; j<=vehicleOrder[i].finalPos; j++){
                for(int next= i+1; next<vehicleOrder.size(); next++){

                    deliveryTime = calculatingDeliveryTime(configuration,startVehicleTime,t,P,vehicleOrder,N);
                    T = calculatingJobTardiness(deliveryTime, N, d);

                    for(int k= vehicleOrder[next].initialPos; k<=vehicleOrder[next].finalPos; k++){

                        //Checking if the capacity of each car will be respected after swap
                        if( (currCap[ vehicleOrder[ i ].id]- s[ configuration[j].id ]+ s[ configuration[k].id ]) <= Q[ vehicleOrder[i].id]
                            && (currCap[ vehicleOrder[ next ].id] - s[configuration[k].id]+ s[ configuration[j].id]) <= Q[ vehicleOrder[next].id] ){

                            swap( configuration[j], configuration[k]);

                            deliveryTime = calculatingDeliveryTime(configuration,startVehicleTime,t,P,vehicleOrder,N);
                            T = calculatingJobTardiness(deliveryTime, N, d);

                            double newDoubleConfig = objFunction(vehicleOrder,configuration, K, N, t, w, T, F);

                            if( newDoubleConfig < bestObj ){

                                improved = true;
                                bestConfig = configuration;
                                bestObj = newDoubleConfig;

                                if( bestOrFirst == 'F' ) //If call is First Improvement, return.
                                    return true;
                            }
                            swap( configuration[j], configuration[k]);
                        }
                    }
                }
            }
        }

        if( improved ){
            configuration = bestConfig;
            return true;
        }else{
            return false;
        }
    }

    /*  Neighborhood 3: Inserting jobs in positions inside a vehicle */
    bool nbhood3(char bestOrFirst, vector<data> &configuration, vector<vehicleLoaded> &vehicleOrder, int N, int K, const vector<double> &w,
        const vector<int> &P, const vector<vector<int>> &t, const vector<int> &F , const vector<int> &d){

        vector<int> startVehicleTime(K, 0);
        vector<int> deliveryTime = calculatingDeliveryTime(configuration,startVehicleTime,t,P,vehicleOrder,N);
        vector<int> T = calculatingJobTardiness(deliveryTime, N, d);

        double bestImprove = 0;
        bool improved = false;
        vector<data> bestConfig = configuration;

        double iniObj = objFunction(vehicleOrder,configuration, K, N, t, w, T, F);

        for(int i=0; i< vehicleOrder.size(); i++){

            if(vehicleOrder[i].initialPos == vehicleOrder[i].finalPos)
                continue;
            
            double bestObj = objFVehicle(i, vehicleOrder,configuration, K, N, t, w, T, F);

            //Saving the original Jobs Configuration of vehicle
            vector<int> original;
            for(int k = vehicleOrder[i].initialPos; k <= vehicleOrder[i].finalPos; k++)
                    original.push_back( configuration[ k ].id );
            
            for(int j = vehicleOrder[i].initialPos; j <= vehicleOrder[i].finalPos; j++){

                int insertLeftward = j - vehicleOrder[i].initialPos - 1;

                int contUpward = 0;

                //Inserting jobs leftward (except for the immediatly close)
                while( insertLeftward-- > 0){

                    vector<int> newOrder ( vehicleOrder[i].finalPos - vehicleOrder[i].initialPos +1, -1);
                    set<int> jobsIn;

                    int insertCounter = 0;

                    newOrder[ contUpward ] = configuration[j].id; //Inserting job in the required place
                    jobsIn.insert( configuration[j].id );

                    for(int k = vehicleOrder[i].initialPos; k <= vehicleOrder[i].finalPos; k++){

                        if( jobsIn.find( configuration[k].id ) == jobsIn.end()){

                            while( insertCounter < newOrder.size() && newOrder[insertCounter]!=-1)
                                insertCounter++;
                            
                            newOrder[ insertCounter ] = configuration[k].id;
                            jobsIn.insert( configuration[k].id );
                        }
                    }

                    int CC = 0; // Copy Counter

                    for(int k = vehicleOrder[i].initialPos; k <= vehicleOrder[i].finalPos; k++){
                        configuration[ k ].id = newOrder[ CC ];
                        CC++;
                    }

                    deliveryTime = calculatingDeliveryTime(configuration, startVehicleTime, t, P, vehicleOrder, N);
                    T = calculatingJobTardiness(deliveryTime, N, d);
                    double newObj = objFVehicle(i, vehicleOrder, configuration, K, N, t, w, T, F);

                    if( newObj < bestObj && (newObj-bestObj) < bestImprove) {

                        bestImprove= (newObj-bestObj);
                        improved = true;
                        bestConfig = configuration;

                        //If it is a First Improvement call, return
                        if( bestOrFirst == 'F')
                            return true;
                    }

                    CC = 0;

                    for(int k = vehicleOrder[i].initialPos; k <= vehicleOrder[i].finalPos; k++){
                        configuration[ k ].id = original[ CC ];
                        CC++;
                    }

                    contUpward++;
                }

                int jobPosition = j - vehicleOrder[i].initialPos + 1;
                int jobsNb = j + 1 ;

                while(jobsNb <= vehicleOrder[i].finalPos){

                    vector<int> newOrder(vehicleOrder[i].finalPos - vehicleOrder[i].initialPos + 1, -1);
                    set<int> jobsIn;


                    newOrder[jobPosition] = configuration[j].id;
                    jobsIn.insert( configuration[ j ].id );

                    int insertCounterVec = 0;

                    for(int k = vehicleOrder[i].initialPos; k <= vehicleOrder[i].finalPos; k++){

                        if( jobsIn.find( configuration[k].id ) == jobsIn.end()){

                            while( insertCounterVec < newOrder.size() && newOrder[insertCounterVec]!=-1)
                                insertCounterVec++;
                            
                            newOrder[ insertCounterVec ] = configuration[k].id;
                            jobsIn.insert( configuration[k].id );
                            insertCounterVec++;
                        }
                    }

                    int CC = 0; // Copy Counter

                    for(int k = vehicleOrder[i].initialPos; k <= vehicleOrder[i].finalPos; k++){
                        configuration[ k ].id = newOrder[ CC ];
                        CC++;
                    }

                    deliveryTime = calculatingDeliveryTime(configuration, startVehicleTime, t, P, vehicleOrder, N);
                    T = calculatingJobTardiness(deliveryTime, N, d);
                    double newObj = objFVehicle(i, vehicleOrder, configuration, K, N, t, w, T, F);

                    if( (newObj < (bestObj-EPS)) && (newObj-bestObj-EPS) < bestImprove) {

                        bestImprove= (newObj-bestObj);
                        improved = true;
                        bestConfig = configuration;

                        //If it is a First Improvement call
                        if( bestOrFirst == 'F')
                            return true;
                    }

                    CC=0;

                    for(int k = vehicleOrder[i].initialPos; k <= vehicleOrder[i].finalPos; k++){
                        configuration[ k ].id = original[ CC ];
                        CC++;
                    }

                    jobsNb++;
                    jobPosition++;
                }
            }
        }

        if( improved ){
            configuration = bestConfig;
            return true;
        }else{
            return false;
        }
    }

    /*  Neighborhood 4: Inserting jobs in positions inside OTHERS vehicles */
    bool nbhood4(char bestOrFirst, vector<data> &configuration, vector<vehicleLoaded> &vehicleOrder, int N, int K, const vector<double> &w,
        const vector<int> &P, const vector<vector<int>> &t, const vector<int> &F , const vector<int> &d, const vector<int> &Q, const vector<int> &s){

        //Calculating the current car volume
        vector<int> currCap(K, 0);
        vector<bool> carInUse(K, false);

        vector<int> startVehicleTime(K, 0);
        vector<int> deliveryTime = calculatingDeliveryTime(configuration, startVehicleTime, t, P, vehicleOrder, N);
        vector<int> T = calculatingJobTardiness(deliveryTime, N, d);

        double bestObj = objFunction(vehicleOrder, configuration, K, N, t, w, T, F);
        double iniObj = bestObj;
        double improved = false;
        vector<data> bestConfig = configuration;
        vector<vehicleLoaded> bestVOrder = vehicleOrder;

        for(int i=0; i < vehicleOrder.size(); i++){

            int whichVehicle = vehicleOrder[i].id;
            carInUse[ whichVehicle ] = true;

            for(int j=vehicleOrder[i].initialPos; j <= vehicleOrder[i].finalPos; j++)
                currCap[ whichVehicle ] += s[ configuration[j].id ];
        }

        for(int i=0; i < vehicleOrder.size(); i++){

            int carsBeforeMe = i;
            int carsAfterMe = vehicleOrder.size() - i - 1;

            int tempBefore = carsBeforeMe;

            while(tempBefore > 0){

                --tempBefore;

                //For each job in vehicle...
                for(int j=vehicleOrder[i].initialPos; j <= vehicleOrder[i].finalPos; j++){
                    
                    //IF the car before me can handle carrying this job, we calculate the benefit int puting the job on it
                    if( (currCap[ vehicleOrder[ tempBefore ].id ] + s[ configuration[j].id ]) <= (Q[ vehicleOrder[tempBefore].id] )){

                        int range = vehicleOrder[ tempBefore].finalPos - vehicleOrder[ tempBefore ].initialPos + 2;
                        int idCarInsert = vehicleOrder[ tempBefore ].id;

                        //Trying to put in each space avaiable on the other car
                        for(int z=0; z < range; z++){

                            vector<int> newcarried( range ,-1);

                            newcarried[ z ] = configuration[j].id;
                            int copy= vehicleOrder[ tempBefore ].initialPos;
                            for(int u=0; u< range; u++){
                                if(z==u)
                                    continue;
                                newcarried[ u ] = configuration[ copy++ ].id ;
                            }

                            vector<data> newConfig( configuration.size());

                            int copyConfig =0;
                            int contInsertNew = 0;

                            while( copyConfig < configuration.size() ){

                                if( configuration[ copyConfig ].job && configuration[ copyConfig ].id == configuration[j].id){
                                    //Job is moved
                                    copyConfig++;
                                    continue;
                                }

                                if( !configuration[ copyConfig ].job && configuration[ copyConfig ].id == idCarInsert){
                                    
                                    newConfig[ contInsertNew ] = configuration[ copyConfig ];

                                    copyConfig++;
                                    contInsertNew++;

                                    int insert = range;
                                    int contUpward = 0;

                                    while(insert > 0){
                                        newConfig[ contInsertNew ].job = true;
                                        newConfig[ contInsertNew ].id = newcarried[ contUpward ];
                                        
                                        contUpward++;
                                        contInsertNew++;
                                        copyConfig++;

                                        insert--;
                                    }

                                    copyConfig--; //Deleting the extra job inserted
                                    continue;
                                }

                                newConfig[ contInsertNew ] = configuration[ copyConfig ];
                                contInsertNew++;
                                copyConfig++;
                            }

                            vector<vehicleLoaded> newVehicleOrder = generateVehicleOrder(newConfig, K);//Generating info about vehicle transportation

                            //Creating new configuration
                            vector<int> newStartVehicleTime(K, 0);
                            vector<int> newDeliveryTime = calculatingDeliveryTime(newConfig,newStartVehicleTime, t, P, newVehicleOrder, N);
                            vector<int> newT = calculatingJobTardiness(newDeliveryTime, N, d);

                            double newObj=0;
                            newObj = objFunction(newVehicleOrder, newConfig, K, N, t, w, newT, F);

                            if(newObj < bestObj && newObj!=0){

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

            int carAhead = i+1 ;

            while(carAhead < vehicleOrder.size()  ){

                //For each job in vehicle ...
                for(int j = vehicleOrder[i].initialPos; j <= vehicleOrder[i].finalPos; j++){

                    //IF the car before me can handle carrying this job, we calculate the benefit int puting the job on it
                    if( (currCap[ vehicleOrder[ carAhead ].id ] + s[ configuration[j].id ]) <= (Q[ vehicleOrder[carAhead].id] )){

                        int range = vehicleOrder[ carAhead].finalPos - vehicleOrder[ carAhead ].initialPos + 2;
                        int idCarInsert = vehicleOrder[ carAhead ].id;

                        //Trying to put in each space avaiable on the other car
                        for(int z=0; z < range; z++){

                            vector<int> newcarried(range , -1);

                            newcarried[ z ] = configuration[j].id;
                            int copy= vehicleOrder[ carAhead ].initialPos;

                            for(int u=0; u<range; u++){
                                if(z==u)
                                    continue;
                                newcarried[ u ] = configuration[ copy++ ].id ;
                            }

                            vector<data> newConfig(configuration.size());

                            int copyConfig=0;
                            int contInsertNew = 0;

                            while( copyConfig < configuration.size()){

                                if( configuration[ copyConfig ].job && configuration[ copyConfig ].id == configuration[j].id){
                                    //Job is moved
                                    copyConfig++;
                                    continue;
                                }

                                if(!configuration[ copyConfig ].job && configuration[ copyConfig ].id == idCarInsert){

                                    newConfig[ contInsertNew ] = configuration[ copyConfig ];

                                    copyConfig++;
                                    contInsertNew++;

                                    int insert = range;
                                    int contUpward = 0;

                                    while(insert > 0  ){ 
                                        
                                        newConfig[ contInsertNew ].job = true;
                                        newConfig[ contInsertNew ].id = newcarried[ contUpward ];
                                        
                                        contUpward++;
                                        contInsertNew++;
                                        copyConfig++;

                                        insert--;
                                    }

                                    copyConfig--; //Deleting inserted extra job
                                    continue;
                                }

                                if( contInsertNew > configuration.size() || copyConfig > configuration.size())
                                    break;

                                newConfig[ contInsertNew ] = configuration[ copyConfig ];

                                contInsertNew++;
                                copyConfig++;
                            }

                            vector<vehicleLoaded> newVehicleOrder=generateVehicleOrder(newConfig, K);//Generating info about vehicle transportation
                            
                            //Creating new configuration
                            vector<int> newStartVehicleTime(K, 0);
                            vector<int> newDeliveryTime = calculatingDeliveryTime(newConfig, newStartVehicleTime, t, P,newVehicleOrder, N);
                            vector<int> newT = calculatingJobTardiness(newDeliveryTime, N, d);

                            double newObj = 0;
                            newObj = objFunction(newVehicleOrder, newConfig, K, N, t, w, newT, F);

                            if(newObj < bestObj && newObj!=0){

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
                ++carAhead;
            }
        }

        for(int j=0; j<K; j++){

            // Inserting in free space vehicles
            if(!carInUse[ j ]){

                for(int i=0;i<vehicleOrder.size();i++){

                    vector<int> startVehicleTime(K, 0);
                    vector<int> deliveryTime = calculatingDeliveryTime(configuration, startVehicleTime, t, P, vehicleOrder, N);
                    vector<int> T = calculatingJobTardiness(deliveryTime, N, d);

                    //For each task in this car...
                    for(int k = vehicleOrder[i].initialPos; k <= vehicleOrder[i].finalPos; k++){

                        if(Q[ j ]>= s[ configuration[k].id ]){

                            vector<data> newConfig (configuration.size());

                            int insert = 0;
                            for(int z=0; z < configuration.size(); z++){

                                if( !configuration[z].job && configuration[z].id== j){ //The new car to insert each job

                                    newConfig[insert++] = configuration[z];
                                    newConfig[ insert ].job = true;
                                    newConfig[ insert ].id = configuration[k].id;
                                    ++insert;
                                }else if( configuration[z].job && configuration[z].id == configuration[k].id){
                                    continue;
                                }else{
                                    newConfig[ insert++ ] = configuration[z];
                                }
                            }

                            //Checking if OBJ function improves with this change
                            vector<vehicleLoaded> newVehicleOrder=generateVehicleOrder(newConfig, K);//Generating info about vehicle transportation
                            vector<int> newStartVehicleTime(K, 0);
                            vector<int> newDeliveryTime = calculatingDeliveryTime(newConfig, newStartVehicleTime, t, P, newVehicleOrder, N);
                            vector<int> newT = calculatingJobTardiness(newDeliveryTime, N, d);

                            double newObj = objFunction(newVehicleOrder, newConfig, K, N, t, w, newT, F);

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

            return true;
        }else{
            return false;
        }
    }

    /*  Neighborhood 5: Swap entire vehicle (with all its jobs) with another ones */
    bool nbhood5(char bestOrFirst, vector<data> &config, vector<vehicleLoaded> &vehicleOrder, int N, int K, const vector<double> &w,
        const vector<int> &P, const vector<vector<int>> &t, const vector<int> &F , const vector<int> &d){


        vector<int> startVehicleTime(K, 0);
        vector<int> deliveryTime = calculatingDeliveryTime(config, startVehicleTime, t, P, vehicleOrder, N);
        vector<int> T = calculatingJobTardiness(deliveryTime, N, d);

        double iniOfEverything = objFunction(vehicleOrder, config, K, N, t, w, T, F);
        double iniObj = iniOfEverything;
        bool improvedBest = false;

        vector<data> bestConfig;
        vector<vehicleLoaded> bestVOrder;

        for(int i=0; i< vehicleOrder.size()-1; i++){

            int nbJobs = vehicleOrder[i].finalPos - vehicleOrder[i].initialPos +1 ;

            vector<data> tempOut( nbJobs + 1 ); 

            // Copying vehicle and its job configuration
            tempOut[ 0 ] = config[ vehicleOrder[i].initialPos-1 ];

            for(int i1=1; i1<=nbJobs; i1++) 
                tempOut[i1] = config[ vehicleOrder[i].initialPos +(i1-1) ];

            int vehicleOut = config[ vehicleOrder[i].initialPos-1 ].id; 

            for(int j=i+1; j < vehicleOrder.size(); j++){

                int nbJobsIn = vehicleOrder[j].finalPos - vehicleOrder[j].initialPos +1 ;

                vector<data> tempIn ( nbJobsIn + 1); 

                tempIn[ 0 ] = config[ vehicleOrder[j].initialPos-1 ]; 

                for(int i1=1; i1 <= nbJobsIn; i1++) 
                    tempIn[ i1 ] = config[ vehicleOrder[j].initialPos +(i1-1)  ];

                int vehicleIn = config[ vehicleOrder[j].initialPos-1 ].id;

                // Swapping vehicles
                int copy = 0;
                int insert = 0;

                vector<data> newConfig(config.size());

                while(copy < config.size()){

                    if(!config[copy].job && config[copy].id!=vehicleOut && config[copy].id!=vehicleIn){

                        int next = copy + 1;

                        newConfig[ insert++ ] = config[copy]; //copy vehicle

                        while( next < config.size() && config[next].job){
                            newConfig[ insert ] = config[ next ];
                            next++; 
                            insert++;
                        }

                        copy = next;

                    }else if(!config[copy].job && config[copy].id== vehicleOut){

                        int cont = 0;
                        
                        while( cont <= nbJobsIn ){
                            newConfig[ insert ]= tempIn[ cont ];
                            insert++; 
                            cont++;
                        }
                        
                        copy += nbJobs+1;

                    }else if( !config[ copy].job && config[copy].id == vehicleIn){

                        int cont = 0;
                        while( cont <= nbJobs ){
                            newConfig[ insert ]= tempOut[ cont ];
                            insert++; 
                            cont++;
                        }
                        copy += nbJobsIn+1;
                    }
                }

                // Catching new Obj Function value
                vector<vehicleLoaded> newVehicleOrder=generateVehicleOrder(newConfig, K);
                vector<int> newStartVehicleTime(K, 0);
                vector<int> newDeliveryTime = calculatingDeliveryTime(newConfig,newStartVehicleTime, t, P, newVehicleOrder, N);
                vector<int> newT = calculatingJobTardiness(newDeliveryTime, N, d);

                double newObj = objFunction(newVehicleOrder, newConfig, K, N, t, w, newT, F);

                if(newObj < iniObj){
                    if( bestOrFirst == 'F'){ // First Improvement 

                        vehicleOrder = newVehicleOrder;
                        config = newConfig;

                        return true;

                    }else{ // Best Improvement

                        iniObj = newObj;
                        bestConfig = newConfig;
                        bestVOrder = newVehicleOrder;

                        improvedBest = true;
                    }
                }
            }
        }

        if( improvedBest && bestOrFirst == 'B'){

            config = bestConfig;
            vehicleOrder = bestVOrder;

            return true;
        }

        return false;
    }

    /*  Neighborhood 6: Insert entire vehicle (with all its jobs) in  another positions, except for the immediately before */
    bool nbhood6(char bestOrFirst, vector<data> &config, vector<vehicleLoaded> &vehicleOrder, int N, int K, const vector<double> &w,
        const vector<int> &P, const vector<vector<int>> &t, const vector<int> &F , const vector<int> &d){

        vector<int> startVehicleTime(K, 0);
        vector<int> deliveryTime = calculatingDeliveryTime(config, startVehicleTime, t, P, vehicleOrder, N);
        vector<int> T = calculatingJobTardiness(deliveryTime, N, d);

        double iniOfEverything = objFunction(vehicleOrder, config, K, N, t, w, T, F);
        double iniObj = iniOfEverything;
        bool improvedBest = false;

        //Caution, data not initialized
        vector<data> bestConfig;
        vector<vehicleLoaded> bestVOrder;

        //Saving original car positions in the received config
        vector<int> carPos(K, -1);

        int contcar=0;

        for(int i=0; i < config.size(); i++){
            if(!config[i].job){
                carPos[contcar] = config[i].id;
                contcar++;
            }
        }

        unordered_set<int> inUse; //vehicles in use
        unordered_map<int,int> locate; //first == vehicle, second == position in array

        for(int i=0; i < vehicleOrder.size(); i++){
            inUse.insert( vehicleOrder[i].id );
            locate[ vehicleOrder[i].id ] = vehicleOrder[i].initialPos;
        }

        //  Inserting using Forward Swap cars with Adjacent Vehicles
        for(int i=0; i<K-1; i++){
            
            //Inserting in vehicles in use.
            if( inUse.find( carPos[i] ) == inUse.end())
                continue; //not in use

            vector<int> newCarPos = carPos;

            for(int j=i+1; j < K; j++){

                int carJ = newCarPos[j];
                swap(newCarPos[i], newCarPos[j]);

                //Generating this solution if car j is in use!
                if( inUse.find(carJ) == inUse.end())
                    continue; //continue swapping, because this swap will not increase the obj function

                //Generating new configuration
                vector<data> newConfig(config.size());

                int insert = 0;

                for(int k=0; k < K; k++){

                    if(inUse.find(newCarPos[k]) == inUse.end()){ // Case vehicle NOT in use

                        newConfig[ insert ].job = false;
                        newConfig[ insert++ ].id = newCarPos[k];

                    }else{ // Case vehicle IS in use

                        newConfig[ insert ].job = false;
                        newConfig[ insert++ ].id = newCarPos[k];

                        int copy = locate[ newCarPos[k] ];

                        while(copy < config.size() && config[copy].job){

                            newConfig[ insert ].job = true;
                            newConfig[ insert++ ].id = config[copy].id;

                            copy++;
                        }
                    }
                }

                //Checking new OBJ function value
                vector<vehicleLoaded> newVehicleOrder=generateVehicleOrder(newConfig, K);
                vector<int> newStartVehicleTime(K, 0);
                vector<int> newDeliveryTime = calculatingDeliveryTime(newConfig, newStartVehicleTime, t, P, newVehicleOrder, N);
                vector<int> newT = calculatingJobTardiness(newDeliveryTime,N,d);

                double newObj = objFunction(newVehicleOrder,newConfig, K, N, t, w, newT, F);

                if( newObj < iniObj){

                    if( bestOrFirst == 'F'){
                        vehicleOrder = newVehicleOrder;
                        config = newConfig;

                        return true;
                    }else{
                        iniObj = newObj;

                        bestConfig = newConfig;
                        bestVOrder = newVehicleOrder;
                        
                        improvedBest = true;
                    }
                }
            }
        }

        //Swap backwards except for the immediatly before.
        for(int i = K-1; i>0; i--){

            //Vehicle NOT IN USE, ignore it
            if(inUse.find( carPos[i] ) == inUse.end())
                continue; 

            vector<int> newCarPos = carPos;

            for(int j=i-1; j>=0; j--){

                int carJ= newCarPos[j];
                swap(newCarPos[i], newCarPos[j]);

                //Generating this solution if car j is in use!
                if( j==(i-1) || inUse.find( carJ) == inUse.end())
                    continue; 

                //Generating new configuration
                vector<data> newConfig( config.size() );
                int insert = 0;

                for(int k=0; k < K; k++){

                    if( inUse.find( newCarPos[k]) == inUse.end()){ // Vehicle IN use

                        newConfig[ insert ].job = false;
                        newConfig[ insert++].id = newCarPos[k];

                    }else{ // Vehicle NOT in use

                        newConfig[ insert ].job= false;
                        newConfig[ insert++ ].id = newCarPos[k];

                        int copy = locate[ newCarPos[k] ];

                        while( copy < config.size() && config[ copy ].job){

                            newConfig[ insert ].job = true;
                            newConfig[ insert++ ].id = config[copy].id;

                            copy++;
                        }
                    }
                }

                //Checking new config OBJ!
                vector<vehicleLoaded> newVehicleOrder=generateVehicleOrder(newConfig, K);
                vector<int> newStartVehicleTime(K, 0);
                vector<int> newDeliveryTime = calculatingDeliveryTime(newConfig, newStartVehicleTime, t, P, newVehicleOrder, N);
                vector<int> newT = calculatingJobTardiness(newDeliveryTime, N, d);

                double newObj = objFunction(newVehicleOrder,newConfig, K, N, t, w, newT, F);

                if(newObj < iniObj){

                    if( bestOrFirst == 'F'){

                        vehicleOrder = newVehicleOrder;
                        config = newConfig;

                        return true;
                    }else{
                        iniObj = newObj;
                        
                        bestConfig = newConfig;
                        bestVOrder = newVehicleOrder;

                        improvedBest = true;
                    }
                }
            }
        }

        if(improvedBest && bestOrFirst == 'B'){

            config = bestConfig;
            vehicleOrder = bestVOrder;

            return true;

        }

        return false;
    }

    /*  Neighborhood 7: 2-OPT */
    bool nbhood7(char bestOrFirst, vector<data> &config, vector<vehicleLoaded> &vehicleOrder, int N, int K, const vector<double> &w,
        const vector<int> &P, const vector<vector<int>> &t, const vector<int> &F , const vector<int> &d, const vector<int> &Q, const vector<int> &s){

        if(!validConfig(config, Q, s, N, K))
            return false;
        
        vector<int> startVehicleTime(K, 0);
        vector<int> deliveryTime = calculatingDeliveryTime(config, startVehicleTime, t, P, vehicleOrder, N);
        vector<int> T = calculatingJobTardiness(deliveryTime, N, d);

        vector<data> bestConfig;
        bool improved = false;
        double biggestImprove = 0;

        double initialObj = objFunction(vehicleOrder,config, K, N, t, w, T, F);
        double bestObj = initialObj;

        vector<int> carUsed;

        //The position of each car 'i' will be referred in edgesByCar[i]
        vector<vector< pair<int, int>>> edgesByCar( K);
        vector<int> jobPos ( N );

        for(int i=0;i < config.size(); i++){
            if( config[i].job)
                jobPos[ config[i].id ]= i;
        }

        //Generating edges.
        for(int i=0; i < vehicleOrder.size(); i++){

            int beg = vehicleOrder[i].initialPos; 
            int end = vehicleOrder[i].finalPos;

            //Checking if there is enough edges to do 2-opt in each car.
            if( (end-beg) <= 2 )
                continue;
            
            int carId = vehicleOrder[i].id;
            carUsed.push_back( carId );

            for(int j = beg; j < end; j++)
                edgesByCar[ carId ].push_back({config[j].id, config[j+1].id});
        }

        //Randomize vehicles.
        random_shuffle( carUsed.begin(), carUsed.end());

        for(int i=0; i < carUsed.size(); i++){

            int car = carUsed[ i ];

            random_shuffle( edgesByCar[ car ].begin(), edgesByCar[ car ].end());

            for(int j=0; j < edgesByCar[ car].size()-1; j++){

                for(int k=j+1; k < edgesByCar[ car ].size(); k++){

                    set<int> points;
                    int p1, p2, p3, p4;

                    p1 = edgesByCar[ car ][j].first;
                    p2 = edgesByCar[ car ][j].second;
                    p3 = edgesByCar[ car ][k].first;
                    p4 = edgesByCar[ car ][k].second;

                    points.insert(p1);
                    points.insert(p2);
                    points.insert(p3);
                    points.insert(p4);

                    if( points.size() < 4) // ADJACENT EDGES aren't valid.
                        continue;
                    
                    vector<data> newConfig = config;

                    // Do 2-OPT Changes.
                    swap(config[ jobPos[ p2 ]], config[ jobPos[ p3 ]]);

                    // Checking new OBJ Function value.
                    vector<int> newstartVehicleTime(K, 0);
                    vector<int> newdeliveryTime = calculatingDeliveryTime(config, newstartVehicleTime, t, P, vehicleOrder, N);
                    vector<int> newT = calculatingJobTardiness(newdeliveryTime, N, d);

                    double newObj = objFunction(vehicleOrder, config, K, N, t, w, newT, F);

                    if( newObj < bestObj){

                        bestObj = newObj;
                        bestConfig = config;
                        improved = true;

                        if( bestOrFirst == 'F')
                            return true;                        
                    }
                    // Undo 2-OPT Changes.
                    swap( config[ jobPos[ p2 ]], config[ jobPos[ p3 ]] );
                }
            }
        }

        if( bestOrFirst == 'F'){

            if(!improved)
                return false;

        }else{ // Best Improvement.

            if(improved){ 
                config = bestConfig;
                return true;
            }else{
                return false;
            }
        }
    }
    

//pair< ValueObjFunction, validSolution>
pair<double, vector<data>> RVND_Custom(bool reuse, int N, int K, const vector<double> &w,const vector<int> &P, const vector<vector<int>> &t,
   const vector<int> &F , const vector<int> &d, const vector<int> &Q, const vector<int> &s, vector<data> &initialConfig){

	if( !reuse ){
        initialConfig = generateValidRandomConfig(N,K,Q,s);
    }

	vector<int> interRoute = {2, 4, 5, 6};
	random_shuffle( interRoute.begin(), interRoute.end());
	int it=0;

	vector<vehicleLoaded> vehicleOrder = generateVehicleOrder(initialConfig, K);//Generating info about vehicle transportation
    vector<int> startVehicleTime(K, 0);//Calculating Delivery time(D) and Starting time(Sk)
    vector<int> deliveryTime = calculatingDeliveryTime(initialConfig, startVehicleTime, t, P, vehicleOrder, N);
    vector<int> jobTardiness = calculatingJobTardiness(deliveryTime, N, d);//Generating Job Tardiness (T) of each job (O(N))

    double bestObj = objFunction(vehicleOrder, initialConfig, K, N, t, w, jobTardiness, F);
    double iniOfEverything = bestObj;
    bool generalImprove = false;

    //Caution Unitialized data
    vector<data> bestConfig;

	while( it < 4){

		int neighbor = interRoute[ it ];
		bool improved = false;

		switch(neighbor){
			case 2:
            	improved = nbhood2('B',initialConfig, vehicleOrder, N, K, w, P, t, F, d, Q, s);
			case 4:
            	improved = nbhood4('B',initialConfig, vehicleOrder, N, K, w, P, t, F, d ,Q, s);
			case 5:
            	improved = nbhood5('B',initialConfig, vehicleOrder, N, K, w, P, t, F, d);
			case 6:
            	improved = nbhood6('B',initialConfig, vehicleOrder, N, K, w, P, t, F, d);
		}

		if( improved ){
			generalImprove = true;
			while( true ){ //intra route
				if( nbhood1('B',initialConfig, vehicleOrder, N, K, w, P, t, F, d)){
					continue;
				}
				if( nbhood3('B',initialConfig, vehicleOrder, N, K, w, P, t, F, d)){
					continue;
				}
                if( nbhood7('B',initialConfig, vehicleOrder, N, K, w, P, t, F, d,Q,s)){
					continue;
				}
				break;
			}

			bestConfig = initialConfig;
			it = 0;
			random_shuffle( interRoute.begin(), interRoute.end());

		}else{
			it++;
		}
	}

	if( generalImprove)
		initialConfig = bestConfig;

	vehicleOrder=generateVehicleOrder(initialConfig,K);//Generating info about vehicle transportation
    deliveryTime = calculatingDeliveryTime(initialConfig,startVehicleTime,t,P,vehicleOrder,N);
    jobTardiness = calculatingJobTardiness(deliveryTime, N, d);//Generating Job Tardiness (T) of each job (O(N))
    double result= objFunction(vehicleOrder,initialConfig,K,N,t,w,jobTardiness,F);


	return {result, initialConfig};
}

    //pair< ValueObjFunction, validSolution>
    pair<double, vector<data>> RVND(bool reuse, int N, int K, const vector<double> &w,const vector<int> &P, const vector<vector<int>> &t,
    const vector<int> &F , const vector<int> &d, const vector<int> &Q, const vector<int> &s, vector<data> &initialConfig){


        //Checking if it is needed to generate a new one or if the current solution will be used
        if( !reuse ){
            initialConfig = randomConfigSequential(N,K,Q,s);
        }

        vector<vehicleLoaded> vehicleOrder=generateVehicleOrder(initialConfig,K);//Generating info about vehicle transportation
        vector<int> startVehicleTime(K, 0);//Calculating Delivery time(D) and Starting time(Sk)
        vector<int> deliveryTime = calculatingDeliveryTime(initialConfig,startVehicleTime,t,P,vehicleOrder,N);
        vector<int> jobTardiness = calculatingJobTardiness(deliveryTime, N, d);//Generating Job Tardiness (T) of each job (O(N))

        double iniObj= objFunction(vehicleOrder,initialConfig,K,N,t,w,jobTardiness,F);

        vector<int> neighbors = {1,2,3,4,5,6,7};


        random_shuffle( neighbors.begin(), neighbors.end());

        int it = 0;

        while( it < 7 ){

            int whichNeighbor = neighbors[ it ];

            bool improved = false;

            if( whichNeighbor == 1){
                improved = nbhood1('B',initialConfig, vehicleOrder, N, K, w, P, t, F, d);
            }else if( whichNeighbor == 2){
                improved = nbhood2('B',initialConfig, vehicleOrder, N, K, w, P, t, F, d ,Q, s);
            }else if( whichNeighbor == 3){
                improved = nbhood3('B',initialConfig, vehicleOrder, N, K, w, P, t, F, d);
            }else if(whichNeighbor ==4){
                improved = nbhood4('B',initialConfig, vehicleOrder, N, K, w, P, t, F, d ,Q, s);
            }else if(whichNeighbor == 5){
                improved = nbhood5('B',initialConfig, vehicleOrder, N, K, w, P, t, F, d);
            }else if(whichNeighbor == 6){
                improved = nbhood6('B',initialConfig, vehicleOrder, N, K, w, P, t, F, d);
            }else if(whichNeighbor == 7){
                improved = nbhood7('B',initialConfig, vehicleOrder, N, K, w, P, t, F, d, Q, s);
            }

            if( improved ){
                it = 0;
                random_shuffle( neighbors.begin(), neighbors.end());
            }else{
                it++;
            }

        }

        vehicleOrder=generateVehicleOrder(initialConfig,K);//Generating info about vehicle transportation
        deliveryTime = calculatingDeliveryTime(initialConfig,startVehicleTime,t,P,vehicleOrder,N);
        jobTardiness = calculatingJobTardiness(deliveryTime, N, d);//Generating Job Tardiness (T) of each job (O(N))

        double finalObj= objFunction(vehicleOrder,initialConfig,K,N,t,w,jobTardiness,F);

        return {finalObj, initialConfig};

    }

#endif