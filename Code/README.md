# C++ Codes

## Folder: ILS-RVND and GA-LS

In this folder you'll find the following **implementations**:

*"Metaheuristics.h"*:
- ILS-RVND-1.
- ILS-RVND-2.
- GA-LS.

*"LocalSearch.h":*
- RVND.
- Neighborhoods.

The solution representation used in this folder consists of an array of vehicles and jobs, a position is called "data". For instance, consider the following data array:

**V1** J2 J3 **V2** J1 J4 J5

It means that vehicle 1 carries jobs J2 and J3, and vehicle 2 carries jobs J1, J4 and J5. 
Also, the vehicle order represents which vehicle will left the factory first. In this case, vehicle 1 comes out first and then vehicle 2.

## ILS-RVND-1

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


## GA2 and GA-Paper

...
