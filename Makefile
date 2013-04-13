all:
	clang++ -std=c++11 -stdlib=libc++ -o test test.cpp
clean:
	rm -rf test
