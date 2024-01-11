This is a simulation of a restaurant. The restaurant requires careful coordination of resources, which are the tables in the restaurant, the cooks available to cook the order, and the machines available to cook the food (we assume that there is no contention on other possible resources, like servers to take orders and serve the orders).

In this project, I wrote a simulation for a restaurant. The simulation has four sets of parameters - the number of eaters (diners) that wish to enter the restaurant, the times they
approach the restaurant and what they order, the number of tables in the restaurant (there can only be as many eaters as there are tables at a time; other eaters must wait for a free
table before placing their order), and the number of cooks in the kitchen that process orders. Eaters in the restaurant place their orders when they get seated. These orders are then handled by available cooks. Each cook handles one order at a time. A cook handles an order by using machines to cook the food items.

To simplify the simulation, assumptions->
There are only three types of food served by the restaurant - a Buckeye Burger,Brutus Fries, and Coke. 
Each person entering the restaurant occupies one table and orders one or more burgers, zero or more orders of fries, and zero or one glass of coke. 
The cook needs to use the burger machine for 5 minutes to prepare each burger, fries machine for 3 minutes for one order of fries, and the soda machine for 1 minute to fill a glass with coke. 
The cook can use at most one of these three machines at any given time. There is only one machine of each type in the restaurant.
Once the food (all items at the same time) are brought to a diner's table, they take exactly 30 minutes to finish eating them, and then leave the restaurant, making the table available for another diner (immediately).

Usage:
1. Simply run `make` inside the project root directory. 
2. Create a file named `input.txt` which takes in the input for this program.
3. Run `make run`  inside project root directory to generate the output of the program inside `output.txt`.

Note: Program simulation takes time and you can check the progress of the program inside `output.txt`.



Assumptions:

This program only takes orders of diners which arrive before or on 120 seconds mark. Any diner who arrives at the restaurant after 120 seconds mark will be discarded. Added the input and output for the first testcase.
