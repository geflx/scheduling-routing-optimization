#include "core.h"
#include "nbhoods.h"

vector<data> generateGreedy(const string &which, const vector<int> &jobSize, int N, int K, const vector<double> &weight,
    const vector<int> &procTime, const vector<int> &dueDate, const vector<int> &carPrices, const vector<int> &carCap) {
    //Calculating processing time average
    double procTimeAvg=0;
    bool success= true; // If we generate a valid configuration

    for(int i=0;i<N;i++){

        procTimeAvg+=procTime[i];

    }
    procTimeAvg /= N;

     /* ------------ debug of parameters readed -------------

    cout<<"Jobs:" <<N<<" Cars: "<<K<<endl;
    cout<<"Sizes (P,d): "<<procTime.size()<<" "<<dueDate.size()<<" Sizes(F,Q): "<<carPrices.size()<<" "<<carCap.size()<<endl;
    cout<<"P: "; for(int i: procTime) cout<<i<<" "; cout<<endl;
    cout<<"d: "; for(int i: dueDate) cout<<i<<" "; cout<<endl;
    cout<<"F: "; for(int i: carPrices) cout<<i<<" "; cout<<endl;
    cout<<"Q: "; for(int i: carCap) cout<<i<<" "; cout<<endl;

     -------------- end of debugging ----------------------*/

    //Variables to generate processing order of jobs:
    vector<data> jobOrder(N);
    vector<bool> visitedJob(N,false);
    int acumulatedD=0;

    // cout<<"numtrbs"<<N<<endl;
    for(int i=0;i<N;i++){

        //Create priority queue of Dispatching Rule
        priority_queue<pair<double, int> > pq;

        for(int j=0;j<N;j++){

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
    vector<data> vehicleDispatching(K);

    priority_queue<pair<int,int> > vehiclePrice; //We will select the cars in the cheapest order.

    for(int i=0; i<K; i++){
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
    vector<vector<int>> adjcarry( K );

    int job = 0;
    for( int i=0; i<K; i++){
        int cap = carCap[ carbyprice[ i ] ]; //getting id and cap

        int accCap = 0;
        while( job < N && (accCap + jobSize[ jobOrder[ job ].id ])<= cap ){
            adjcarry[ i ].push_back( jobOrder[ job ].id);
            accCap += jobSize[ jobOrder[ job ].id];
            job++;

        }
    }

    vector<data> configuration( N + K );
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




void graspV1(int N, int K, const vector<double> &w,const vector<int> &P, const vector<vector<int>> &t,
	const vector<int> &F , const vector<int> &d, const vector<int> &Q, const vector<int> &s, int nbiter){


		double bestObj;
		vector<data> bestConfig;
		int iterBestFound=0;
		for(int i=0; i< nbiter; i++){

		    vector<data> initialConfig = generateValidRandomConfig(N,K,Q,s);

		    vector<vehicleLoaded> vehicleOrder=generateVehicleOrder(initialConfig,K);//Generating info about vehicle transportation
		    vector<int> startVehicleTime(K, 0);//Calculating Delivery time(D) and Starting time(Sk)
		    vector<int> deliveryTime = calculatingDeliveryTime(initialConfig,startVehicleTime,t,P,vehicleOrder,N);
		    vector<int> jobTardiness = calculatingJobTardiness(deliveryTime, N, d);//Generating Job Tardiness (T) of each job (O(N))

		    double resultObjFunction= objFunction(vehicleOrder,initialConfig,K,N,t,w,jobTardiness,F);


		    if( i == 0 ){
		    	bestObj = resultObjFunction;
		    	bestConfig = initialConfig;
		    	// cout<<" Started with: "<<bestObj<<endl;
		    }

		    bool generalImprove = true;
		    while(generalImprove){

		     	bool imp1,imp2,imp3,imp4;

		     	imp1 = nbhood1('F',initialConfig, vehicleOrder, N, K, w, P, t, F, d );
		     	imp2 = nbhood2('F',initialConfig, vehicleOrder, N, K, w, P, t, F, d , Q, s);
		     	imp3 = nbhood3('F',initialConfig, vehicleOrder, N, K, w, P, t, F, d );
		     	imp4 = nbhood4('F',initialConfig, vehicleOrder, N, K, w, P, t, F, d , Q, s);

		     	generalImprove = (imp1 || imp2 || imp3 || imp4);
		    }

		    vehicleOrder=generateVehicleOrder(initialConfig,K);//Generating info about vehicle transportation
		  	deliveryTime = calculatingDeliveryTime(initialConfig,startVehicleTime,t,P,vehicleOrder,N);
		   	jobTardiness = calculatingJobTardiness(deliveryTime, N, d);//Generating Job Tardiness (T) of each job (O(N))


		    double newObj= objFunction(vehicleOrder,initialConfig,K,N,t,w,jobTardiness,F);
		    if( newObj < bestObj){
		    	iterBestFound = i;
		    	bestObj = newObj;
		    	bestConfig = initialConfig;
		    }
		}

        // cout<<"Stop getting better at: "<<iterBestFound<<" iteration"<<endl<<endl;

}

void printConfig( double fObj, const char* message, const vector<data> &config, const vector<int> &carCap, const vector<int> &jobSize,
                  int N, int K, const vector<int> &processTime, const vector<vector<int>> &time, const vector<int> &dueDate,
                  const vector<double> &weight, const vector<int> &carPrices){


    vector<vehicleLoaded> vehicleOrder = generateVehicleOrder( config, K);
    vector<int> startVehicleTime(K, 0);
    vector<int> deliveryTime = calculatingDeliveryTime(config ,startVehicleTime,time,processTime,vehicleOrder,N);
    vector<int> T = calculatingJobTardiness(deliveryTime,N,dueDate);

    if(fObj == 0) //Calculate
        fObj = objFunction(vehicleOrder,config, K, N,time,weight,T,carPrices);

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

vector<int> carryWeight( vector<data> &config, const vector<int> &jobSize, int K){

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

//Soft transformation: swap two jobs between different vehicles
vector<data> mutation( vector<data> &solution, const vector<int> &capacities, const vector<int> &jobSize, int N, int K){

    vector<int> carrying = carryWeight( solution, jobSize, K);

    vector<int> jobPos (N, -1);
    for(int i=0; i<solution.size(); i++)
        if( solution[i].job)
            jobPos[ solution[i].id ] = i;

    vector<int> dad( N, -1 );

    vector<vehicleLoaded> order = generateVehicleOrder( solution, K);

    //Saves the position of vehicle in the vehicleOrder array
    vector<int> posiVehicleOrder (K,-1);

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

        if(randomTries > 2*N){
            // cout<<"Caution - Force break!! Tried N*2 times to swap between cars and get invalid move. \n";
            break;
        }
        int first = rand()%N;
        int sec = rand()%N;

        //Optimize - adding a new data structure
        while( dad[first] == dad[sec]){
            first = rand()%N;
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

vector<data> perturb( vector<data> &solution, const vector<int> &capacities, const vector<int> &jobSize, int N, int K){

    int mode[3] = { 1,2 };
    double intensity[11]= { 0.5, 0.6, 0.7, 0.8, 0.9, 1.0, 1.1, 1.2, 1.3, 1.4, 1.5};

    double sizePerturb = K * intensity[ rand()%11 ];

    //rand()%3
    int whichMode = mode[ rand()%2 ];

    vector<int> carrying = carryWeight( solution, jobSize, K);

    vector<int> jobPos (N, -1);
    for(int i=0; i<solution.size(); i++)
        if( solution[i].job)
            jobPos[ solution[i].id ] = i;

    vector<int> dad( N, -1 );


    vector<vehicleLoaded> order = generateVehicleOrder( solution, K);

    //Saves the position of vehicle in the vehicleOrder array
    vector<int> posiVehicleOrder (K,-1);

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

                if(randomTries > 2*N){
                    // cout<<"Caution - Force break!! Tried N*2 times to swap between cars and get invalid move. \n";
                    break;
                }
                int first = rand()%N;
                int sec = rand()%N;

                //Optimize - adding a new data structure

                int changeDadTries = 0;
                while( dad[first] == dad[sec]){

                    if(changeDadTries>100){ //It means we got stuck because ALL jobs are in just one vehicle
                       return solution;
                    }
                    changeDadTries++;
                    first = rand()%N;
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

         	/* Perturbation mode 2: Insert Jobs in other vehicles */
             for(int i=0; i<sizePerturb; i++){

                 if(randomTries > 2*N){
                     // cout<<"Caution - Force break!! Tried N*2 times to swap between cars and get invalid move. \n";
                     break;
                 }
                 int first = rand()%N;
                 int sec = rand()%N;

                 //Optimize - adding a new data structure
                 int changeDadTries = 0;
                 while( dad[first] == dad[sec]){

                     if(changeDadTries>100){ //It means we got stuck because ALL jobs are in just one vehicle
                        return solution;
                     }

                     first = rand()%N;
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

pair<double, vector<data>> ils_rvnd(int N, int K, const vector<double> &w,const vector<int> &P, const vector<vector<int>> &t,
    const vector<int> &F , const vector<int> &d, const vector<int> &Q, const vector<int> &s, int maxIter, int maxIterIls){


    int nbValidSolutions = 0;
    vector< vector<data> > validSolutions;

    // Generating initial greedy solutions
    vector<data> atcConfig = generateGreedy(atc, s, N, K, w, P, d, F, Q);
    vector<data> weddConfig = generateGreedy(wedd, s, N, K, w, P, d, F, Q);
    vector<data> wmddConfig = generateGreedy(wmdd, s, N, K, w, P, d, F, Q);

    //Danger: Verify ATC!!!
    // Verifying if they are really valid

    // cerr<<" Trying ATC: \n";
    if( validConfig( atcConfig, Q, s, N, K)){
        validSolutions.push_back( atcConfig );
        nbValidSolutions++;
    }
    // cerr<<" Trying WMDD: \n";

    if( validConfig( wmddConfig, Q, s, N, K)){

        validSolutions.push_back( wmddConfig);
        nbValidSolutions++;
    }

    // cerr<<" Trying WEDD: \n";
    if( validConfig( weddConfig, Q, s, N, K)){

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

            callRvnd = RVND( true, N, K, w, P, t, F, d , Q, s, solution);

            contGetSolution++;
        }else{
        //Otherwise we generate a random one
            callRvnd = RVND( false, N, K, w, P, t, F, d , Q, s, solution);
         }

        for(int b=0; b< maxIterIls; b++){

            if( !justCalledRvnd ){
                callRvnd = RVND( true, N, K, w, P, t, F, d , Q, s, solution);
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
             solution = perturb( solution, Q, s, N, K);
        }
    }
 	if( validConfig( bestSolution, Q, s, N, K))
    	return {bestResult,bestSolution};
    else
    	return {-1,bestSolution};
}

pair<double, vector<data>> ils_rvnd_custom(int N, int K, const vector<double> &w,const vector<int> &P, const vector<vector<int>> &t,
    const vector<int> &F , const vector<int> &d, const vector<int> &Q, const vector<int> &s, int maxIter, int maxIterIls){


    int nbValidSolutions = 0;
    vector< vector<data> > validSolutions;

    // Generating initial greedy solutions
    vector<data> atcConfig = generateGreedy(atc, s, N, K, w, P, d, F, Q);
    vector<data> weddConfig = generateGreedy(wedd, s, N, K, w, P, d, F, Q);
    vector<data> wmddConfig = generateGreedy(wmdd, s, N, K, w, P, d, F, Q);

    //Danger: Verify ATC!!!
    // Verifying if they are really valid
    if( validConfig( atcConfig, Q, s, N, K)){
        validSolutions.push_back( atcConfig );
        nbValidSolutions++;
    }
    if( validConfig( wmddConfig, Q, s, N, K)){
        validSolutions.push_back( wmddConfig);
        nbValidSolutions++;
    }
    if( validConfig( weddConfig, Q, s, N, K)){
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

            callRvndCustom = RVND_Custom( true, N, K, w, P, t, F, d , Q, s, solution);

            contGetSolution++;
        }else{
        //Otherwise we generate a random one
            callRvndCustom = RVND_Custom( false, N, K, w, P, t, F, d , Q, s, solution);
         }

        for(int b=0; b< maxIterIls; b++){

            if( !justCalledRvndCustom ){

                callRvndCustom = RVND_Custom( true, N, K, w, P, t, F, d , Q, s, solution);

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
            solution = perturb( solution, Q, s, N, K);

        }
    }

    if( validConfig( bestSolution, Q, s, N, K))
    	return {bestResult,bestSolution};
    else
    	return {-1,bestSolution};
}

//pair< BOOL, solution > indicating if it is valid
pair< bool, vector<data> > crossOver( const vector<data> &f1, const vector<data> &f2,const vector<int> &capacities, const vector<int> &jobSize, int N, int K){

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

    bool isvalid = validConfig( son, capacities, jobSize, N, K );

    return { isvalid, son};
}

// * Ident OK
// GA-LS's local search method.
pair<double, vector<data>> fastLocalSearch(bool reuse, int N, int K, const vector<double> &w,const vector<int> &P, const vector<vector<int>> &t,
   const vector<int> &F , const vector<int> &d, const vector<int> &Q, const vector<int> &s, vector<data> &initialConfig){

    vector<vehicleLoaded> vehicleOrder = generateVehicleOrder(initialConfig, K);//Generating info about vehicle transportation

    nbhood1('B', initialConfig, vehicleOrder, N, K, w, P, t, F, d);
    nbhood2('B', initialConfig, vehicleOrder, N, K, w, P, t, F, d ,Q, s);
    nbhood3('B', initialConfig, vehicleOrder, N, K, w, P, t, F, d);
    nbhood4('B', initialConfig, vehicleOrder, N, K, w, P, t, F, d ,Q, s);
    nbhood5('B', initialConfig, vehicleOrder, N, K, w, P, t, F, d);
    nbhood6('B', initialConfig, vehicleOrder, N, K, w, P, t, F, d);


    // Calculate new OBJ Function.
    vehicleOrder = generateVehicleOrder(initialConfig, K);
    vector<int> startVehicleTime(K, 0);
    vector<int> deliveryTime = calculatingDeliveryTime(initialConfig, startVehicleTime, t, P, vehicleOrder, N);
    vector<int> jobTardiness = calculatingJobTardiness(deliveryTime, N, d);

    double finalObj = objFunction(vehicleOrder, initialConfig, K, N, t, w, jobTardiness, F);

    return {finalObj, initialConfig};
}

pair<double, vector<data>> genAlgo1(int N, int K, const vector<double> &w,const vector<int> &P, const vector<vector<int>> &t,
    const vector<int> &F , const vector<int> &d, const vector<int> &Q, const vector<int> &s, int popSize){


        vector< double > popObj( popSize , -1);
        vector< vector<data> > pop( popSize );

        vector< data > bestConfig;
        double bestObj = INF;

        for(int i=0; i< popSize; i++){
            vector<data> trash; // will be use as a puppet to call function generateRandom
            pair<double, vector<data>> callRvnd = RVND( false, N, K, w, P, t, F, d , Q, s, trash);
            pop[ i ] = callRvnd.second;
            popObj[ i ] = callRvnd.first;

            if( popObj[ i ] < bestObj ){
                bestObj = popObj[ i ];
                bestConfig = pop[ i ];
            }
        }

        int maxIter = 8 * (N + K);
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

                pair<bool,vector<data>> son1 = crossOver( pop[ father1 ], pop[ father2 ], Q, s,  N, K);
                pair<bool,vector<data>> son2 = crossOver( pop[ father1 ], pop[ father2 ], Q, s,  N, K);

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

                    vector<data> afterMutation = mutation( newPop[ newPopSize ], Q, s, N, K);
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
                pair<double, vector<data>> callFastLocalSearch = fastLocalSearch(true,N,K,w,P,t,F,d,Q,s, newPop[ i ]);
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
    int N, K;

    // Reading parameters from file
    string fileName;
    float varMi,sigmaUm,sigmaDois;
    int numInstancia;

    cout<<"Input file name + .ext to open: ";


    getline(cin,fileName);
    ifstream in(fileName);


    while(in>>numInstancia){

        in>>varMi>>sigmaDois;
        in>>N;
        in>>K;
        char instancia[50];
        sprintf(instancia, "%d_%d_%d_%.1lf_%.1lf", numInstancia, N, K, varMi, sigmaDois);
        cout<<instancia<<endl;
        vector<int> F(K); //Car's cost
        vector<int> Q(K); //Car's capacity

        // Reading parameters
        vector<vector<int> > t(N+1,vector<int>(N+1,0)); //Time travels
        vector<int> P(N); //Processing time
        vector<double> w(N); //Penalty weight
        vector<int> d(N); //Due date
        vector<int> s(N); //Job size

        for(int i=0;i<N;i++)
            in>>P[i];
        for(int i=0;i<N;i++)
            in>>d[i];
        for(int i=0;i<N;i++)
            in>>s[i];
        for(int i=0;i<N;i++)
            in>>w[i];
        for(int i=0;i<K;i++)
            in>>Q[i];
        for(int i=0;i<K;i++)
            in>>F[i];
        for(int i=0;i<=N;i++){
            for(int j=0;j<=N;j++){
                in>>t[i][j];
            }
        }

        //Calling for ILS Rvnd
        time_t time2;
        time(&time2);
        pair<double,vector<data>> ils2 = ils_rvnd( N, K, w, P, t, F, d, Q, s, 5, 10);

        time_t time2end;
        time(&time2end);
        double diff2 = difftime(time2end,time2);
        printConfig( ils2.first, "ils_rvnd", ils2.second, Q, s, N, K, P, t, d, w, F);


        //Calling for ILS Rvnd Custom ( Intra Route and Inter Route, from article)
        time_t time1;
        time(&time1);
        pair<double,vector<data>> ils = ils_rvnd_custom( N, K, w, P, t, F, d, Q, s, 30, 100);
        time_t time1end;
        time(&time1end);
        double diff1 = difftime(time1end,time1);
        printConfig( ils.first, "ils_rvnd_custom", ils.second, Q, s, N, K, P, t, d, w, F);

        //Calling for Genetic Algorithm Version 1
        time_t time3;
        time(&time3);
        pair<double,vector<data>> ga1 = genAlgo1(N, K, w, P, t, F, d, Q, s, 50);
        time_t time3end;
        time(&time3end);
        double diff3 = difftime(time3end,time3);
        printConfig( ga1.first, "gen_algo_1 ", ga1.second, Q, s, N, K, P, t, d, w, F);

    }

    in.close();

}
