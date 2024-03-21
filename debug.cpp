#include <iostream>
#include <string>
#include "Searcher.hpp"

int main()
{
    ns_searcher::Searcher searcher;
    std::string query;
    
    while (true)
    {
        std::string json_string;
        std::cout << "Please Enter: ";
        std::getline(std::cin, query);
        searcher.Search(query, &json_string);

        // std::cout << json_string << std::endl;
    }

    return 0;
}