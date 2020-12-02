# Integrated Machine Scheduling and Heterogeneous Fleet Vehicle Routing Problem :calendar::truck:

This problem consists in scheduling _N_ jobs inside a linear machine with _K_ available vehicles composing a Heterogeneous Fleet. This is a new variation of the Integrated Problem in literature, which has vehicles with **different capacities** and jobs with **different sizes** simultaneosly. The main objective of this problem is to minimize the total delivery time, total vehicle use costs and total tardiness weight.

Each job _i_ has the following atributes:

- _Pi_ → Processing Time
- _wi_ → Penalty weight
- _di_ → Due date
- _si_ → Size

In a similar way, each vehicle _k_ has the following atributes:

- _Fk_ → Cost
- _Qk_ → Capacity

The problem's objective function is defined as follows, being composed by three parts:
<p align="center">
<img src="Results/CPLEX Model/statistics/obj_function.png" width=540 height=80>
</p>

(1) Minimize total delivery time, (2) Minimize vehicle use costs and (3) Minimize total tardiness weight.

## Metaheuristics :pencil:

The *main* metaheuristics developed for this problem were two hybrids _Iterated Local Search (ILS) - RVND_ and one _Genetic Algorithm - RVND_. In the local search were implemented **seven** different neighborhoods subdivided in two groups: Inter-Route and Intra-Route neighborhoods.

Find more information at _Code folder_.

## MILP Model :pencil:

A mixed-integer linear programming model was developed to solve the given problem for small instances and generate insight for small instances. By doing this it is possible to find optimal solutions and then compare with new metaheuristics.

The model was implemented in C++ using the **CPLEX Concert** library. It is also possible to **reuse** previous solutions using the _MIPStart Technique
_ to start in a \"checkpoint\".
Find more information at _CPLEX Model folder_. 

## Results :bar_chart:

In small scale instances, all three metaheuristics outperformed CPLEX Solver, being ILS-RVND-1 and GA-LS statistically equivalent:

<p align="center">
<img src="Results/Minitab and Graphics/Small - White Background/general_avg_rpd_int.png" width=540 height=390>
</p>

In large scale instances, the best metaheuristic was ILS-RVND-1, obtaining better results than others and executing in less time:

<p align="center">
<img src="Results/Minitab and Graphics/Large - White Background/avg_rpd_all_algos.png" width=540 height=390>
</p>
