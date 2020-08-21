#ifndef HEURISTICS_H
#define HEURISTICS_H

// Reviews: 2
vector<data> GreedySolution(bool constructiveGreedy, int ruleID, const vector<int>& S, int N, int K, const vector<double>& w,
    const vector<int>& P, const vector<int>& d, const vector<int>& F, const vector<int>& Q)
{
    // Processing time average.
    double averageP = 0.0;
    for (int i = 0; i < N; i++) 
        averageP += P[i];
    averageP /= N;

    // Greedy rule variables.
    vector<int> jobOrder(N);
    vector<bool> visitedJob(N, false);
    int acumulatedD = 0;

    if(constructiveGreedy)
        ruleID = rand() % 3;
    

    for (int i = 0; i < N; i++) {

        // Rule PQueue.
        priority_queue<pair<double, int>> PQ;

        for (int j = 0; j < N; j++) {

            if (visitedJob[j]) continue;
            double ruleValue = 0.0;

            if (ruleID == 0) { 
                // 0: ATC Rule.
                ruleValue = (w[j] / P[j]) * exp((-1) * (max(d[j] - P[j] - acumulatedD, 0)) / averageP);
                PQ.push(make_pair(ruleValue, j));
            }
            else if (ruleID == 1) { 
                // 1: WMDD Rule.
                ruleValue = (1 / w[j]) * max(P[j], d[j] - acumulatedD);
                PQ.push(make_pair((-1) * ruleValue, j));
            }
            else if (ruleID == 2) {
                // 2: WEDD Rule.
                ruleValue = (d[j] / w[j]);
                PQ.push(make_pair((-1) * ruleValue, j));
            }
        }

        // Insert best job and sum its P time.
        jobOrder[i] = (PQ.top()).second;
        visitedJob[jobOrder[i]] = true;
        acumulatedD += P[jobOrder[i]];
    }

    vector<data> answer(N + K);
    
    /* Constructive Greedy method:
            1 - Select Dispatching Rule at random.
            2 - Random shuffle vehicles.
            3 - Insert jobs from ruleArray of positions rand() % min(ruleArray.size(), 0.2 * N) 
    */
    if(constructiveGreedy){

        // Create random vehicle list.
        vector<int> vList(K);
        for(int i = 0; i < K; i++) 
            vList[i] = i;
        random_shuffle(vList.begin(), vList.end());

        // Special case: Selected job couldn't be inserted, then save it. <JobId, JobPosition>.
        pair<int, int> jobCache = make_pair(-1, -1);

        int contJob = 0, contVehi = 0, contAnswer = 0;

        while(contVehi < K){

            int vCap = Q[vList[contVehi]];

            do{
                answer[contAnswer].job = false;
                answer[contAnswer].id = vList[contVehi];
                contAnswer++;
                contVehi++;

            }while(contJob == N && contVehi < K);

            // While vehicle isn't full, fulfill it with jobs.
            while(contJob < N){
                int randomJobPos = rand()% min((int)jobOrder.size(), (int)ceil(0.2 * N));
                int randomJob = jobOrder[randomJobPos];

                // Pending job to insert.
                if(jobCache.first != -1){
                    randomJob = jobCache.first;
                    randomJobPos = jobCache.second;
                }

                if((vCap - S[randomJob]) >= 0){
                    // Insert job in solution.
                    vCap -= S[randomJob];
                    jobCache = make_pair(-1, -1);

                    answer[contAnswer].job = true;
                    answer[contAnswer].id = randomJob;
                    contAnswer++; 
                    contJob++;

                    // Removing selected job from jobOrder array.
                    for(int i = randomJobPos; i < jobOrder.size() - 1; i++)
                        jobOrder[i] = jobOrder[i+1];
                    jobOrder.pop_back();

                }else{
                    // Vehicle can't handle selected job.
                    jobCache = make_pair(randomJob, randomJobPos);
                    break;
                }
            }
        }
    }
    else{
        // Selecting cheapest vehicles first and then populating it.
        vector<pair<int, int>> vList(K);
        for(int i = 0; i < K; i++)
            vList[i] = make_pair(F[i], i);
        sort(vList.begin(), vList.end());


        // Adj. list of vehicles x jobs.
        vector<vector<int>> adjList(K);

        int cont = 0;
        for (int i = 0; i < K; i++) {
            int accCap = 0;
            while (cont < N && (accCap + S[jobOrder[cont]]) <= Q[vList[i].second]) {
                adjList[i].push_back(jobOrder[cont]);
                accCap += S[jobOrder[cont]];
                cont++;
            }
        }

        int insert = 0;
        for (int i = 0; i < adjList.size(); i++) {
            // Insert vehicle.
            answer[insert].job = false;
            answer[insert++].id = vList[i].second;

            // Insert its jobs.
            for (int j = 0; j < adjList[i].size(); j++) {
                answer[insert].job = true;
                answer[insert++].id = adjList[i][j];
            }
        }
    }

    return answer;
}
//Soft transformation: swap two jobs between different vehicles
vector<data> mutation(vector<data>& solution, const vector<int>& capacities, const vector<int>& jobSize, int N, int K)
{
    vector<int> carrying = getWeightByVehicle(solution, jobSize, K);

    vector<int> jobPos(N, -1);
    for (int i = 0; i < solution.size(); i++)
        if (solution[i].job)
            jobPos[solution[i].id] = i;

    vector<int> dad(N, -1);

    vector<vehicleLoaded> order = getVOrder(solution, K);

    //Saves the position of vehicle in the vehicleOrder array
    vector<int> posiVehicleOrder(K, -1);

    //Getting the car id of each job carried
    for (int i = 0; i < order.size(); i++) {

        posiVehicleOrder[order[i].id] = i;

        for (int j = order[i].initialPos; j <= order[i].finalPos; j++) {

            if (solution[j].job) {
                int whichJob = solution[j].id;
                dad[whichJob] = order[i].id;
            }
        }
    }

    //Variable to not get stuck successivily while trying to move 2 jobs between cars
    int randomTries = 0;

    while (true) {

        if (randomTries > 2 * N) {
            // cout<<"Caution - Force break!! Tried N*2 times to swap between cars and get invalid move. \n";
            break;
        }
        int first = rand() % N;
        int sec = rand() % N;

        //Optimize - adding a new data structure
        while (dad[first] == dad[sec]) {
            first = rand() % N;
        }
        int posiFirst = jobPos[first];
        int posiSec = jobPos[sec];

        int carFirst = dad[first];
        int carSec = dad[sec];

        if (capacities[carFirst] >= (carrying[carFirst] - jobSize[first] + jobSize[sec])
            && (capacities[carSec] >= (carrying[carSec] - jobSize[sec] + jobSize[first]))) {

            //Updating car's carried weight
            carrying[carFirst] = carrying[carFirst] - jobSize[first] + jobSize[sec];
            carrying[carSec] = carrying[carSec] - jobSize[sec] + jobSize[first];

            //Updating carrying vehicles' id
            swap(dad[first], dad[sec]);

            //Swapping and updating
            swap(solution[posiFirst], solution[posiSec]);
            swap(jobPos[first], jobPos[sec]);

            randomTries = 0;
            break;
        }
        else {
            //Swap again
            randomTries++;
        }
    }
    return solution;
}

vector<data> perturb(vector<data>& solution, const vector<int>& capacities, const vector<int>& jobSize, int N, int K, int sizePerturb)
{

    int mode[3] = { 1, 2 };
    double intensity[11] = { 0.5, 0.6, 0.7, 0.8, 0.9, 1.0, 1.1, 1.2, 1.3, 1.4, 1.5 };


    //rand()%3
    int whichMode = mode[rand() % 2];

    vector<int> carrying = getWeightByVehicle(solution, jobSize, K);

    vector<int> jobPos(N, -1);
    for (int i = 0; i < solution.size(); i++)
        if (solution[i].job)
            jobPos[solution[i].id] = i;

    vector<int> dad(N, -1);

    vector<vehicleLoaded> order = getVOrder(solution, K);

    //Saves the position of vehicle in the vehicleOrder array
    vector<int> posiVehicleOrder(K, -1);

    //Getting the car id of each job carried
    for (int i = 0; i < order.size(); i++) {

        posiVehicleOrder[order[i].id] = i;

        for (int j = order[i].initialPos; j <= order[i].finalPos; j++) {

            if (solution[j].job) {
                int whichJob = solution[j].id;
                dad[whichJob] = order[i].id;
            }
        }
    }

    //Variable to not get stuck successivily while trying to move 2 jobs between cars
    int randomTries = 0;

    switch (whichMode) {

    case 1:
        /*Perturbation Swap(1,1) Permutation between a customer k from a route r1 and a customer l, from a route r2
                    Only valid moves are allowed */

        for (int i = 0; i < sizePerturb; i++) {

            if (randomTries > 2 * N) {
                // cout<<"Caution - Force break!! Tried N*2 times to swap between cars and get invalid move. \n";
                break;
            }
            int first = rand() % N;
            int sec = rand() % N;

            //Optimize - adding a new data structure

            int changeDadTries = 0;
            while (dad[first] == dad[sec]) {

                if (changeDadTries > 100) { //It means we got stuck because ALL jobs are in just one vehicle
                    return solution;
                }
                changeDadTries++;
                first = rand() % N;
            }
            int posiFirst = jobPos[first];
            int posiSec = jobPos[sec];

            int carFirst = dad[first];
            int carSec = dad[sec];

            if (capacities[carFirst] >= (carrying[carFirst] - jobSize[first] + jobSize[sec])
                && (capacities[carSec] >= (carrying[carSec] - jobSize[sec] + jobSize[first]))) {

                //Updating car's carried weight
                carrying[carFirst] = carrying[carFirst] - jobSize[first] + jobSize[sec];
                carrying[carSec] = carrying[carSec] - jobSize[sec] + jobSize[first];

                //Updating carrying vehicles' id
                swap(dad[first], dad[sec]);

                //Swapping and updating
                swap(solution[posiFirst], solution[posiSec]);
                swap(jobPos[first], jobPos[sec]);

                randomTries = 0;
            }
            else {
                i--; //Swap again
                randomTries++;
            }
        }

        // Caution. Remove this if necessary.
        break;

    case 2:

        /* Perturbation mode 2: Insert Jobs in other vehicles */
        for (int i = 0; i < sizePerturb; i++) {

            if (randomTries > 2 * N) {
                // cout<<"Caution - Force break!! Tried N*2 times to swap between cars and get invalid move. \n";
                break;
            }
            int first = rand() % N;
            int sec = rand() % N;

            //Optimize - adding a new data structure
            int changeDadTries = 0;
            while (dad[first] == dad[sec]) {

                if (changeDadTries > 100) { //It means we got stuck because ALL jobs are in just one vehicle
                    return solution;
                }

                first = rand() % N;
                changeDadTries++;
            }

            int carFirst = dad[first];
            int carSec = dad[sec];

            if (capacities[carFirst] >= (carrying[carFirst] - jobSize[first] + jobSize[sec])
                && (capacities[carSec] >= (carrying[carSec] - jobSize[sec] + jobSize[first]))) {

                //Updating car's carried weight
                carrying[carFirst] = carrying[carFirst] - jobSize[first] + jobSize[sec];
                carrying[carSec] = carrying[carSec] - jobSize[sec] + jobSize[first];

                //Updating carrying vehicles' id
                swap(dad[first], dad[sec]);

                //Create an array for both of vehicles
                int carFirstPosiOrder = posiVehicleOrder[carFirst];
                int carSecPosiOrder = posiVehicleOrder[carSec];

                int beg1 = order[carFirstPosiOrder].initialPos;
                int beg2 = order[carSecPosiOrder].initialPos;

                int end1 = order[carFirstPosiOrder].finalPos;
                int end2 = order[carSecPosiOrder].finalPos;

                int sizeFirst = end1 - beg1 + 1;
                int sizeSec = end2 - beg2 + 1;

                vector<int> jobsFirst(sizeFirst, -1);
                vector<int> jobsSec(sizeSec, -1);

                //Generating random positions to insert job in vehicle
                int insertOne = rand() % sizeFirst;
                int insertTwo = rand() % sizeSec;

                //Generating new array of jobs of vehicles!

                jobsFirst[insertOne] = sec;
                jobsSec[insertTwo] = first;

                int in1 = 0;
                int in2 = 0;
                for (int k = beg1; k <= end1; k++) {

                    if (in1 < (jobsFirst.size() - 1) && jobsFirst[in1] != -1)
                        in1++;

                    if (solution[k].job && solution[k].id == first) {
                        continue;
                    }
                    else {
                        jobsFirst[in1++] = solution[k].id;
                    }
                }

                for (int k = beg2; k <= end2; k++) {

                    if (in2 < (jobsSec.size() - 1) && jobsSec[in2] != -1)
                        in2++;

                    if (solution[k].job && solution[k].id == sec) {
                        continue;
                    }
                    else {
                        jobsSec[in2++] = solution[k].id;
                    }
                }

                //Inserting the new sequences in solution
                int walk = 0;
                while (walk < solution.size()) {

                    int walkAhead = walk + 1;

                    if (solution[walk].job == false) {

                        if (solution[walk].id == carFirst) {
                            int copyfirst = 0;
                            while (walkAhead < solution.size() && solution[walkAhead].job == true) {
                                solution[walkAhead++].id = jobsFirst[copyfirst++];
                            }
                        }
                        else if (solution[walk].id == carSec) {
                            int copySec = 0;
                            while (walkAhead < solution.size() && solution[walkAhead].job == true) {
                                solution[walkAhead++].id = jobsSec[copySec++];
                            }
                        }
                        else { // some other car
                            while (walkAhead < solution.size() && solution[walkAhead].job == true) {
                                walkAhead++;
                            }
                        }
                    }
                    walk = walkAhead;
                }

                randomTries = 0;
            }
            else {
                i--; //Swap again
                randomTries++;
            }
        }

        break;
    }

    return solution;
}


// Reviews: III
double IG_InsertPos(bool simulate, int pos, const data &Data, vector<data> &S, int N, int K, const vector<double>& w, 
    const vector<int>& P, const vector<vector<int> >& t, const vector<int>& F, const vector<int>& d, const vector<int>& Q, const vector<int>& s) 
{
    vector<data> S_Temp = S;

    // Swapping positions backwards = inserting in position.
    for(int i = S.size() - 2, j = S.size() - 1; i >= 1 && i >= pos; i--, j--)
        swap(S_Temp[i], S_Temp[j]);

    // Evaluate solution.
    vector<vehicleLoaded> vOrder = getVOrder(S_Temp, K);
    double ObjF = ObjectiveFunction(S_Temp, vOrder, false, K, N, t, w, P, d, F, -1);

    if(!simulate)
        S = S_Temp;

    return ObjF;
}

// Reviews: I
void IG_AlocateData(const data &Data, vector<data> &S, int N, int K, const vector<double>& w, const vector<int>& P, const vector<vector<int> >& t,
    const vector<int>& F, const vector<int>& d, const vector<int>& Q, const vector<int>& s)
{
    S.push_back(Data);

    // Inserting at every position from [1, N + K] and calculating total cost.
    priority_queue<pair<double, int>> status;
    for(int i = 1; i < S.size(); i++)
    {
        double ObjF = IG_InsertPos(true, i, Data, S, N, K, w, P, t, F, d, Q, s);
        status.push(make_pair(-1.0 * ObjF, i));        
    }

    // Insert in best position available.
    IG_InsertPos(false, status.top().second, Data, S, N, K, w, P, t, F, d, Q, s);
}

// Remove assert.
// Reviews: II
pair<bool, double> IteratedGreedy(vector<data> &S, int N, int K, const vector<double>& w, const vector<int>& P, const vector<vector<int> >& t,
    const vector<int>& F, const vector<int>& d, const vector<int>& Q, const vector<int>& s, double removePercentual)
{
    pair<bool, double> answer;
    int removeSz = ceil(removePercentual * N);

    assert(S.size() == (N + K));
    
    // Insert positions from [1, N + K) in an array and shuffle it.
    vector<int> removePos (N + K - 1);
    for(int i = 1; i < (N + K); i++)
        removePos[i - 1] = i;
    random_shuffle(removePos.begin(), removePos.end());

    // Copy data from removed positions.
    vector<data> reinsert; 
    reinsert.reserve(removeSz);
    unordered_set<int> setRemove;
    for(int i = 0, j = 0; j < removeSz && i < removePos.size(); i++, j++)
    {
        reinsert.push_back(S[removePos[i]]);
        setRemove.insert(removePos[i]);
    }

    // Copy new array to S_New.
    vector<data> S_New;
    S_New.reserve(N + K - removeSz);
    for(int i = 0; i < S.size() && S_New.size() < (S.size() - removeSz); i++)
        if(setRemove.find(i) == setRemove.end())
            S_New.push_back(S[i]);

    assert(S_New.size() == (N + K - removeSz));
    assert(removeSz == reinsert.size());
    // Try to reinsert each removed data.
    for(int i = 0; i < reinsert.size(); i++)
        IG_AlocateData(reinsert[i], S_New, N, K, w, P, t, F, d, Q, s);

    // If feasible: accept and evaluate.
    if(IsFeasible(S_New, Q, s, N, K))
    {
        assert(S_New.size() == (N + K));
        S = S_New;
        vector<vehicleLoaded> vOrder = getVOrder(S, K);
        double ObjF = ObjectiveFunction(S, vOrder, false, K, N, t, w, P, d, F, -1);

        return make_pair(true, ObjF);
    }

    return make_pair(false, INF);
}

pair<double, vector<data>> ILS_RVND_1(int N, int K, const vector<double>& w, const vector<int>& P, const vector<vector<int> >& t,
    const vector<int>& F, const vector<int>& d, const vector<int>& Q, const vector<int>& s, int maxIter, int maxIterIls, int perturbSize)
{

    int nbValidSolutions = 0;
    vector<vector<data> > validSolutions;

    // Generating initial greedy solutions
    vector<data> atcConfig = GreedySolution(false, 0, s, N, K, w, P, d, F, Q);
    vector<data> wmddConfig = GreedySolution(false, 1, s, N, K, w, P, d, F, Q);
    vector<data> weddConfig = GreedySolution(false, 2, s, N, K, w, P, d, F, Q);

    // Verifying if they are really valid

    if (IsFeasible(atcConfig, Q, s, N, K)) {
        validSolutions.push_back(atcConfig);
        nbValidSolutions++;
    }

    if (IsFeasible(wmddConfig, Q, s, N, K)) {

        validSolutions.push_back(wmddConfig);
        nbValidSolutions++;
    }

    if (IsFeasible(weddConfig, Q, s, N, K)) {

        validSolutions.push_back(weddConfig);
        nbValidSolutions++;
    }

    int contGetSolution = 0;

    double bestResult = INF;
    vector<data> bestSolution;

    for (int a = 0; a < maxIter; a++) {

        vector<data> solution;
        pair<double, vector<data> > callRvnd;

        //Getting the greedy solutions if there is one avaiable

        bool justCalledRvnd = true;
        if (contGetSolution < nbValidSolutions) {

            solution = validSolutions[contGetSolution];

            callRvnd = RVND(true, N, K, w, P, t, F, d, Q, s, solution);

            contGetSolution++;
        }
        else {
            //Otherwise we generate a random one
            callRvnd = RVND(false, N, K, w, P, t, F, d, Q, s, solution);
        }

        for (int b = 0; b < maxIterIls; b++) {

            if (!justCalledRvnd) {        	
                
                callRvnd = RVND(true, N, K, w, P, t, F, d, Q, s, solution);
            }
            else {
                justCalledRvnd = false;
            }

            if (callRvnd.first < bestResult) {

                bestResult = callRvnd.first;
                bestSolution = callRvnd.second;

                b = 0; //Reseting ILS!!
            }
            solution = perturb(solution, Q, s, N, K, perturbSize);
        }
    }
    if (IsFeasible(bestSolution, Q, s, N, K))
        return { bestResult, bestSolution };
    else
        return { -1, bestSolution };
}

pair<double, vector<data>> ILS_RVND_1_SBPO(int N, int K, const vector<double>& w, const vector<int>& P, const vector<vector<int> >& t,
    const vector<int>& F, const vector<int>& d, const vector<int>& Q, const vector<int>& s, int maxIter, int maxIterIls, int perturbSize)
{

    int feasibleNb = 0;
    vector<vector<data> > feasibleVec;

    // Generating initial greedy solutions
    vector<data> atcConfig = GreedySolution(false, 0, s, N, K, w, P, d, F, Q);
    vector<data> wmddConfig = GreedySolution(false, 1, s, N, K, w, P, d, F, Q);
    vector<data> weddConfig = GreedySolution(false, 2, s, N, K, w, P, d, F, Q);

    // Verifying if greedy solutions are feasible.

    if (IsFeasible(atcConfig, Q, s, N, K)) {
        feasibleVec.push_back(atcConfig);
        feasibleNb++;
    }

    if (IsFeasible(wmddConfig, Q, s, N, K)) {

        feasibleVec.push_back(wmddConfig);
        feasibleNb++;
    }

    if (IsFeasible(weddConfig, Q, s, N, K)) {

        feasibleVec.push_back(weddConfig);
        feasibleNb++;
    }

    int contGetSolution = 0;

    pair<double, vector<data>> SBest;
    SBest.first = INF;

    for (int a = 0; a < maxIter; a++) {

        vector<data> solution;
        pair<double, vector<data>> S;

        //Get the greedy solutions if feasible.

        bool justCalledRvnd = true;
        if (contGetSolution < feasibleNb) {

            solution = feasibleVec[contGetSolution];

            S = RVND(true, N, K, w, P, t, F, d, Q, s, solution);

            contGetSolution++;
        }
        else {
            //Otherwise generate a random one.
            S = RVND(false, N, K, w, P, t, F, d, Q, s, solution);
        }

        for (int b = 0; b < maxIterIls; b++) {
            
            vector<data> S1 = perturb(S.second, Q, s, N, K, perturbSize);

            pair<double, vector<data>> S2 = RVND(true, N, K, w, P, t, F, d, Q, s, S1);
           
            if (S2.first < S.first) {
                S = S2;
                b = -1; //Reseting ILS iterations.
            }
        }
        if(S.first < SBest.first){
            SBest = S;
        }
    }

    if (IsFeasible(SBest.second, Q, s, N, K))
        return SBest;
    else
        exit(0);
}

pair<double, vector<data>> ILS_RVND_1_UPDATED(int N, int K, const vector<double>& w, const vector<int>& P, const vector<vector<int> >& t,
    const vector<int>& F, const vector<int>& d, const vector<int>& Q, const vector<int>& s, int maxIter, int maxIterIls, int perturbSize)
{

    int feasible_size = 0;
    vector<vector<data> > feasible;

    // Generate ATC Rule solution and check its feasibility.
    vector<data> atcConfig = GreedySolution(false, 0, s, N, K, w, P, d, F, Q);
    if (IsFeasible(atcConfig, Q, s, N, K)) {
        feasible.push_back(atcConfig);
        feasible_size++;
    }

    // Generate WMDD Rule solution and check its feasibility.
    vector<data> wmddConfig = GreedySolution(false, 1, s, N, K, w, P, d, F, Q);
    if (IsFeasible(wmddConfig, Q, s, N, K)) {
        feasible.push_back(wmddConfig);
        feasible_size++;
    }

    // Generate WEDD Rule solution and check its feasibility.
    vector<data> weddConfig = GreedySolution(false, 2, s, N, K, w, P, d, F, Q);
    if (IsFeasible(weddConfig, Q, s, N, K)) {
        feasible.push_back(weddConfig);
        feasible_size++;
    }

    int contGetSolution = 0;

    pair<double, vector<data>> S_Best;
    S_Best.first = std::numeric_limits<double>::infinity();

    for (int a = 0; a < maxIter; a++) {

        vector<data> S, S_2;
        pair<double, vector<data> > S_1, S_3;

        // Using Greedy Solutions when available in iterations [0,2].
        if (contGetSolution < feasible_size) {

            S = feasible[contGetSolution];
            S_1 = RVND(true, N, K, w, P, t, F, d, Q, s, S);
            contGetSolution++;
        }
        else {
            // Otherwise, in iterations [3,maxIter) Random solutions are generated.
            S_1 = RVND(false, N, K, w, P, t, F, d, Q, s, S);
        }

        // Updating best solution found.
        if (S_1.first < S_Best.first) 
            S_Best = S_1;        

        for (int b = 0; b < maxIterIls; b++) {

            S_2 = perturb(S_1.second, Q, s, N, K, perturbSize);
            S_3 = RVND(true, N, K, w, P, t, F, d, Q, s, S_2);

            // Accept S''' as new S' (solution to be perturbed later).
            if (S_3.first < S_1.first)
                S_1 = S_3;       

            // Best solution found. Reseting ILS.
            if (S_1.first < S_Best.first) {
                S_Best = S_1;
                b = -1;
            }
        }
    }
    
    return S_Best;
}

pair<double, vector<data>> ILS_RVND_2_SBPO(int N, int K, const vector<double>& w, const vector<int>& P, const vector<vector<int> >& t,
    const vector<int>& F, const vector<int>& d, const vector<int>& Q, const vector<int>& s, int maxIter, int maxIterIls, int perturbSize)
{

    int feasibleNb = 0;
    vector<vector<data> > feasibleVec;

    // Generating initial greedy solutions
    vector<data> atcConfig = GreedySolution(false, 0, s, N, K, w, P, d, F, Q);
    vector<data> wmddConfig = GreedySolution(false, 1, s, N, K, w, P, d, F, Q);
    vector<data> weddConfig = GreedySolution(false, 2, s, N, K, w, P, d, F, Q);

    // Verifying if greedy solutions are feasible.

    if (IsFeasible(atcConfig, Q, s, N, K)) {
        feasibleVec.push_back(atcConfig);
        feasibleNb++;
    }

    if (IsFeasible(wmddConfig, Q, s, N, K)) {

        feasibleVec.push_back(wmddConfig);
        feasibleNb++;
    }

    if (IsFeasible(weddConfig, Q, s, N, K)) {

        feasibleVec.push_back(weddConfig);
        feasibleNb++;
    }

    int contGetSolution = 0;

    pair<double, vector<data>> SBest;
    SBest.first = INF;

    for (int a = 0; a < maxIter; a++) {

        vector<data> solution;
        pair<double, vector<data>> S;

        //Get the greedy solutions if feasible.

        bool justCalledRvnd = true;
        if (contGetSolution < feasibleNb) {

            solution = feasibleVec[contGetSolution];

            S = RVND_Custom_updated(true, N, K, w, P, t, F, d, Q, s, solution);

            contGetSolution++;
        }
        else {
            //Otherwise generate a random one.
            S = RVND_Custom_updated(false, N, K, w, P, t, F, d, Q, s, solution);
        }

        for (int b = 0; b < maxIterIls; b++) {
            
            vector<data> S1 = perturb(S.second, Q, s, N, K, perturbSize);

            pair<double, vector<data>> S2 = RVND_Custom_updated(true, N, K, w, P, t, F, d, Q, s, S1);
           
            if (S2.first < S.first) {
                S = S2;
                b = 0; //Reseting ILS iterations.
            }
        }
        if(S.first < SBest.first){
            SBest = S;
        }
    }

    if (IsFeasible(SBest.second, Q, s, N, K))
        return SBest;
    else
        exit(0);
}

pair<double, vector<data>> ILS_RVND_2(int N, int K, const vector<double>& w, const vector<int>& P, const vector<vector<int> >& t,
    const vector<int>& F, const vector<int>& d, const vector<int>& Q, const vector<int>& s, int maxIter, int maxIterIls, int sizePerturb)
{

    int nbValidSolutions = 0;
    vector<vector<data> > validSolutions;

    // Generating initial greedy solutions
    vector<data> atcConfig = GreedySolution(false, 0, s, N, K, w, P, d, F, Q);
    vector<data> wmddConfig = GreedySolution(false, 1, s, N, K, w, P, d, F, Q);
    vector<data> weddConfig = GreedySolution(false, 2, s, N, K, w, P, d, F, Q);

    //Danger: Verify ATC!!!
    // Verifying if they are really valid
    if (IsFeasible(atcConfig, Q, s, N, K)) {
        validSolutions.push_back(atcConfig);
        nbValidSolutions++;
    }
    if (IsFeasible(wmddConfig, Q, s, N, K)) {
        validSolutions.push_back(wmddConfig);
        nbValidSolutions++;
    }
    if (IsFeasible(weddConfig, Q, s, N, K)) {
        validSolutions.push_back(weddConfig);
        nbValidSolutions++;
    }

    int contGetSolution = 0;

    double bestResult = INF;
    vector<data> bestSolution;

    for (int a = 0; a < maxIter; a++) {

        vector<data> solution;
        pair<double, vector<data> > callRvndCustom;

        //Getting the greedy solutions if there is one avaiable

        bool justCalledRvndCustom = true;
        if (contGetSolution < nbValidSolutions) {

            solution = validSolutions[contGetSolution];

            callRvndCustom = RVND_Custom(true, N, K, w, P, t, F, d, Q, s, solution);

            contGetSolution++;
        }
        else {
            //Otherwise we generate a random one
            callRvndCustom = RVND_Custom(false, N, K, w, P, t, F, d, Q, s, solution);
        }

        for (int b = 0; b < maxIterIls; b++) {

            if (!justCalledRvndCustom) {

                callRvndCustom = RVND_Custom(true, N, K, w, P, t, F, d, Q, s, solution);
            }
            else {
                justCalledRvndCustom = false;
            }

            if (callRvndCustom.first < bestResult) {

                bestResult = callRvndCustom.first;
                bestSolution = callRvndCustom.second;

                b = 0; //Reseting ILS!!
            }
            solution = perturb(solution, Q, s, N, K, sizePerturb);
        }
    }

    if (IsFeasible(bestSolution, Q, s, N, K))
        return { bestResult, bestSolution };
    else
        return { -1, bestSolution };
}

// Reviews: II
pair<bool, vector<data>> CROSSOVER_1_POINT(const vector<data>& S1, const vector<data>& S2, const vector<int>& Q, const vector<int>& S, int N, int K)
{
    int cutSize;
    do{
        cutSize = rand() % (S1.size());
    }
    while (cutSize == (S1.size() - 1));

    // Remove cutSize first positions from S1.
    vector<data> answer;
    answer.reserve(N + K);
    unordered_set<int> cuttedJob, cuttedVehicle;
  
    for(int i = cutSize; i < S1.size(); i++)
        if(S1[i].job)
            cuttedJob.insert(S1[i].id);
        else
            cuttedVehicle.insert(S1[i].id);

    for(int i = 0; i < cutSize; i++)
        answer.push_back(S1[i]);

    // Reinsert them following S2's order.
    for(int i = 0; i < S2.size(); i++){
        if(S2[i].job){
            if(cuttedJob.find(S2[i].id) != cuttedJob.end())
                answer.push_back(S2[i]);
        }
        else{
            if(cuttedVehicle.find(S2[i].id) != cuttedVehicle.end())
                answer.push_back(S2[i]);            
        }
    }

    assert(answer.size() == S1.size());

    for(auto i: answer)
        if(i.job)
            cout << "J"<<i.id<<" ";
        else
            cout << "V"<<i.id<<" ";
    cout << endl;

    // Checking feasibility.
    bool isFeasible = IsFeasible(answer, Q, S, N, K);
    return make_pair(isFeasible, answer);
}

pair<bool, vector<data>> CROSSOVER_2_POINT(const vector<data>& f1, const vector<data>& f2, const vector<int>& capacities, const vector<int>& jobSize, int N, int K)
{

    //generating random interval to 2-point crossOver

    int a = rand() % (f1.size());
    int b = rand() % (f1.size());

    while (a == b || a == (f1.size() - 1) || (a == 0) || (b == 0) || (a > b)) {
        a = rand() % (f1.size());
        b = rand() % (f1.size());
    }
    unordered_set<int> jobsCross;
    unordered_set<int> carsCross;

    for (int i = a; i <= b; i++) { // i <= b?
        if (f1[i].job) {
            jobsCross.insert(f1[i].id);
        }
        else {
            carsCross.insert(f1[i].id);
        }
    }

    vector<data> son(f1.size());

    vector<data> neworder;

    for (int i = 0; i < f2.size(); i++) {
        if (f2[i].job) {
            if (jobsCross.find(f2[i].id) != jobsCross.end()) { //job is int our cross
                data temp;
                temp.job = true;
                temp.id = f2[i].id;
                neworder.push_back(temp);
            }
        }
        else {
            if (carsCross.find(f2[i].id) != carsCross.end()) { //job is int our cross
                data temp;
                temp.job = false;
                temp.id = f2[i].id;
                neworder.push_back(temp);
            }
        }
    }

    int insert = 0;
    int copy = 0;

    while (insert < f1.size()) {
        if (insert < a || insert > b) {
            son[insert] = f1[insert];
        }
        else {
            son[insert] = neworder[copy++];
        }
        insert++;
    }

    bool isFeasible = IsFeasible(son, capacities, jobSize, N, K);

    return make_pair(isFeasible, son);
}

pair<double, vector<data>> FastLocalSearch(bool reuse, int N, int K, const vector<double>& w, const vector<int>& P, const vector<vector<int> >& t,
    const vector<int>& F, const vector<int>& d, const vector<int>& Q, const vector<int>& s, vector<data>& S)
{

    vector<vehicleLoaded> vOrder = getVOrder(S, K); //Generating info about vehicle transportation

    Neighborhood_1('B', S, vOrder, N, K, w, P, t, F, d);
    Neighborhood_2('B', S, vOrder, N, K, w, P, t, F, d, Q, s);
    Neighborhood_3('B', S, vOrder, N, K, w, P, t, F, d);
    Neighborhood_4('B', S, vOrder, N, K, w, P, t, F, d, Q, s);
    Neighborhood_5('B', S, vOrder, N, K, w, P, t, F, d);
    Neighborhood_6('B', S, vOrder, N, K, w, P, t, F, d);
    Neighborhood_7('B', S, vOrder, N, K, w, P, t, F, d, Q, s);

    // Calculate new OBJ Function.
    double finalObj = ObjectiveFunction(S, vOrder, true, K, N, t, w, P, d, F, -1);

    return make_pair(finalObj, S);
}

pair<double, vector<data>> GA_LS(int N, int K, const vector<double>& w, const vector<int>& P, const vector<vector<int> >& t,
    const vector<int>& F, const vector<int>& d, const vector<int>& Q, const vector<int>& s, int popSize)
{

    vector<double> popObj(popSize, -1);
    vector<vector<data> > pop(popSize);

    vector<data> bestConfig;
    double bestObj = INF;

    for (int i = 0; i < popSize; i++) {
        vector<data> trash; // will be use as a puppet to call function generateRandom
        pair<double, vector<data> > callRvnd = RVND(false, N, K, w, P, t, F, d, Q, s, trash);
        pop[i] = callRvnd.second;
        popObj[i] = callRvnd.first;

        if (popObj[i] < bestObj) {
            bestObj = popObj[i];
            bestConfig = pop[i];
        }
    }

    int maxIter = 8 * (N + K);
    int contIter = 0;
    while (contIter++ < maxIter) {

        int newPopSize = 0;
        vector<vector<data> > newPop(popSize);
        vector<double> newPopObj(popSize, -1);

        //It is (popSize-1) to carry the best solution
        while (newPopSize < (popSize - 1)) {

            int father1 = -1;
            int father2 = -1;

            // Generatin fathers 1 and 2 by tournament
            while (father1 == father2) {
                int fatherLeft1 = rand() % popSize;
                int fatherLeft2 = rand() % popSize;
                while (fatherLeft1 == fatherLeft2) //Need to be DIF
                    fatherLeft1 = rand() % popSize;

                if (popObj[fatherLeft1] < popObj[fatherLeft2]) {
                    father1 = fatherLeft1;
                }
                else {
                    father1 = fatherLeft2;
                }

                int fatherRight1 = rand() % popSize;
                int fatherRight2 = rand() % popSize;
                while (fatherRight1 == fatherRight2) //Need to be DIF
                    fatherRight1 = rand() % popSize;

                if (popObj[fatherRight1] < popObj[fatherRight2]) {
                    father2 = fatherRight1;
                }
                else {
                    father2 = fatherRight2;
                }
            }

            pair<bool, vector<data> > son1 = CROSSOVER_2_POINT(pop[father1], pop[father2], Q, s, N, K);
            pair<bool, vector<data> > son2 = CROSSOVER_2_POINT(pop[father1], pop[father2], Q, s, N, K);

            // Both feasible: Binary Tournament.
            if (son1.first && son2.first) {

                vector<vehicleLoaded> vOrder1 =  getVOrder(son1.second, K);
                double son1_Obj = ObjectiveFunction(son1.second, vOrder1, false, K, N, t, w, P, d, F, -1);

                vector<vehicleLoaded> vOrder2 =  getVOrder(son2.second, K);
                double son2_Obj = ObjectiveFunction(son2.second, vOrder2, false, K, N, t, w, P, d, F, -1);

                if(son1_Obj <= son2_Obj)
                    newPop[newPopSize] = son1.second;
                else
                    newPop[newPopSize] = son2.second;
            }
            else if (son1.first) {
                newPop[newPopSize] = son1.second;
            }
            else if (son2.first) {
                newPop[newPopSize] = son2.second;
            }
            else { 
                // Both offsprings infeasible, pick one father randomly.
                int randomPickFather = rand() % 2;

                if (randomPickFather == 0)
                    newPop[newPopSize] = pop[father1];
                else
                    newPop[newPopSize] = pop[father2];
                
                // Apply mutation.
                vector<data> afterMutation = mutation(newPop[newPopSize], Q, s, N, K);
                newPop[newPopSize] = afterMutation;
            }
            ++newPopSize;
        }

        //Putting in the last position the best solution
        newPop[newPopSize] = bestConfig;
        newPopObj[newPopSize] = bestObj;

        for (int i = 0; i < popSize; i++) {
            
            // Apply fast local search.
            pair<double, vector<data>> newS = FastLocalSearch(true, N, K, w, P, t, F, d, Q, s, newPop[i]);
            pop[i] = newS.second;
            popObj[i] = newS.first;

            if (popObj[i] < bestObj) {
                bestObj = popObj[i];
                bestConfig = pop[i];
            }
        }
        
    }

    return make_pair(bestObj, bestConfig);
}

pair<double, vector<data>> GA_LS_UPDATED(int N, int K, const vector<double>& w, const vector<int>& P, const vector<vector<int> >& t,
    const vector<int>& F, const vector<int>& d, const vector<int>& Q, const vector<int>& s, int popSize)
{

    // Generating greedy rule's solutions.
    int feasible_size = 0;
    vector<vector<data>> feasible;

    // Generate ATC Rule solution and check its feasibility.
    vector<data> atcConfig = GreedySolution(false, 0, s, N, K, w, P, d, F, Q);
    if (IsFeasible(atcConfig, Q, s, N, K)) {
        feasible.push_back(atcConfig);
        feasible_size++;
    }

    // Generate WMDD Rule solution and check its feasibility.
    vector<data> wmddConfig = GreedySolution(false, 1, s, N, K, w, P, d, F, Q);
    if (IsFeasible(wmddConfig, Q, s, N, K)) {
        feasible.push_back(wmddConfig);
        feasible_size++;
    }

    // Generate WEDD Rule solution and check its feasibility.
    vector<data> weddConfig = GreedySolution(false, 2, s, N, K, w, P, d, F, Q);
    if (IsFeasible(weddConfig, Q, s, N, K)) {
        feasible.push_back(weddConfig);
        feasible_size++;
    }

    int getGreedy = 0;


    vector<double> popObj(popSize, -1);
    vector<vector<data> > pop(popSize);

    vector<data> bestConfig;
    double bestObj = std::numeric_limits<double>::infinity();

    for (int i = 0; i < popSize; i++) {

        vector<data> empty;
        pair<double, vector<data>> S;

        if(feasible_size > 0){
            S = RVND(true, N, K, w, P, t, F, d, Q, s, feasible[getGreedy]);
            getGreedy++;
            feasible_size--;
        }
        else{
            S = RVND(false, N, K, w, P, t, F, d, Q, s, empty);
        }

        pop[i] = S.second;
        popObj[i] = S.first;

        if (popObj[i] < bestObj) {
            bestObj = popObj[i];
            bestConfig = pop[i];
        }
    }

    int maxIter = 8 * (N + K);
    int contIter = 0;

    while (contIter++ < maxIter) {

        int newPopSize = 0;
        vector<vector<data> > newPop(popSize);
        vector<double> newPopObj(popSize, -1);

        // PSIZE-1 solutions due to last solution will get best solution found.
        while (newPopSize < (popSize - 1)) {

            int father1 = -1;
            int father2 = -1;

            // Selecting Parents by individual tournaments.
            while (father1 == father2) {

                int fatherLeft1 = rand() % popSize;
                int fatherLeft2 = rand() % popSize;

                while (fatherLeft1 == fatherLeft2) //Need to be DIF
                    fatherLeft1 = rand() % popSize;

                if (popObj[fatherLeft1] < popObj[fatherLeft2]) 
                    father1 = fatherLeft1;                
                else 
                    father1 = fatherLeft2;
                
                int fatherRight1 = rand() % popSize;
                int fatherRight2 = rand() % popSize;
                while (fatherRight1 == fatherRight2) //Need to be DIF
                    fatherRight1 = rand() % popSize;

                if (popObj[fatherRight1] < popObj[fatherRight2]) 
                    father2 = fatherRight1;                
                else 
                    father2 = fatherRight2;
                
            }

            pair<bool, vector<data> > son1 = CROSSOVER_2_POINT(pop[father1], pop[father2], Q, s, N, K);
            pair<bool, vector<data> > son2 = CROSSOVER_2_POINT(pop[father1], pop[father2], Q, s, N, K);

            // Both feasible: Binary Tournament.
            if (son1.first && son2.first) {

                vector<vehicleLoaded> vOrder1 =  getVOrder(son1.second, K);
                double son1_Obj = ObjectiveFunction(son1.second, vOrder1, false, K, N, t, w, P, d, F, -1);

                vector<vehicleLoaded> vOrder2 =  getVOrder(son2.second, K);
                double son2_Obj = ObjectiveFunction(son2.second, vOrder2, false, K, N, t, w, P, d, F, -1);

                if(son1_Obj <= son2_Obj)
                    newPop[newPopSize] = son1.second;
                else
                    newPop[newPopSize] = son2.second;
            }
            else if (son1.first) {
                newPop[newPopSize] = son1.second;
            }
            else if (son2.first) {
                newPop[newPopSize] = son2.second;
            }
            else { 
                // Both offsprings infeasible, pick one father randomly.
                int randomPickFather = rand() % 2;

                if (randomPickFather == 0)
                    newPop[newPopSize] = pop[father1];
                else
                    newPop[newPopSize] = pop[father2];
                
                // Apply mutation.
                vector<data> afterMutation = mutation(newPop[newPopSize], Q, s, N, K);
                newPop[newPopSize] = afterMutation;
            }
            ++newPopSize;
        }

        //Putting in the last position the best solution
        newPop[newPopSize] = bestConfig;
        newPopObj[newPopSize] = bestObj;

        //Fast Local Search in new population
        // AND calculating object function!!

        for (int i = 0; i < popSize; i++) {

            if(i < popSize/3){
                pair<double, vector<data> > newS = FastLocalSearch(true, N, K, w, P, t, F, d, Q, s, newPop[i]);
                pop[i] = newS.second;
                popObj[i] = newS.first;
            }else{
                pop[i] = newPop[i];
                vector<vehicleLoaded> vOrder =  getVOrder(pop[i], K);
                popObj[i] = ObjectiveFunction(pop[i], vOrder, false, K, N, t, w, P, d, F, -1);
            }
          
            if (popObj[i] < bestObj) {
                bestObj = popObj[i];
                bestConfig = pop[i];
            }
        }
        
    }

    return { bestObj, bestConfig };
}

void Test(int N, int K, const vector<double>& w, const vector<int>& P, const vector<vector<int> >& t,
    const vector<int>& F, const vector<int>& d, const vector<int>& Q, const vector<int>& s)
{
    for(int i = 0; i < 10; i++)
    {
        vector<data> Solution = RandomSolution(N, K, Q, s);
        vector<vehicleLoaded> vOrder = getVOrder(Solution, K);
        double ObjF = ObjectiveFunction(Solution, vOrder, false, K, N, t, w, P, d, F, -1);

        auto begin = chrono::high_resolution_clock::now();  
        int tries = 0;
        auto ans = IteratedGreedy(Solution, N, K, w, P, t, F, d, Q, s, 0.1);
        while(!ans.first)
        {
            ans = IteratedGreedy(Solution, N, K, w, P, t, F, d, Q, s, 0.1);
            tries++;
        }
        auto end = chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - begin).count();

        cout << i <<": From"<< setw(13) << ObjF << setw(6) << " to " << setw(13) << ans.second << setw(12) << "tries: " << setw(3) << tries 
             << " in " << setw(6) << duration << "ms. " << setw(5) << "Got" << setw(4) << (int) (100 - ((ans.second * 100)/ObjF)) <<" (%) better.\n";
    }
    cout << "\n";
    /*
    cout <<"Delete here\n";
    // Generate Random Solutions.
    for(int i = 0; i < 10; i++)
    {
        vector<data> Solution = RandomSolution(N, K, Q, s);
        if(IsFeasible(Solution, Q, s, N, K)){
            vector<vehicleLoaded> vOrder = getVOrder(Solution, K);
            cout << "Random Feasible. Obj. Function: " << ObjectiveFunction(Solution, vOrder, false, K, N, t, w, P, d, F, -1) << "\n";

            cout << "SOL1: Before 1-point cross over: ";
            for(data j: Solution)
                if(j.job)
                    cout << "J" << j.id << " ";
                else
                    cout << "V" << j.id << " ";
            cout << "\n";

            vector<data> Solution2 = RandomSolution(N, K, Q, s);
            cout << "SOL2: Before 1-point cross over: ";
            for(data j: Solution2)
                if(j.job)
                    cout << "J" << j.id << " ";
                else
                    cout << "V" << j.id << " ";
            cout << "\n";
            
            pair<bool, vector<data>> Offspring = CROSSOVER_1_POINT(Solution, Solution2, Q, s, N, K);

            if(!Offspring.first){
                cout <<  "Infeasible CrossOver.\n";
            }else{

               assert(Offspring.second.size() == (N + K));
               cout <<"OBJ.F. After cross: " << ObjectiveFunction(Offspring.second, vOrder, true, K, N, t, w, P, d, F, -1) << "\n";
                cout << "OFFSPRING: After 1-point cross over: ";
                for(data j: Offspring.second)
                    if(j.job)
                        cout << "J" << j.id << " ";
                    else
                        cout << "V" << j.id << " "; 
            }
            
            cout << "\n\n\n\n";
            
        }else{
            cout << "Aborted.\n";
            exit(0);
        }
    }

    cout << "-----------------------------------\n";
    // Generate Constructive Greedy Solutions

    for(int i = 0; i < 10; i++)
    {
        vector<data> Solution = GreedySolution(true, 1000, s, N, K, w, P, d, F, Q);
        if(IsFeasible(Solution, Q, s, N, K)){

            vector<vehicleLoaded> vOrder = getVOrder(Solution, K);
            cout << "CONSTRUCTIVE. Obj. Function: " << ObjectiveFunction(Solution, vOrder, false, K, N, t, w, P, d, F, -1) << "\n";

            for(data j: Solution)
                if(j.job)
                    cout << "J" << j.id << " ";
                else
                    cout << "V" << j.id << " ";
            cout << "\n";
        }else{
            cout <<"CONSTRUCTIVE INFEASIBLE!!\n";
        }
    } */
}

#endif