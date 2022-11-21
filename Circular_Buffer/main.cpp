#include "Circular_Buffer.h"

void test_case_1() {

    CircularBuffer<int> cb(3);
    cb.add(1);
    cb.add(2);
    cb.add(3);

    std::cout << "is full: " << cb.is_full() << std::endl;
    cb.add(4);
    
    std::cout << cb[0] << std::endl; 
    std::cout << cb[1] << std::endl; 
    std::cout << cb[2] << std::endl; 

}

int main() {
    
    test_case_1();
    return 0;
}