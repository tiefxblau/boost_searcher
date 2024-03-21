#include "cpp-httplib/httplib.h"
#include "Searcher.hpp"
#include <iostream>
#include <string>
#include "Log.hpp"

const std::string root_path = "./wwwroot";

int main()
{
    ns_searcher::Searcher searcher;

    httplib::Server svr;
    svr.set_base_dir(root_path.c_str());
    svr.Get("/s", [&searcher](const httplib::Request& req, httplib::Response& resp){
        if (!req.has_param("words"))
        {
            resp.set_content("必须要有搜索关键字", "text/plain; charset=utf-8");
            return;
        }

        std::string query = req.get_param_value("words");
        // std::cout << "用户正在搜索: " << query << std::endl;
        Log(NORMAL, "用户正在搜索: " + query);

        std::string json_string;
        searcher.Search(query, &json_string);
        resp.set_content(json_string, "application/json");
    });

    svr.listen("0.0.0.0", 8080);
    return 0;
}