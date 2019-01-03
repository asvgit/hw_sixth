#include <iostream>
#include <cassert>
#include <sstream>

int main(int argc, char *argv[]) {
	try {
		assert(argc == 2);
		const int bulk_size = [&]() {
			std::stringstream ss(argv[1]);
			int n;
			ss >> n;
			return n;
		}();

		std::string bulk;
		std::string line;
		int count = 0;
		while (std::getline(std::cin, line)) {
			if (bulk.empty())
				bulk = line;
			else
				bulk += ", " + line;
			if (++count == bulk_size) {
				std::cout << "bulk: " << bulk << std::endl;
				bulk.clear();
				count = 0;
			}
		}

		// std::cout << "hw_sixth" << std::endl;
	} catch(const std::exception &e) {
		std::cerr << e.what() << std::endl;
	}
	return 0;
}
