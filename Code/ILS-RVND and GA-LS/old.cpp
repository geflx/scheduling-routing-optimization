
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
