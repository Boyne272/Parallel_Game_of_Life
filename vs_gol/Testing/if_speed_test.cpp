#include <iostream>
#include <chrono>
using namespace std;


	// here I am timing what takes less time, if or 4 bool additions
int main() {

	int a = 1;
	bool b = 0;

	auto start = std::chrono::system_clock::now();
	for (int i = 0; i < 10000000; i++)
		a += b;
	auto end = std::chrono::system_clock::now();
	cout << "adding bool " << (end - start).count() << "ms\n";

	start = std::chrono::system_clock::now();
	for (int i = 0; i < 10000000 / 4; i++)
		if (b != 0)
			a += b;
	end = std::chrono::system_clock::now();
	cout << "if statment " << (end - start).count() << "ms\n";

	system("pause");
}