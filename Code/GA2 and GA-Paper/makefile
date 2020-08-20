default:
	reset
	g++ main.cpp -O3

test:
	reset
	g++ main.cpp -g
	valgrind --leak-check=full ./a.out

clear:
	rm -f *.out
	rm -f *.txt

