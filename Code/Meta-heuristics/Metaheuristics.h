#ifndef HEURISTICS_H
#define HEURISTICS_H

vector<data> greedyMethod(const string& which, const vector<int>& jobSize, int N, int K, const vector<double>& weight,
    const vector<int>& procTime, const vector<int>& dueDate, const vector<int>& carPrices, const vector<int>& carCap)
{
    //Calculating processing time average
    double procTimeAvg = 0;
    bool success = true; // If we generate a valid configuration

    for (int i = 0; i < N; i++)
        procTimeAvg += procTime[i];

    procTimeAvg /= N;

    //Variables to generate processing order of jobs:
    vector<data> jobOrder(N);
    vector<bool> visitedJob(N, false);
    int acumulatedD = 0;

    // cout<<"numtrbs"<<N<<endl;
    for (int i = 0; i < N; i++) {

        //Create priority queue of Dispatching Rule
        priority_queue<pair<double, int> > pq;

        for (int j = 0; j < N; j++) {

            if (visitedJob[j])
                continue;

            double value = 0;

            if (which == atc) {

                value = (weight[j] / procTime[j]) * exp((-1) * (max(dueDate[j] - procTime[j] - acumulatedD, 0)) / procTimeAvg);
                pq.push(make_pair(value, j));
            }
            else if (which == wmdd) {

                value = (1 / weight[j]) * max(procTime[j], dueDate[j] - acumulatedD);
                pq.push(make_pair((-1) * value, j));
            }
            else if (which == wedd) {

                value = (dueDate[j] / weight[j]);
                pq.push(make_pair((-1) * value, j));
            }
        }

        jobOrder[i].id = (pq.top()).second;
        int jobInserted = jobOrder[i].id;
        // cout<<"inseri "<<jobInserted<<endl;

        visitedJob[jobInserted] = true;

        acumulatedD += procTime[jobInserted];
    }

    // Puting in a greedy way the cars to carry the jobs, choosing the cheaper one.
    vector<data> vehicleDispatching(K);

    priority_queue<pair<int, int> > vehiclePrice; //We will select the cars in the cheapest order.

    for (int i = 0; i < K; i++) {
        vehiclePrice.push(make_pair((-1) * carPrices[i], i)); //Ascending order ( *(-1) )
    }

    vector<int> carbyprice;
    while (!vehiclePrice.empty()) {
        carbyprice.push_back(vehiclePrice.top().second);
        vehiclePrice.pop();
    }

    //Creating adj list of carried job ( sorted by car prices in vector below)
    vector<vector<int> > adjcarry(K);

    int job = 0;
    for (int i = 0; i < K; i++) {
        int cap = carCap[carbyprice[i]]; //getting id and cap

        int accCap = 0;
        while (job < N && (accCap + jobSize[jobOrder[job].id]) <= cap) {
            adjcarry[i].push_back(jobOrder[job].id);
            accCap += jobSize[jobOrder[job].id];
            job++;
        }
    }

    vector<data> configuration(N + K);
    int configSize = configuration.size();

    int insert = 0;
    for (int i = 0; i < adjcarry.size(); i++) {
        configuration[insert].job = false;
        configuration[insert++].id = carbyprice[i];
        for (int j = 0; j < adjcarry[i].size(); j++) {
            configuration[insert].job = true;
            configuration[insert++].id = adjcarry[i][j];
        }
    }
    return configuration;
}

//Soft transformation: swap two jobs between different vehicles
vector<data> mutation(vector<data>& solution, const vector<int>& capacities, const vector<int>& jobSize, int N, int K)
{

    vector<int> carrying = carryWeight(solution, jobSize, K);

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

    vector<int> carrying = carryWeight(solution, jobSize, K);

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

        // case 3:
    }

    return solution;
}

pair<double, vector<data> > ils_rvnd(int N, int K, const vector<double>& w, const vector<int>& P, const vector<vector<int> >& t,
    const vector<int>& F, const vector<int>& d, const vector<int>& Q, const vector<int>& s, int maxIter, int maxIterIls, int perturbSize)
{

    int nbValidSolutions = 0;
    vector<vector<data> > validSolutions;

    // Generating initial greedy solutions
    vector<data> atcConfig = greedyMethod(atc, s, N, K, w, P, d, F, Q);
    vector<data> weddConfig = greedyMethod(wedd, s, N, K, w, P, d, F, Q);
    vector<data> wmddConfig = greedyMethod(wmdd, s, N, K, w, P, d, F, Q);

    // Verifying if they are really valid

    if (validConfig(atcConfig, Q, s, N, K)) {
        validSolutions.push_back(atcConfig);
        nbValidSolutions++;
    }

    if (validConfig(wmddConfig, Q, s, N, K)) {

        validSolutions.push_back(wmddConfig);
        nbValidSolutions++;
    }

    if (validConfig(weddConfig, Q, s, N, K)) {

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
    if (validConfig(bestSolution, Q, s, N, K))
        return { bestResult, bestSolution };
    else
        return { -1, bestSolution };
}

pair<double, vector<data> > ils_rvnd_custom(int N, int K, const vector<double>& w, const vector<int>& P, const vector<vector<int> >& t,
    const vector<int>& F, const vector<int>& d, const vector<int>& Q, const vector<int>& s, int maxIter, int maxIterIls, int sizePerturb)
{

    int nbValidSolutions = 0;
    vector<vector<data> > validSolutions;

    // Generating initial greedy solutions
    vector<data> atcConfig = greedyMethod(atc, s, N, K, w, P, d, F, Q);
    vector<data> weddConfig = greedyMethod(wedd, s, N, K, w, P, d, F, Q);
    vector<data> wmddConfig = greedyMethod(wmdd, s, N, K, w, P, d, F, Q);

    //Danger: Verify ATC!!!
    // Verifying if they are really valid
    if (validConfig(atcConfig, Q, s, N, K)) {
        validSolutions.push_back(atcConfig);
        nbValidSolutions++;
    }
    if (validConfig(wmddConfig, Q, s, N, K)) {
        validSolutions.push_back(wmddConfig);
        nbValidSolutions++;
    }
    if (validConfig(weddConfig, Q, s, N, K)) {
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

    if (validConfig(bestSolution, Q, s, N, K))
        return { bestResult, bestSolution };
    else
        return { -1, bestSolution };
}

//pair< BOOL, solution > indicating if it is valid
pair<bool, vector<data> > crossOver(const vector<data>& f1, const vector<data>& f2, const vector<int>& capacities, const vector<int>& jobSize, int N, int K)
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

    bool isvalid = validConfig(son, capacities, jobSize, N, K);

    return { isvalid, son };
}

// * Ident OK
// GA-LS's local search method.
pair<double, vector<data> > fastLocalSearch(bool reuse, int N, int K, const vector<double>& w, const vector<int>& P, const vector<vector<int> >& t,
    const vector<int>& F, const vector<int>& d, const vector<int>& Q, const vector<int>& s, vector<data>& initialConfig)
{

    vector<vehicleLoaded> vehicleOrder = getVOrder(initialConfig, K); //Generating info about vehicle transportation

    nbhood1('B', initialConfig, vehicleOrder, N, K, w, P, t, F, d);
    nbhood2('B', initialConfig, vehicleOrder, N, K, w, P, t, F, d, Q, s);
    nbhood3('B', initialConfig, vehicleOrder, N, K, w, P, t, F, d);
    nbhood4('B', initialConfig, vehicleOrder, N, K, w, P, t, F, d, Q, s);
    nbhood5('B', initialConfig, vehicleOrder, N, K, w, P, t, F, d);
    nbhood6('B', initialConfig, vehicleOrder, N, K, w, P, t, F, d);

    // Calculate new OBJ Function.
    double finalObj = objFunction2(initialConfig, vehicleOrder, true, K, N, t, w, P, d, F, -1);

    return make_pair(finalObj, initialConfig);
}

pair<double, vector<data> > genAlgo1(int N, int K, const vector<double>& w, const vector<int>& P, const vector<vector<int> >& t,
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

            pair<bool, vector<data> > son1 = crossOver(pop[father1], pop[father2], Q, s, N, K);
            pair<bool, vector<data> > son2 = crossOver(pop[father1], pop[father2], Q, s, N, K);

            //new person to be inserted in new population
            vector<data> newPerson;

            //IF Both sons are feasible
            if (son1.first && son2.first) {
                newPop[newPopSize] = son1.second;
                newPop[newPopSize] = son2.second;

                //When just one son is feasible
            }
            else if (son1.first) {

                newPop[newPopSize] = son1.second;
                // newPopObj[ newPopSize ] = resSon1;
            }
            else if (son2.first) {

                newPop[newPopSize] = son2.second;
            }
            else { //both sons are infeasible

                //pick one father at random
                int randomPickFather = rand() % 2;

                if (randomPickFather == 0) {
                    newPop[newPopSize] = pop[father1];
                }
                else { // == 1
                    newPop[newPopSize] = pop[father2];
                }

                vector<data> afterMutation = mutation(newPop[newPopSize], Q, s, N, K);
                //Do mutation in father picked

                newPop[newPopSize] = afterMutation;
            }

            newPopSize++;
        }

        //Putting in the last position the best solution
        newPop[newPopSize] = bestConfig;
        newPopObj[newPopSize] = bestObj;

        //Fast Local Search in new population
        // AND calculating object function!!

        for (int i = 0; i < popSize; i++) {
            pair<double, vector<data> > callFastLocalSearch = fastLocalSearch(true, N, K, w, P, t, F, d, Q, s, newPop[i]);
            pop[i] = callFastLocalSearch.second;
            popObj[i] = callFastLocalSearch.first;

            if (popObj[i] < bestObj) {
                bestObj = popObj[i];
                bestConfig = pop[i];
            }
        }
    }

    return { bestObj, bestConfig };
}

#endif