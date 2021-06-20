#include "skiplist.h"
#define FILE_PATH "dumpFile"

int main() {
    SkipList<std::string, std::string> skipList(6);
    skipList.insert_element("name", "ivan"); 
	skipList.insert_element("age", "18"); 
	skipList.insert_element("weather", "sunny"); 
	skipList.insert_element("temperature", "34 degrees centigrade");
    skipList.insert_element("address", "home");

    std::cout << "skipList size:" << skipList.size() << std::endl;

    skipList.dump_file();

    skipList.search_element("name");

    return 0;
}