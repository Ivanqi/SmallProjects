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
    skipList.search_element("weather");

    skipList.display_list();

    std::cout << "\n-----------  delete_element --------------" << std::endl;
    skipList.delete_element("temperature");
    skipList.delete_element("address");

    std::cout << "\nskipList size:" << skipList.size() << std::endl;

    skipList.display_list();
    return 0;
}