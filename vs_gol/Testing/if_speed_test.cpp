#include <iostream>
#include <chrono>
using namespace std;


	// here I am timing what takes less time, if or 4 bool additions
//int main() {
void test_1() {

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


bool compact(int x, int a, int b) {
	return !(bool)((2 * x - b - a) / (b - a));
}


// here I am timing what takes less time, if or 4 bool additions
int main() {

	int a = 0;

	auto start = std::chrono::system_clock::now();
	for (int i = 0; i < 10000000; i++)
		a += compact(5, 4, 6);
	auto end = std::chrono::system_clock::now();
	cout << "compact " << (end - start).count() << "ms\n";

	start = std::chrono::system_clock::now();
	for (int i = 0; i < 10000000; i++)
		if ((5 > 4) && (6 > 5))
			a += 1;
	end = std::chrono::system_clock::now();
	cout << "if " << (end - start).count() << "ms\n";

	system("pause");
}