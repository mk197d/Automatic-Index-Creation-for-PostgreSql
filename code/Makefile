all: main.cpp helper.cpp index.cpp
	g++ -std=c++17 -Wno-unknown-attributes -o run main.cpp helper.cpp index.cpp -I/opt/homebrew/include -L/opt/homebrew/lib -lpqxx -lpq -lreadline
clean:
	rm -f run