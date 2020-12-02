# How to compile

```
makefile && ./a.out
```

## ILS-RVND-1

In this work, this hybrid ILS and RVND metaheuristic was the best developed method to solve the integrated problem. This metaheuristic is based on consecutives pertubations and local searches over solutions, returning the best solution found in the end.__
It's pseudocode is shown below:__


```
S* = INF
for i = 0 to maxIter

    S = GreedySolution()
    S' = RVND(S)

    for j = 0 to maxIterILS
        if j!=0
            S' = RVND(S')
        if f(S') < f(S*)
            S* = S'
            j = 0
        S' = Perturb(S')
return S*
```

## GA2 and GA-Paper

Recreated Tamannaei and Rasti-Barzoki's Genetic Algorithm and attempted to improve it using heuristics.__ Compared to the previous developed metaheuristics, no significant improvement was found.
