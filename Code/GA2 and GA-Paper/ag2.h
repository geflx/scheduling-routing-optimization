#ifndef AG2_H
#define AG2_H

using namespace std;

long long int genPop;

// Complexity: O(max(N, K))
pair<vector<int>, int> getOverloadStatus(Solution &S, int N, int K, const vector<int> &s, const vector<int> &Q){
    
    vector<int> accSize (K, 0); // * Acc job size per vehicle

    for(int i=0; i<N; i++)
        accSize[ S.M[0][i] ] += s[i];

    int countOver = 0;

    for(int i=0; i<K; i++)
        if(accSize[i] > Q[i])
            countOver++;
    
    return {accSize, countOver};
}

// Complexity: O(N*K) , N and K being maximum
void makeFeasible(Solution &S, int N, int K, const vector<int> &P, const vector<int> &d, const vector<int> &s,
            const vector<double> &w, const vector<int> &Q, const vector<int> &F,
            const vector<vector<int>> &t)
{
    pair<vector<int>, int> status = getOverloadStatus(S, N, K, s, Q); // * First: job size sum (size: K), Second: total overload

    vector<vector<int>> m( K, vector<int>()); // Matrix with K vectors indicating carried jobs (disabled pos == -1)
    
    for(int i=0; i<N; i++) // Assign job to vehicle
        m[ S.M[0][i] ].push_back( i );
     
    for(int i=0; i<K; i++){
        if( status.first[i] <= Q[i] ) // If Vehicle is OK, ignore it
            continue;

        for(int j=0; j<m[i].size(); j++){ //For each job

            if(status.first[i] <= Q[i]) // Check if vehicle is OK now
                break;

            if(m[i][j] == -1) // ! Ignore disabled job positions 
                continue;

            for(int k=0; k<K; k++){
 
                if(i == k)  // ! Same vehicle? continue ...
                    continue;

                if( (status.first[k] + s[ m[i][j] ]) <= Q[k] ){ // * If job fits in vehicle k:
                    
                    status.first[i] -= s[ m[i][j] ];
                    status.first[k] += s[ m[i][j] ];

                    m[k].push_back( m[i][j] );
                    m[i][j] = -1;

                    break;
                }
            }
        }
    }

    for(int i=0; i<K; i++)
        for(int j=0; j<m[i].size(); j++)
            if(m[i][j] != -1)
                S.M[0][ m[i][j] ] = i; // Assign job m[i][j] to vehicle i
        
    S.Value = calculateObj(S, N, K, P, d, s, w, Q, F, t);

}

void greedySolution(Solution &S, const string &ruleParam, const bool random, int N, int K, const vector<int> &P, const vector<int> &d, const vector<int> &s,
                   const vector<double> &w, const vector<int> &Q, const vector<int> &F,
                   const vector<vector<int>> &t)
{
    //Variables to generate processing order of jobs:
    vector<int> jobOrder(N);
    vector<bool> visiJob(N, false);

    string rule = ruleParam;

    int accD = 0;
    double PAvg = 0.0;

    for(int i=0; i<N; i++) 
        PAvg += P[i];
    PAvg /= N;

    if(random){ // pick a random rule
        vector<string> rules = {"atc", "wmdd", "wedd"};
        rule = rules[ rand()%3 ];
    }

    //Generate Rule's Job Order
    for(int i=0; i<N; i++){

        priority_queue<pair<double, int>> pq;

        for(int j=0; j<N; j++){

            if(visiJob[j]) 
                continue;

            double ruleValue;

            if(rule == "atc"){

                ruleValue = (w[j] / P[j]) * exp( (-1) * (max(d[j]- P[j]- accD , 0)) / PAvg);
                pq.push( {ruleValue, j} );

            }else if(rule == "wmdd"){

                ruleValue=  (1 / w[j]) * max(P[j], d[j] - accD) ;
                pq.push( {-1 * ruleValue, j} );

            }else if(rule == "wedd"){

                ruleValue = d[j] / w[j];
                pq.push( {-1 * ruleValue,j} );
                
            }
        }

        jobOrder[i] = pq.top().second;
        visiJob[ jobOrder[i] ] = true;

        accD += P[ jobOrder[i] ];
    }

    for(int i=0; i<N; i++)
        S.M[1][i] = jobOrder[i];

    bool infeasible = false;

    unordered_map<int, int> carStatus; //First: ID, Second: Free Space
    int itV = 0, itJ = 0;

    if(!random){

        vector<pair<int, int>> sortedV(K); //First: Q, Second: ID
        for(int i=0; i<K; i++){
            sortedV[i] = {Q[i], i};
        }
        sort(sortedV.begin(), sortedV.end());

        carStatus[ sortedV[0].second ] = Q[ sortedV[0].second ];

        while(itJ < N){
            // * Job ID location:  jobOrder[itJ]
            // * Vehicle ID location: sortedV[itV].second 
            bool inserted = false;

            while(!inserted && itV < K){ // If there is available vehicles, use them

                if( (carStatus[ sortedV[itV].second ] - s[ jobOrder[itJ] ]) < 0 ){ // dont Fit

                    itV++;
                    if(itV < K)
                        carStatus[ sortedV[itV].second ] = Q[ sortedV[itV].second ];

                }else{
                    
                    S.M[0][ jobOrder[itJ] ] = sortedV[itV].second;
                    carStatus[ sortedV[itV].second ] -= s[ jobOrder[itJ] ];

                    inserted = true;
                    itJ++;

                }
            }
            
            if(itV == K && !inserted){ //  All vehicles in use, find the most free to minimize Overlap
                            
                pair<int, int> bestFit; //First: Best Free Space, Second: vehicle ID
                bestFit.first = -987654321;
                bestFit.second = -1;

                for(int i=0; i<K; i++){
                    if(carStatus[i] > bestFit.first){  // Refreshing most free..
                        bestFit.first = carStatus[i];
                        bestFit.second = i;
                    }
                }

                S.M[0][ jobOrder[itJ] ] = bestFit.second; // Put job into vehicle
                carStatus[ bestFit.second ] -= s[ jobOrder[itJ] ];

                if(carStatus[ bestFit.second ] < 0)
                    infeasible = true; // ! Reminder that this solution is infeasible: "minimizing overlap"
                
                inserted = true;
                itJ++; 

            }
        }

    }else{
        // * Job ID location:  jobOrder[itJ]
        // * Vehicle ID location: shuffledV[itV] 

        vector<int> shuffledV(K); // Random shuffled vehicles
        for(int i=0; i<K; i++)
            shuffledV[i] = i;
        random_shuffle(shuffledV.begin(), shuffledV.end());
    
        double rangeConst;
        if( N <= 20)
            rangeConst = 0.2;
        else
            rangeConst = 0.1;

        carStatus[ shuffledV[0] ] = Q[ shuffledV[0] ];
        
        while(jobOrder.size() != 0){ // While there are still jobs unassigned ...

            itJ = rand() % min( (int)jobOrder.size(), (int)ceil(rangeConst * N)); // * select job from set 0 ... min(size, ceil(rangeConst * N)) 
            bool inserted = false;

            while(!inserted && itV < K){ // While there are still vehicles available, use them ...

                if( (carStatus[ shuffledV[itV] ] - s[ jobOrder[itJ] ]) < 0 ){ // Job dont fit in this vehicle, go to next one

                    itV++;
                    if(itV < K)
                        carStatus[ shuffledV[itV] ] = Q[ shuffledV[itV] ];

                }else{ // Job fits in vehicle ...
                    
                    S.M[0][ jobOrder[itJ] ] = shuffledV[itV];
                    carStatus[ shuffledV[itV] ] -= s[ jobOrder[itJ] ];

                    inserted = true;
                }
            }
            
            if(itV == K && !inserted){ //  If all vehicles are in use ...
        
                pair<int, int> bestFit; //First: Best Free Space, Second: vehicle ID
                bestFit.first = -987654321;
                bestFit.second = -1;

                for(int i=0; i<K; i++){
                    if(carStatus[i] > bestFit.first){ // Find available most free vehicle ...
                        bestFit.first = carStatus[i];
                        bestFit.second = i;
                    }
                }

                S.M[0][ jobOrder[itJ] ] = bestFit.second; // Insert job into most free vehicle 
                carStatus[ bestFit.second ] -= s[ jobOrder[itJ] ];

                if(carStatus[ bestFit.second ] < 0)
                    infeasible = true; // ! Reminder that this solution is infeasible: "minimizing overlap"
                
                inserted = true;
            }

            if(inserted){
                for(int i = itJ; i < jobOrder.size()-1; i++) //Remove currJob
                    jobOrder[i] = jobOrder[i+1]; 
                
                jobOrder.pop_back();
            }
        }
    }  
    
    // assert( isSolution(S, N, K) == true);

    if( !isFeasible(S, N, K, s, Q)){
        makeFeasible(S, N, K, P, d, s, w, Q, F, t);
    }


    S.Value =  calculateObj(S, N, K, P, d, s, w, Q, F, t);
}

// Complexity: O(N) due to Solution Evaluation
Solution CrossOver(const Solution &S1, const Solution &S2, int N, int K, const vector<int> &P, const vector<int> &d, const vector<int> &s,
                   const vector<double> &w, const vector<int> &Q, const vector<int> &F,
                   const vector<vector<int>> &t)
{
    Solution S;
    S = S1;

    int point = rand() % N;
    
    unordered_set<int> MixJobs;
    for(int i=point; i<N; i++) 
        MixJobs.insert( S.M[1][i] );
    
    int cont = point;

    for(int i=0; i<N; i++){
        
        if(cont == N)
            break;

        if( CROSSOVER_OPTION == 0){
            // Change Job and Vehicle  
            if( MixJobs.find(S2.M[1][i]) != MixJobs.end() ){

                S.M[0][cont] = S2.M[0][i];
                S.M[1][cont] = S2.M[1][i];

                ++cont;
            }

        }else if( CROSSOVER_OPTION == 1){
            // Change just Job, keeping the same Vehicle
            if( MixJobs.find(S2.M[1][i]) != MixJobs.end() ){

                S.M[1][cont] = S2.M[1][i];
                ++cont;
            }
        }
    }
    
    S.Value = calculateObj(S, N, K, P, d, s, w, Q, F, t);

    return S;

}

// Complexity: O(N) due to Solution Evaluation
Solution Mutation( Solution &S, int N, int K, const vector<int> &P, const vector<int> &d, const vector<int> &s,
                   const vector<double> &w, const vector<int> &Q, const vector<int> &F,
                   const vector<vector<int>> &t)
{
    int A, B;
    A = B = rand() % N;
    
    while(A == B){
        B = rand() % N;
    }
    if( A > B ) 
        swap(A, B);

    pair<int, int> TempA = { S.M[0][A], S.M[1][A] };

    // Mutate Job
    S.M[1][A] = S.M[1][B];
    S.M[1][B] = TempA.second;

    if( MUTATION_OPTION == 0){
        // Also mutate Vehicle
        S.M[0][A] = S.M[0][B];
        S.M[0][B] = TempA.first;

    }

    S.Value = calculateObj(S, N, K, P, d, s, w, Q, F, t);

    return S;
    
}

Solution GA_Tam_1 (int N, int K, int itNumber, int popSize,
              const vector<int> &P, const vector<int> &d, const vector<int> &s,
              const vector<double> &w, const vector<int> &Q, const vector<int> &F,
              const vector<vector<int>> &t)
{
    Solution S_best;
    S_best.alocate(N);
    S_best.Value = numeric_limits<double>::infinity();

    vector<Solution> PP(popSize);

    for(Solution &S: PP){
        S.alocate(N);
        randomSolution(S, N, K);
        S.Value = calculateObj(S, N, K, P, d, s, w, Q, F, t);

        if(S_best.Value > S.Value){
            S_best = S;
        }
    }
    
    int cont = 0;

    while(cont < itNumber){

        ++cont;

        int idP1, idP2;
        idP1 = idP2 = rand() % popSize;

        while(idP2 == idP1)
            idP2 = rand() % popSize;
        
        Solution OFF1, OFF2;
        OFF1 = CrossOver(PP[idP1], PP[idP2], N, K, P, d, s, w, Q, F, t); // ! CO(P1, P2)
        OFF2 = CrossOver(PP[idP2], PP[idP1], N, K, P, d, s, w, Q, F, t); // ! CO(P2, P1)

        Solution Local_S_best;
        if(OFF1.Value < OFF2.Value)
            Local_S_best = OFF1;
        else
            Local_S_best = OFF2;

        OFF1 = Mutation(OFF1, N, K, P, d, s, w, Q, F, t);
        OFF2 = Mutation(OFF2, N, K, P, d, s, w, Q, F, t);

        if(OFF1.Value <= OFF2.Value){
            if(OFF1.Value < Local_S_best.Value)
                Local_S_best = OFF1;
        }else{
            if(OFF2.Value < Local_S_best.Value)
                Local_S_best = OFF2;
        }
        // Parents will be replaced with Offsprings
        PP[idP1] = OFF1;
        PP[idP2] = OFF2;

        if(Local_S_best.Value < S_best.Value){
            S_best = Local_S_best;
            cont = 0;
        }
    }
    return S_best;
}

Solution GA_Tam_2 (int N, int K, int itNumber, int popSize,
              const vector<int> &P, const vector<int> &d, const vector<int> &s,
              const vector<double> &w, const vector<int> &Q, const vector<int> &F,
              const vector<vector<int>> &t)
{
    Solution S_best;
    S_best.alocate(N);
    S_best.Value = numeric_limits<double>::infinity();

    vector<Solution> PP(popSize);

    for(Solution &S: PP){
        S.alocate(N);
        randomSolution(S, N, K);
        S.Value = calculateObj(S, N, K, P, d, s, w, Q, F, t);

        if(S_best.Value > S.Value){
            S_best = S;
        }
    }
    
    int cont = 0;

    while(cont < itNumber){

        ++cont;
        
        vector<Solution> P_New;

        Solution Local_S_best;
        Local_S_best.Value = numeric_limits<double>::infinity();

        for(int i=0; i< popSize; i+= 2){ // ! i+=2 : adding two new solutions

            int idP1, idP2;
            idP1 = idP2 = rand() % popSize;

            while(idP2 == idP1)
                idP2 = rand() % popSize;
                        
            Solution OFF1, OFF2;
            OFF1 = CrossOver(PP[idP1], PP[idP2], N, K, P, d, s, w, Q, F, t); // ! CO(P1, P2)
            OFF2 = CrossOver(PP[idP2], PP[idP1], N, K, P, d, s, w, Q, F, t); // ! CO(P2, P1)

            OFF1 = Mutation(OFF1, N, K, P, d, s, w, Q, F, t);
            OFF2 = Mutation(OFF2, N, K, P, d, s, w, Q, F, t);

            P_New.push_back(OFF1);
            P_New.push_back(OFF2);

            if(OFF1.Value <= OFF2.Value){
                if(OFF1.Value < Local_S_best.Value)
                    Local_S_best = OFF1;
            }else{
                if(OFF2.Value < Local_S_best.Value)
                    Local_S_best = OFF2;
            }
        }

        if(Local_S_best.Value < S_best.Value){

            S_best = Local_S_best;
            cont = 0;
        
        }
        // Update Population
        PP = P_New; 
        
        int randomSol = rand() % PP.size();
        PP[ randomSol ] = S_best;              
    }
    return S_best;
}

// Complexity: O( itNumber * ( 2*popSize + O(N*K) ) )
Solution GA2_Repr1 (int N, int K, int itNumber, int popSize, double mutateProb,
    const vector<int> &P, const vector<int> &d, const vector<int> &s,
    const vector<double> &w, const vector<int> &Q, const vector<int> &F,
    const vector<vector<int>> &t)
{

    Solution S_best;
    S_best.alocate(N);
    S_best.Value = numeric_limits<double>::infinity();

    vector<Solution> PP(popSize);

    int contS = 0;

    for(Solution &S: PP){

        S.alocate(N);
        
        if(contS == 0){

           greedySolution(S, "atc", false, N, K, P, d, s, w, Q, F, t);

        }else if(contS == 1){

           greedySolution(S, "wmdd", false, N, K, P, d, s, w, Q, F, t);

        }else if(contS == 2){

            greedySolution(S, "wedd", false, N, K, P, d, s, w, Q, F, t);

        }else if(contS <= 0.7 * popSize){

           greedySolution(S, " ", true, N, K, P, d, s, w, Q, F, t);
           
        }else{
            randomSolution(S, N, K);
            S.Value = calculateObj(S, N, K, P, d, s, w, Q, F, t);
        }
        
        contS++;

        if( !isFeasible(S, N, K, s, Q) )
            makeFeasible(S, N, K, P, d, s, w, Q, F, t);

        if(S_best.Value > S.Value)
            S_best = S;
    }
    
    int cont = 0;

    while(cont < itNumber){

        genPop++;
        ++cont;

        vector<Solution> P_New;

        Solution Local_S_best;
        Local_S_best.Value = numeric_limits<double>::infinity();

        for(int i=0; i<popSize; i+=2){ // ! i+=2

            int idP1, idP2;
            idP1 = idP2 = rand() % popSize;

            while(idP2 == idP1)
                idP2 = rand() % popSize;
            
            Solution OFF1, OFF2;
            OFF1 = CrossOver(PP[idP1], PP[idP2], N, K, P, d, s, w, Q, F, t); // ! CO(P1, P2)
            OFF2 = CrossOver(PP[idP2], PP[idP1], N, K, P, d, s, w, Q, F, t); // ! CO(P2, P1)

            double R = ( rand()%101 ) / 100.0;

            if( R <= mutateProb ){

                OFF1 = Mutation(OFF1, N, K, P, d, s, w, Q, F, t);
                OFF2 = Mutation(OFF2, N, K, P, d, s, w, Q, F, t);

            }
            bool feasibleOFF1 = isFeasible(OFF1, N, K, s, Q);
            bool feasibleOFF2 = isFeasible(OFF2, N, K, s, Q);

            if( !feasibleOFF1 || !feasibleOFF2 ){

                int B = rand()%2; // * 0 or 1

                // * if(B == 0) penalize obj functions: Ok.

                if(B == 1){

                    if(!feasibleOFF1)
                        makeFeasible(OFF1, N, K, P, d, s, w, Q, F, t);
                    
                    if(!feasibleOFF2)
                        makeFeasible(OFF2, N, K, P, d, s, w, Q, F, t);
                }
            }
            P_New.push_back( OFF1 );
            P_New.push_back( OFF2 );

            if(OFF1.Value < OFF2.Value)
                if(OFF1.Value < Local_S_best.Value)
                    Local_S_best = OFF1;
            else
                if(OFF2.Value < Local_S_best.Value)
                    Local_S_best = OFF2;
            
        }
        if(Local_S_best.Value < S_best.Value){

            S_best = Local_S_best;
            // cont = 0; // ! Fixed iterations...
    
        }

        // ! Adding size(P) solutions to P' and then sorting it
        for(int i=0; i<PP.size(); i++)
            P_New.push_back( PP[i] );

        sort(P_New.begin(), P_New.end(), compareSolution); // ! Increasing results:  0 -> 0.001 -> 0.002 ...

        PP.clear();

        for(int i=0; i<popSize; i++){
            PP.push_back( P_New[i] );
        }
    }

    return S_best;
}

// Implementing ...
Solution GA2_Rep1_BL (int N, int K, int itNumber, int popSize, double mutateProb,
              const vector<int> &P, const vector<int> &d, const vector<int> &s,
              const vector<double> &w, const vector<int> &Q, const vector<int> &F,
              const vector<vector<int>> &t)
{
    Solution S_best;
    S_best.alocate(N);
    S_best.Value = numeric_limits<double>::infinity();

    vector<Solution> PP(popSize);

    int contS = 0;

    for(Solution &S: PP){

        S.alocate(N);
        
        if(contS == 0){

           greedySolution(S, "atc", false, N, K, P, d, s, w, Q, F, t);

        }else if(contS == 1){

           greedySolution(S, "wmdd", false, N, K, P, d, s, w, Q, F, t);

        }else if(contS == 2){

            greedySolution(S, "wedd", false, N, K, P, d, s, w, Q, F, t);

        }else if(contS <= 0.7 * popSize){

           greedySolution(S, " ", true, N, K, P, d, s, w, Q, F, t);
           
        }else{
            randomSolution(S, N, K);
            S.Value = calculateObj(S, N, K, P, d, s, w, Q, F, t);
        }
        
        contS++;

        if( !isFeasible(S, N, K, s, Q) ){

            makeFeasible(S, N, K, P, d, s, w, Q, F, t);

            if( !isFeasible(S, N, K, s, Q) ){
                
                do{

                    randomSolution(S, N, K);

                }while(!isFeasible(S, N, K, s, Q));
               
            }
        }

        //Call Fast local search.
        S = fastLS(S, N, K, P, d, s, w, Q, F, t);

        if(S_best.Value > S.Value)
            S_best = S;
    }
    
    int cont = 0;

    while(cont < itNumber){

        genPop++;
        ++cont;

        vector<Solution> P_New;

        Solution Local_S_best;
        Local_S_best.Value = numeric_limits<double>::infinity();

        for(int i=0; i<popSize; i+=2){ // ! i+=2

            int idP1, idP2;
            idP1 = idP2 = rand() % popSize;

            while(idP2 == idP1)
                idP2 = rand() % popSize;
            
            Solution OFF1, OFF2;
            OFF1 = CrossOver(PP[idP1], PP[idP2], N, K, P, d, s, w, Q, F, t); // ! CO(P1, P2)
            OFF2 = CrossOver(PP[idP2], PP[idP1], N, K, P, d, s, w, Q, F, t); // ! CO(P2, P1)

            double R = ( rand()%101 ) / 100.0;

            if( R <= mutateProb ){

                OFF1 = Mutation(OFF1, N, K, P, d, s, w, Q, F, t);
                OFF2 = Mutation(OFF2, N, K, P, d, s, w, Q, F, t);

            }
            bool feasibleOFF1 = isFeasible(OFF1, N, K, s, Q);
            bool feasibleOFF2 = isFeasible(OFF2, N, K, s, Q);

            if( !feasibleOFF1 || !feasibleOFF2 ){

                // Obligate offsprings to be feasible.
                if(!feasibleOFF1)
                    makeFeasible(OFF1, N, K, P, d, s, w, Q, F, t);
                
                if(!feasibleOFF2)
                    makeFeasible(OFF2, N, K, P, d, s, w, Q, F, t);

                while(!isFeasible(OFF1, N, K, s, Q)){

                    randomSolution(OFF1, N, K);
                }

                while(!isFeasible(OFF2, N, K, s, Q)){

                    randomSolution(OFF2, N, K);
                }
                
            }
            P_New.push_back( OFF1 );
            P_New.push_back( OFF2 );

            if(OFF1.Value < OFF2.Value)
                if(OFF1.Value < Local_S_best.Value)
                    Local_S_best = OFF1;
            else
                if(OFF2.Value < Local_S_best.Value)
                    Local_S_best = OFF2;
            
        }
        if(Local_S_best.Value < S_best.Value){

            S_best = Local_S_best;
            // cont = 0; // ! Fixed iterations...
            
        }
        
        // ! Apply LS in Psize/2 best solutions.
        sort(P_New.begin(), P_New.end(), compareSolution); 
        
        // Selecting which new solutions will be in Fast Local Search method.
        fastLS(P_New[0], N, K, P, d, s, w, Q, F, t);

        for(int i=1; i< ceil(popSize/2); i++){
            
            // Checking if P[i].Value == P[i-1].Value.
            if( fabs(P_New[i].Value - P_New[i-1].Value) < 0.0001 ){

                Solution tmp = Mutation(P_New[i], N, K, P, d, s, w, Q, F, t);

                // If is feasible, accept solution.
                if(isFeasible(tmp, N, K, s, Q)){
                	P_New[i] = fastLS(tmp, N, K, P, d, s, w, Q, F, t);
                }


            }else{

            	P_New[i] = fastLS(P_New[i], N, K, P, d, s, w, Q, F, t);

            }

        }
        

        // ! Adding to P': Psize solutions from P and then sorting it.
        for(int i=0; i<PP.size(); i++)
            P_New.push_back( PP[i] );

        sort(P_New.begin(), P_New.end(), compareSolution); // ! Increasing results:  0 -> 0.001 -> 0.002 ...

        PP.clear();

        for(int i=0; i<popSize; i++){
            PP.push_back( P_New[i] );
        }
    }

    return S_best;
}

#endif