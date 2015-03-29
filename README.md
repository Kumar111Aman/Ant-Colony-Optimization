# Ant-Colony-Optimization
Virtual Machine Placement Problem project under Asst. Prof. Amgoth Tarachand, Indian School of Mines, Dhanbad.


Input type:

1.There must exist a solution of the problem.
2.All servers must have capacity of atleast twice of maximum capacity of largest vm requirement of each objective.
3.Sum of CPU demands of all vm's must be less than sum of Threshold CPU of all servers.
4.Sum of MEM demands of all vm's must be less than sum of Threshold MEM of all servers.

NOTE:
->Max size is set to 50, can be increased upto 100

->Adjust the number of ants and number of iterations according to size of input, otherwise it runs out of
memory and shows runtime error.

->For number of vm=100
number of ants=4 and number of iterations=1

->For number of vm=50
number of ants=15 and number of iterations=5
