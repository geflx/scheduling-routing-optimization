default:
	g++ main.cpp -O3 -fopenmp

test:
	reset
	g++ main.cpp -g
	valgrind --leak-check=full ./a.out

clear:
	rm -f *.out
	rm -f *.txt
	rm -f *.csv

