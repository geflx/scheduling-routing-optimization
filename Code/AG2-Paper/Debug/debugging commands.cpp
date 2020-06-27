
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

    cout << "TRAVEL2: "<< travelCosts << "PENALTY2: " << penaltyCosts << "USE2: " << vehicleCosts <<endl;
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
auto it2 = Schedule.begin();

while(it2 != Schedule.end()){
    cout<<"Veiculo "<< it2->first<<" tem: " ;
    for(int i: it2->second){
        cout << i << " ";
    }
    cout<<endl;
    it2++;
}
cout << "A ordem e: ";
for(int i: Order){
    cout << i << " ";
}cout << endl;
cout<<"ACC P TIME SIZE: "<<accPTime.size()<<endl;
auto it = accPTime.begin();

while(it != accPTime.end()){
    cout<<"Veiculo "<<it->first<< " carrega "<< it->second << endl;
    it++;
}

if(OverlapCosts == 0){
    
    for(int i=0;i<N;i++){
        cout<<"V"<<S.M[0][i]<<" ";
    }cout<<endl;
    for(int i=0;i<N;i++){
        cout<<"J"<<S.M[1][i]<<" ";
    }cout<<endl;
    cout<<"TRAVEL: "<< TravelCosts << "PENALTY: " << PenaltyCosts << "USE: " << UseCosts << "\n\n";

    vector<data> initialConfig;
    unordered_set<int> visiJobs;
    
    unordered_map<int, unordered_set<int>> vehiConfig;
    vector<int> jobOrder;

    for(int i=0; i<N; i++){

        int v = S.M[0][i];
        vehiConfig[ v ].insert(i);

        if(visiJobs.find(v) == visiJobs.end()){

            jobOrder.push_back(v);
            visiJobs.insert(v);

        }
    }
   
    for(int i=0; i<jobOrder.size(); i++){

        int v = jobOrder[i];

        data jobConfig;
        jobConfig.job = false; 
        jobConfig.id = v;

        initialConfig.push_back( jobConfig );

        for(int j=0; j<N; j++){

            if(vehiConfig[ v ].find(S.M[1][j]) != vehiConfig[v].end()){

                data jobConfig2; 
                jobConfig2.job = true; 
                jobConfig2.id = S.M[1][j];

                initialConfig.push_back( jobConfig2 );
            }
        }
    }

    //complete
    for(int i=0;i<K; i++){
        if(visiJobs.find(i) == visiJobs.end()){
            data Go;
            Go.id = i;
            visiJobs.insert(i);
            Go.job = false;
            initialConfig.push_back( Go );
        }
    }
    assert( visiJobs.size() == K );
    
    for(int i=0;i<initialConfig.size();i++){
        if(initialConfig[i].job){
            cout<<"J"<<initialConfig[i].id<<" ";
        }else{
            cout<<"V"<<initialConfig[i].id<<" ";
        }
    } cout << endl;

        vector<vehicleLoaded> vehicleOrder=generateVehicleOrder(initialConfig, K);//Generating info about vehicle transportation

    vehicleOrder=generateVehicleOrder(initialConfig, K);//Generating info about vehicle transportation
    vector<int> startVehicleTime(K,0);//Calculating Delivery time(D) and Starting time(Sk)
    vector<int> deliveryTime = calculatingDeliveryTime(initialConfig,startVehicleTime,t,P,vehicleOrder, N);
    vector<int> jobTardiness = calculatingJobTardiness(deliveryTime, N,d);//Generating Job Tardiness (T) of each job (O(N))
    double finalObj= objFunction(vehicleOrder,initialConfig, K, N,t,w,jobTardiness,F);

    cerr <<"FIRST: " << (TravelCosts + UseCosts + PenaltyCosts) << "SECOND: " << finalObj << "\n\n";
    assert(  fabs ( (TravelCosts + UseCosts + PenaltyCosts) - finalObj) <= 0.0001 );
}else{
    cout<<"---------------_PASSSEI DE BOAS -----------------\n\n";
}