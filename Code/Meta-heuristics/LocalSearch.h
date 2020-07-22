#ifndef LOCALSEARCH_H
#define LOCALSEARCH_H

using namespace std;

/*  Neighborhood 1: Swap sequential jobs inside a vehicle */
bool nbhood1(char bestOrFirst, vector<data>& solution, vector<vehicleLoaded>& VOrder, int N, int K, const vector<double>& w,
    const vector<int>& P, const vector<vector<int> >& t, const vector<int>& F, const vector<int>& d)
{

    vector<data> bestConfig;
    bool improved = false;
    double biggestImprove = 0;

    double initialVObj = objFunction2(solution, VOrder, false, K, N, t, w, P, d, F, -1);

    double bestObj = initialVObj;

    for (int i = 0; i < VOrder.size(); i++) {

        bool improvedCar = false;

        // IF There is just one job in vehicle, ignore ..
        if (VOrder[i].initialPos == VOrder[i].finalPos)
            continue;

        for (int j = VOrder[i].initialPos; j < VOrder[i].finalPos; j++) {

            for (int k = (j + 1); k <= VOrder[i].finalPos; k++) {

                swap(solution[j], solution[k]);

                double newObj = objFunction2(solution, VOrder, false, K, N, t, w, P, d, F, -1);

                if (newObj < (bestObj - EPS)) {

                    improved = true;
                    bestObj = newObj;
                    bestConfig = solution;

                    if (bestOrFirst == 'F')
                        return true;
                }
                //Jobs swap back.
                swap(solution[j], solution[k]);
            }
        }
    }

    if (improved) {
        solution = bestConfig;
        return true;
    }
    else {
        return false;
    }
}

/*  Neighborhood 2: Swap jobs between vehicles */
bool nbhood2(char bestOrFirst, vector<data>& solution, vector<vehicleLoaded>& VOrder, int N, int K, const vector<double>& w,
    const vector<int>& P, const vector<vector<int> >& t, const vector<int>& F, const vector<int>& d, const vector<int>& Q, const vector<int>& s)
{

    vector<int> currCap(K, 0);

    for (int i = 0; i < VOrder.size(); i++) {

        int whichVehicle = VOrder[i].id;

        for (int j = VOrder[i].initialPos; j <= VOrder[i].finalPos; j++)
            currCap[whichVehicle] += s[solution[j].id];
    }

    double bestObj = objFunction2(solution, VOrder, false, K, N, t, w, P, d, F, -1);

    double iniObj = bestObj;
    bool improved = false;
    vector<data> bestConfig = solution;

    for (int i = 0; i < VOrder.size() - 1; i++) {
        for (int j = VOrder[i].initialPos; j <= VOrder[i].finalPos; j++) {
            for (int next = i + 1; next < VOrder.size(); next++) {

                for (int k = VOrder[next].initialPos; k <= VOrder[next].finalPos; k++) {

                    //Checking if the capacity of each car will be respected after swap
                    if ((currCap[VOrder[i].id] - s[solution[j].id] + s[solution[k].id]) <= Q[VOrder[i].id]
                        && (currCap[VOrder[next].id] - s[solution[k].id] + s[solution[j].id]) <= Q[VOrder[next].id]) {

                        swap(solution[j], solution[k]);

                        double newDoubleConfig = objFunction2(solution, VOrder, false, K, N, t, w, P, d, F, -1);

                        if (newDoubleConfig < bestObj) {

                            improved = true;
                            bestConfig = solution;
                            bestObj = newDoubleConfig;

                            if (bestOrFirst == 'F') //If call is First Improvement, return.
                                return true;
                        }
                        swap(solution[j], solution[k]);
                    }
                }
            }
        }
    }

    if (improved) {
        solution = bestConfig;
        return true;
    }
    else {
        return false;
    }
}

/*  Neighborhood 3: Inserting jobs in positions inside a vehicle */
bool nbhood3(char bestOrFirst, vector<data>& solution, vector<vehicleLoaded>& VOrder, int N, int K, const vector<double>& w,
    const vector<int>& P, const vector<vector<int> >& t, const vector<int>& F, const vector<int>& d)
{

    vector<int> startVehicleTime(K, 0);
    vector<int> deliveryTime = calculatingDeliveryTime(solution, startVehicleTime, t, P, VOrder, N);
    vector<int> T = calculatingJobTardiness(deliveryTime, N, d);

    double bestImprove = 0;
    bool improved = false;
    vector<data> bestConfig = solution;

    double iniObj = objFunction2(solution, VOrder, false, K, N, t, w, P, d, F, -1);

    for (int i = 0; i < VOrder.size(); i++) {

        if (VOrder[i].initialPos == VOrder[i].finalPos)
            continue;

        double bestObj = objFunction2(solution, VOrder, false, K, N, t, w, P, d, F, i);

        //Saving the original Jobs solution of vehicle
        vector<int> original;
        for (int k = VOrder[i].initialPos; k <= VOrder[i].finalPos; k++)
            original.push_back(solution[k].id);

        for (int j = VOrder[i].initialPos; j <= VOrder[i].finalPos; j++) {

            int insertLeftward = j - VOrder[i].initialPos - 1;

            int contUpward = 0;

            //Inserting jobs leftward (except for the immediatly close)
            while (insertLeftward-- > 0) {

                vector<int> newOrder(VOrder[i].finalPos - VOrder[i].initialPos + 1, -1);
                set<int> jobsIn;

                int insertCounter = 0;

                newOrder[contUpward] = solution[j].id; //Inserting job in the required place
                jobsIn.insert(solution[j].id);

                for (int k = VOrder[i].initialPos; k <= VOrder[i].finalPos; k++) {

                    if (jobsIn.find(solution[k].id) == jobsIn.end()) {

                        while (insertCounter < newOrder.size() && newOrder[insertCounter] != -1)
                            insertCounter++;

                        newOrder[insertCounter] = solution[k].id;
                        jobsIn.insert(solution[k].id);
                    }
                }

                int CC = 0; // Copy Counter

                for (int k = VOrder[i].initialPos; k <= VOrder[i].finalPos; k++) {
                    solution[k].id = newOrder[CC];
                    CC++;
                }

                double newObj = objFunction2(solution, VOrder, false, K, N, t, w, P, d, F, i);

                if (newObj < bestObj && (newObj - bestObj) < bestImprove) {

                    bestImprove = (newObj - bestObj);
                    improved = true;
                    bestConfig = solution;

                    //If it is a First Improvement call, return
                    if (bestOrFirst == 'F')
                        return true;
                }

                CC = 0;

                for (int k = VOrder[i].initialPos; k <= VOrder[i].finalPos; k++) {
                    solution[k].id = original[CC];
                    CC++;
                }

                contUpward++;
            }

            int jobPosition = j - VOrder[i].initialPos + 1;
            int jobsNb = j + 1;

            while (jobsNb <= VOrder[i].finalPos) {

                vector<int> newOrder(VOrder[i].finalPos - VOrder[i].initialPos + 1, -1);
                set<int> jobsIn;

                newOrder[jobPosition] = solution[j].id;
                jobsIn.insert(solution[j].id);

                int insertCounterVec = 0;

                for (int k = VOrder[i].initialPos; k <= VOrder[i].finalPos; k++) {

                    if (jobsIn.find(solution[k].id) == jobsIn.end()) {

                        while (insertCounterVec < newOrder.size() && newOrder[insertCounterVec] != -1)
                            insertCounterVec++;

                        newOrder[insertCounterVec] = solution[k].id;
                        jobsIn.insert(solution[k].id);
                        insertCounterVec++;
                    }
                }

                int CC = 0; // Copy Counter

                for (int k = VOrder[i].initialPos; k <= VOrder[i].finalPos; k++) {
                    solution[k].id = newOrder[CC];
                    CC++;
                }

                double newObj = objFunction2(solution, VOrder, false, K, N, t, w, P, d, F, i);

                if ((newObj < bestObj) && (newObj - bestObj) < bestImprove) {

                    bestImprove = (newObj - bestObj);
                    improved = true;
                    bestConfig = solution;

                    //If it is a First Improvement call
                    if (bestOrFirst == 'F')
                        return true;
                }

                CC = 0;

                // Reset to default solution.
                for (int k = VOrder[i].initialPos; k <= VOrder[i].finalPos; k++) {
                    solution[k].id = original[CC];
                    CC++;
                }

                jobsNb++;
                jobPosition++;
            }
        }
    }

    if (improved) {
        solution = bestConfig;
        return true;
    }
    else {
        return false;
    }
}

/*  Neighborhood 4: Inserting jobs in positions inside OTHERS vehicles */
bool nbhood4(char bestOrFirst, vector<data>& solution, vector<vehicleLoaded>& VOrder, int N, int K, const vector<double>& w,
    const vector<int>& P, const vector<vector<int> >& t, const vector<int>& F, const vector<int>& d, const vector<int>& Q, const vector<int>& s)
{

    //Calculating the current car volume
    vector<int> currCap(K, 0);
    vector<bool> carInUse(K, false);

    double bestObj = objFunction2(solution, VOrder, false, K, N, t, w, P, d, F, -1);

    double iniObj = bestObj;
    double improved = false;
    vector<data> bestConfig = solution;
    vector<vehicleLoaded> bestVOrder = VOrder;

    for (int i = 0; i < VOrder.size(); i++) {

        int whichVehicle = VOrder[i].id;
        carInUse[whichVehicle] = true;

        for (int j = VOrder[i].initialPos; j <= VOrder[i].finalPos; j++)
            currCap[whichVehicle] += s[solution[j].id];
    }

    for (int i = 0; i < VOrder.size(); i++) {

        int carsBeforeMe = i;
        int carsAfterMe = VOrder.size() - i - 1;

        int tempBefore = carsBeforeMe;

        while (tempBefore > 0) {

            --tempBefore;

            //For each job in vehicle...
            for (int j = VOrder[i].initialPos; j <= VOrder[i].finalPos; j++) {

                //IF the car before me can handle carrying this job, we calculate the benefit int puting the job on it
                if ((currCap[VOrder[tempBefore].id] + s[solution[j].id]) <= (Q[VOrder[tempBefore].id])) {

                    int range = VOrder[tempBefore].finalPos - VOrder[tempBefore].initialPos + 2;
                    int idCarInsert = VOrder[tempBefore].id;

                    //Trying to put in each space avaiable on the other car
                    for (int z = 0; z < range; z++) {

                        vector<int> newcarried(range, -1);

                        newcarried[z] = solution[j].id;
                        int copy = VOrder[tempBefore].initialPos;
                        for (int u = 0; u < range; u++) {
                            if (z == u)
                                continue;
                            newcarried[u] = solution[copy++].id;
                        }

                        vector<data> newSolution(solution.size());

                        int copyConfig = 0;
                        int contInsertNew = 0;

                        while (copyConfig < solution.size()) {

                            if (solution[copyConfig].job && solution[copyConfig].id == solution[j].id) {
                                //Job is moved
                                copyConfig++;
                                continue;
                            }

                            if (!solution[copyConfig].job && solution[copyConfig].id == idCarInsert) {

                                newSolution[contInsertNew] = solution[copyConfig];

                                copyConfig++;
                                contInsertNew++;

                                int insert = range;
                                int contUpward = 0;

                                while (insert > 0) {
                                    newSolution[contInsertNew].job = true;
                                    newSolution[contInsertNew].id = newcarried[contUpward];

                                    contUpward++;
                                    contInsertNew++;
                                    copyConfig++;

                                    insert--;
                                }

                                copyConfig--; //Deleting the extra job inserted
                                continue;
                            }

                            newSolution[contInsertNew] = solution[copyConfig];
                            contInsertNew++;
                            copyConfig++;
                        }

                        vector<vehicleLoaded> new_VOrder = getVOrder(newSolution, K); //Generating info about vehicle transportation

                        double newObj = objFunction2(newSolution, new_VOrder, false, K, N, t, w, P, d, F, -1);

                        if (newObj < bestObj && newObj != 0) {

                            //If it is a call for First Improvement
                            if (bestOrFirst == 'F') {
                                solution = newSolution;
                                VOrder = new_VOrder;
                                return true;
                            }
                            bestConfig = newSolution;
                            bestVOrder = new_VOrder;

                            improved = true;
                            bestObj = newObj;
                        }
                    }
                }
            }
        }

        int carAhead = i + 1;

        while (carAhead < VOrder.size()) {

            //For each job in vehicle ...
            for (int j = VOrder[i].initialPos; j <= VOrder[i].finalPos; j++) {

                //IF the car before me can handle carrying this job, we calculate the benefit int puting the job on it
                if ((currCap[VOrder[carAhead].id] + s[solution[j].id]) <= (Q[VOrder[carAhead].id])) {

                    int range = VOrder[carAhead].finalPos - VOrder[carAhead].initialPos + 2;
                    int idCarInsert = VOrder[carAhead].id;

                    //Trying to put in each space avaiable on the other car
                    for (int z = 0; z < range; z++) {

                        vector<int> newcarried(range, -1);

                        newcarried[z] = solution[j].id;
                        int copy = VOrder[carAhead].initialPos;

                        for (int u = 0; u < range; u++) {
                            if (z == u)
                                continue;
                            newcarried[u] = solution[copy++].id;
                        }

                        vector<data> newSolution(solution.size());

                        int copyConfig = 0;
                        int contInsertNew = 0;

                        while (copyConfig < solution.size()) {

                            if (solution[copyConfig].job && solution[copyConfig].id == solution[j].id) {
                                //Job is moved
                                copyConfig++;
                                continue;
                            }

                            if (!solution[copyConfig].job && solution[copyConfig].id == idCarInsert) {

                                newSolution[contInsertNew] = solution[copyConfig];

                                copyConfig++;
                                contInsertNew++;

                                int insert = range;
                                int contUpward = 0;

                                while (insert > 0) {

                                    newSolution[contInsertNew].job = true;
                                    newSolution[contInsertNew].id = newcarried[contUpward];

                                    contUpward++;
                                    contInsertNew++;
                                    copyConfig++;

                                    insert--;
                                }

                                copyConfig--; //Deleting inserted extra job
                                continue;
                            }

                            if (contInsertNew > solution.size() || copyConfig > solution.size())
                                break;

                            newSolution[contInsertNew] = solution[copyConfig];

                            contInsertNew++;
                            copyConfig++;
                        }

                        vector<vehicleLoaded> new_VOrder = getVOrder(newSolution, K); //Generating info about vehicle transportation

                        double newObj = objFunction2(newSolution, new_VOrder, false, K, N, t, w, P, d, F, -1);

                        if (newObj < bestObj && newObj != 0) {

                            //If it is a call for First Improvement
                            if (bestOrFirst == 'F') {
                                solution = newSolution;
                                VOrder = new_VOrder;
                                return true;
                            }

                            bestConfig = newSolution;
                            bestVOrder = new_VOrder;

                            improved = true;
                            bestObj = newObj;
                        }
                    }
                }
            }
            ++carAhead;
        }
    }

    for (int j = 0; j < K; j++) {

        // Inserting in free space vehicles
        if (!carInUse[j]) {

            for (int i = 0; i < VOrder.size(); i++) {

                //For each task in this car...
                for (int k = VOrder[i].initialPos; k <= VOrder[i].finalPos; k++) {

                    if (Q[j] >= s[solution[k].id]) {

                        vector<data> newSolution(solution.size());

                        int insert = 0;
                        for (int z = 0; z < solution.size(); z++) {

                            if (!solution[z].job && solution[z].id == j) { //The new car to insert each job

                                newSolution[insert++] = solution[z];
                                newSolution[insert].job = true;
                                newSolution[insert].id = solution[k].id;
                                ++insert;
                            }
                            else if (solution[z].job && solution[z].id == solution[k].id) {
                                continue;
                            }
                            else {
                                newSolution[insert++] = solution[z];
                            }
                        }

                        //Checking if OBJ function improves with this change
                        vector<vehicleLoaded> new_VOrder = getVOrder(newSolution, K); //Generating info about vehicle transportation

                        double newObj = objFunction2(newSolution, new_VOrder, false, K, N, t, w, P, d, F, -1);

                        if (newObj < bestObj) {

                            //If it is a call for First Improvement
                            if (bestOrFirst == 'F') {
                                solution = newSolution;
                                VOrder = new_VOrder;
                                return true;
                            }

                            bestConfig = newSolution;
                            bestVOrder = new_VOrder;

                            improved = true;
                            bestObj = newObj;
                        }
                    }
                }
            }
        }
    }

    if (bestObj < iniObj) {

        solution = bestConfig;
        VOrder = bestVOrder; //Generating info about vehicle transportation

        return true;
    }
    else {
        return false;
    }
}

/*  Neighborhood 5: Swap entire vehicle (with all its jobs) with another ones */
bool nbhood5(char bestOrFirst, vector<data>& config, vector<vehicleLoaded>& VOrder, int N, int K, const vector<double>& w,
    const vector<int>& P, const vector<vector<int> >& t, const vector<int>& F, const vector<int>& d)
{

    double iniObj = objFunction2(config, VOrder, false, K, N, t, w, P, d, F, -1);
    bool improvedBest = false;

    vector<data> bestConfig;
    vector<vehicleLoaded> bestVOrder;

    for (int i = 0; i < VOrder.size() - 1; i++) {

        int nbJobs = VOrder[i].finalPos - VOrder[i].initialPos + 1;

        vector<data> tempOut(nbJobs + 1);

        // Copying vehicle and its job solution
        tempOut[0] = config[VOrder[i].initialPos - 1];

        for (int i1 = 1; i1 <= nbJobs; i1++)
            tempOut[i1] = config[VOrder[i].initialPos + (i1 - 1)];

        int vehicleOut = config[VOrder[i].initialPos - 1].id;

        for (int j = i + 1; j < VOrder.size(); j++) {

            int nbJobsIn = VOrder[j].finalPos - VOrder[j].initialPos + 1;

            vector<data> tempIn(nbJobsIn + 1);

            tempIn[0] = config[VOrder[j].initialPos - 1];

            for (int i1 = 1; i1 <= nbJobsIn; i1++)
                tempIn[i1] = config[VOrder[j].initialPos + (i1 - 1)];

            int vehicleIn = config[VOrder[j].initialPos - 1].id;

            // Swapping vehicles
            int copy = 0;
            int insert = 0;

            vector<data> newSolution(config.size());

            while (copy < config.size()) {

                if (!config[copy].job && config[copy].id != vehicleOut && config[copy].id != vehicleIn) {

                    int next = copy + 1;

                    newSolution[insert++] = config[copy]; //copy vehicle

                    while (next < config.size() && config[next].job) {
                        newSolution[insert] = config[next];
                        next++;
                        insert++;
                    }

                    copy = next;
                }
                else if (!config[copy].job && config[copy].id == vehicleOut) {

                    int cont = 0;

                    while (cont <= nbJobsIn) {
                        newSolution[insert] = tempIn[cont];
                        insert++;
                        cont++;
                    }

                    copy += nbJobs + 1;
                }
                else if (!config[copy].job && config[copy].id == vehicleIn) {

                    int cont = 0;
                    while (cont <= nbJobs) {
                        newSolution[insert] = tempOut[cont];
                        insert++;
                        cont++;
                    }
                    copy += nbJobsIn + 1;
                }
            }

            // Catching new Obj Function value
            vector<vehicleLoaded> new_VOrder = getVOrder(newSolution, K);

            double newObj = objFunction2(newSolution, new_VOrder, false, K, N, t, w, P, d, F, -1);

            if (newObj < iniObj) {
                if (bestOrFirst == 'F') { // First Improvement

                    VOrder = new_VOrder;
                    config = newSolution;

                    return true;
                }
                else { // Best Improvement

                    iniObj = newObj;
                    bestConfig = newSolution;
                    bestVOrder = new_VOrder;

                    improvedBest = true;
                }
            }
        }
    }

    if (improvedBest && bestOrFirst == 'B') {

        config = bestConfig;
        VOrder = bestVOrder;

        return true;
    }

    return false;
}

/*  Neighborhood 6: Insert entire vehicle (with all its jobs) in  another positions, except for the immediately before */
bool nbhood6(char bestOrFirst, vector<data>& config, vector<vehicleLoaded>& VOrder, int N, int K, const vector<double>& w,
    const vector<int>& P, const vector<vector<int> >& t, const vector<int>& F, const vector<int>& d)
{

    double iniObj = objFunction2(config, VOrder, false, K, N, t, w, P, d, F, -1);
    bool improvedBest = false;

    //Caution, data not initialized
    vector<data> bestConfig;
    vector<vehicleLoaded> bestVOrder;

    //Saving original car positions in the received config
    vector<int> carPos(K, -1);

    int contcar = 0;

    for (int i = 0; i < config.size(); i++) {
        if (!config[i].job) {
            carPos[contcar] = config[i].id;
            contcar++;
        }
    }

    unordered_set<int> inUse; //vehicles in use
    unordered_map<int, int> locate; //first == vehicle, second == position in array

    for (int i = 0; i < VOrder.size(); i++) {
        inUse.insert(VOrder[i].id);
        locate[VOrder[i].id] = VOrder[i].initialPos;
    }

    //  Inserting using Forward Swap cars with Adjacent Vehicles
    for (int i = 0; i < K - 1; i++) {

        //Inserting in vehicles in use.
        if (inUse.find(carPos[i]) == inUse.end())
            continue; //not in use

        vector<int> newCarPos = carPos;

        for (int j = i + 1; j < K; j++) {

            int carJ = newCarPos[j];
            swap(newCarPos[i], newCarPos[j]);

            //Generating this solution if car j is in use!
            if (inUse.find(carJ) == inUse.end())
                continue; //continue swapping, because this swap will not increase the obj function

            //Generating new solution
            vector<data> newSolution(config.size());

            int insert = 0;

            for (int k = 0; k < K; k++) {

                if (inUse.find(newCarPos[k]) == inUse.end()) { // Case vehicle NOT in use

                    newSolution[insert].job = false;
                    newSolution[insert++].id = newCarPos[k];
                }
                else { // Case vehicle IS in use

                    newSolution[insert].job = false;
                    newSolution[insert++].id = newCarPos[k];

                    int copy = locate[newCarPos[k]];

                    while (copy < config.size() && config[copy].job) {

                        newSolution[insert].job = true;
                        newSolution[insert++].id = config[copy].id;

                        copy++;
                    }
                }
            }

            //Checking new OBJ function value
            vector<vehicleLoaded> new_VOrder = getVOrder(newSolution, K);
            double newObj = objFunction2(newSolution, new_VOrder, false, K, N, t, w, P, d, F, -1);
            
            if (newObj < iniObj) {

                if (bestOrFirst == 'F') {
                    VOrder = new_VOrder;
                    config = newSolution;

                    return true;
                }
                else {
                    iniObj = newObj;

                    bestConfig = newSolution;
                    bestVOrder = new_VOrder;

                    improvedBest = true;
                }
            }
        }
    }

    //Swap backwards except for the immediatly before.
    for (int i = K - 1; i > 0; i--) {

        //Vehicle NOT IN USE, ignore it
        if (inUse.find(carPos[i]) == inUse.end())
            continue;

        vector<int> newCarPos = carPos;

        for (int j = i - 1; j >= 0; j--) {

            int carJ = newCarPos[j];
            swap(newCarPos[i], newCarPos[j]);

            //Generating this solution if car j is in use!
            if (j == (i - 1) || inUse.find(carJ) == inUse.end())
                continue;

            //Generating new solution
            vector<data> newSolution(config.size());
            int insert = 0;

            for (int k = 0; k < K; k++) {

                if (inUse.find(newCarPos[k]) == inUse.end()) { // Vehicle IN use

                    newSolution[insert].job = false;
                    newSolution[insert++].id = newCarPos[k];
                }
                else { // Vehicle NOT in use

                    newSolution[insert].job = false;
                    newSolution[insert++].id = newCarPos[k];

                    int copy = locate[newCarPos[k]];

                    while (copy < config.size() && config[copy].job) {

                        newSolution[insert].job = true;
                        newSolution[insert++].id = config[copy].id;

                        copy++;
                    }
                }
            }

            //Checking new config OBJ!
            vector<vehicleLoaded> new_VOrder = getVOrder(newSolution, K);
            double newObj = objFunction2(newSolution, new_VOrder, false, K, N, t, w, P, d, F, -1);

            if (newObj < iniObj) {

                if (bestOrFirst == 'F') {

                    VOrder = new_VOrder;
                    config = newSolution;

                    return true;
                }
                else {
                    iniObj = newObj;

                    bestConfig = newSolution;
                    bestVOrder = new_VOrder;

                    improvedBest = true;
                }
            }
        }
    }

    if (improvedBest && bestOrFirst == 'B') {

        config = bestConfig;
        VOrder = bestVOrder;

        return true;
    }

    return false;
}

/*  Neighborhood 7: 2-OPT */
bool nbhood7(char bestOrFirst, vector<data>& config, vector<vehicleLoaded>& VOrder, int N, int K, const vector<double>& w,
    const vector<int>& P, const vector<vector<int> >& t, const vector<int>& F, const vector<int>& d, const vector<int>& Q, const vector<int>& s)
{

    double initialObj = objFunction2(config, VOrder, false, K, N, t, w, P, d, F, -1);
    double bestObj = initialObj;
    double biggestImprove = 0;
    vector<data> bestConfig;
    bool improved = false;

    vector<int> carUsed;

    //The position of each car 'i' will be referred in edgesByCar[i]
    vector<vector<pair<int, int>>> edgesByCar(K);
    vector<int> jobPos(N);

    for (int i = 0; i < config.size(); i++) {
        if (config[i].job)
            jobPos[config[i].id] = i;
    }

    //Generating edges.
    for (int i = 0; i < VOrder.size(); i++) {

        int beg = VOrder[i].initialPos;
        int end = VOrder[i].finalPos;

        //Checking if there is enough edges to do 2-opt in each car.
        if ((end - beg) <= 2)
            continue;

        int carId = VOrder[i].id;
        carUsed.push_back(carId);

        for (int j = beg; j < end; j++)
            edgesByCar[carId].push_back({ config[j].id, config[j + 1].id });
    }

    //Randomize vehicles.
    random_shuffle(carUsed.begin(), carUsed.end());

    for (int i = 0; i < carUsed.size(); i++) {

        int car = carUsed[i];

        random_shuffle(edgesByCar[car].begin(), edgesByCar[car].end());

        for (int j = 0; j < edgesByCar[car].size() - 1; j++) {

            for (int k = j + 1; k < edgesByCar[car].size(); k++) {

                set<int> points;
                int p1, p2, p3, p4;

                p1 = edgesByCar[car][j].first;
                p2 = edgesByCar[car][j].second;
                p3 = edgesByCar[car][k].first;
                p4 = edgesByCar[car][k].second;

                points.insert(p1);
                points.insert(p2);
                points.insert(p3);
                points.insert(p4);

                if (points.size() < 4) // ADJACENT EDGES aren't valid.
                    continue;

                vector<data> newSolution = config;

                // Do 2-OPT Changes.
                swap(config[jobPos[p2]], config[jobPos[p3]]);

                // Checking new OBJ Function value.
                double newObj = objFunction2(config, VOrder, false, K, N, t, w, P, d, F, -1);

                if (newObj < bestObj) {

                    bestObj = newObj;
                    bestConfig = config;
                    improved = true;

                    if (bestOrFirst == 'F')
                        return true;
                }
                // Undo 2-OPT Changes.
                swap(config[jobPos[p2]], config[jobPos[p3]]);
            }
        }
    }

    if (bestOrFirst == 'F') {

        if (!improved)
            return false;
    }
    else { // Best Improvement.

        if (improved) {
            config = bestConfig;
            return true;
        }
        else {
            return false;
        }
    }
    return false;
}

//pair< ValueObjFunction, validSolution>
pair<double, vector<data> > RVND_Custom(bool reuse, int N, int K, const vector<double>& w, const vector<int>& P, const vector<vector<int> >& t,
    const vector<int>& F, const vector<int>& d, const vector<int>& Q, const vector<int>& s, vector<data>& initialConfig)
{

    if (!reuse)
        initialConfig = generateValidRandomConfig(N, K, Q, s);
    
    vector<int> interRoute = { 2, 4, 5, 6 };
    random_shuffle(interRoute.begin(), interRoute.end());
    int it = 0;

    vector<vehicleLoaded> VOrder;
    double bestObj = objFunction2(initialConfig, VOrder, true, K, N, t, w, P, d, F, -1);

    bool generalImprove = false;

    //Caution Unitialized data
    vector<data> bestConfig;

    while (it < 4) {

        int neighbor = interRoute[it];
        bool improved = false;

        switch (neighbor) {
        case 2:
            improved = nbhood2('B', initialConfig, VOrder, N, K, w, P, t, F, d, Q, s);
        case 4:
            improved = nbhood4('B', initialConfig, VOrder, N, K, w, P, t, F, d, Q, s);
        case 5:
            improved = nbhood5('B', initialConfig, VOrder, N, K, w, P, t, F, d);
        case 6:
            improved = nbhood6('B', initialConfig, VOrder, N, K, w, P, t, F, d);
        }

        if (improved) {
            generalImprove = true;
            while (true) { //intra route
                if (nbhood1('B', initialConfig, VOrder, N, K, w, P, t, F, d)) {
                    continue;
                }
                if (nbhood3('B', initialConfig, VOrder, N, K, w, P, t, F, d)) {
                    continue;
                }
                if (nbhood7('B', initialConfig, VOrder, N, K, w, P, t, F, d, Q, s)) {
                    continue;
                }
                break;
            }

            bestConfig = initialConfig;
            it = 0;
            random_shuffle(interRoute.begin(), interRoute.end());
        }
        else {
            it++;
        }
    }

    if (generalImprove)
        initialConfig = bestConfig;

    double result = objFunction2(initialConfig, VOrder, true, K, N, t, w, P, d, F, -1);

    return make_pair(result, initialConfig);
}

//pair< ValueObjFunction, validSolution>
pair<double, vector<data> > RVND(bool reuse, int N, int K, const vector<double>& w, const vector<int>& P, const vector<vector<int> >& t,
    const vector<int>& F, const vector<int>& d, const vector<int>& Q, const vector<int>& s, vector<data>& initialConfig)
{

    //Checking if it is needed to generate a new one or if the current solution will be used
    if (!reuse) {
        initialConfig = randomConfigSequential(N, K, Q, s);
    }

    vector<vehicleLoaded> VOrder = getVOrder(initialConfig, K); //Generating info about vehicle transportation
    vector<int> startVehicleTime(K, 0); //Calculating Delivery time(D) and Starting time(Sk)
    vector<int> deliveryTime = calculatingDeliveryTime(initialConfig, startVehicleTime, t, P, VOrder, N);
    vector<int> jobTardiness = calculatingJobTardiness(deliveryTime, N, d); //Generating Job Tardiness (T) of each job (O(N))

    double iniObj = objFunction(VOrder, initialConfig, K, N, t, w, jobTardiness, F);
    assert( iniObj - objFunction2(initialConfig, VOrder, true, K, N, t, w, P, d, F, -1) < 0.001);

    vector<int> neighbors = { 1, 2, 3, 4, 5, 6, 7 };

    random_shuffle(neighbors.begin(), neighbors.end());

    int it = 0;

    while (it < 7) {

        int whichNeighbor = neighbors[it];

        bool improved = false;

        if (whichNeighbor == 1) {
            improved = nbhood1('B', initialConfig, VOrder, N, K, w, P, t, F, d);
        }
        else if (whichNeighbor == 2) {
            improved = nbhood2('B', initialConfig, VOrder, N, K, w, P, t, F, d, Q, s);
        }
        else if (whichNeighbor == 3) {
            improved = nbhood3('B', initialConfig, VOrder, N, K, w, P, t, F, d);
        }
        else if (whichNeighbor == 4) {
            improved = nbhood4('B', initialConfig, VOrder, N, K, w, P, t, F, d, Q, s);
        }
        else if (whichNeighbor == 5) {
            improved = nbhood5('B', initialConfig, VOrder, N, K, w, P, t, F, d);
        }
        else if (whichNeighbor == 6) {
            improved = nbhood6('B', initialConfig, VOrder, N, K, w, P, t, F, d);
        }
        else if (whichNeighbor == 7) {
            improved = nbhood7('B', initialConfig, VOrder, N, K, w, P, t, F, d, Q, s);
        }

        if (improved) {
            it = 0;
            random_shuffle(neighbors.begin(), neighbors.end());
        }
        else {
            it++;
        }
    }

    double finalObj = objFunction2(initialConfig, VOrder, true, K, N, t, w, P, d, F, -1);

    return make_pair(finalObj, initialConfig);
}

#endif