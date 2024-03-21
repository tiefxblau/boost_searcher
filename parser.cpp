#include <iostream>
#include <string>
#include <vector>
#include <utility>
#include <boost/filesystem.hpp>
#include "Util.hpp"

const std::string src = "./data/input/";
const std::string output = "./data/raw_html/raw";

typedef struct DocInfo
{
    std::string title;  //标题
    std::string content; //内容
    std::string url;    //url

    void Show() // for debug
    {
        std::cout << "title: " << title << std::endl;
        std::cout << "content: " << content << std::endl;
        std::cout << "url: " << url << std::endl;
    }
} DocInfo_t;

bool EnumFile(const std::string& src, std::vector<std::string>* file_list);
bool ParseHtml(const std::vector<std::string>& file_list, std::vector<DocInfo_t>* results);
bool SaveHtml(const std::vector<DocInfo_t>& results, const std::string& file_list);

int main()
{
    std::vector<std::string> file_list;
    if (!EnumFile(src, &file_list)) //将html文件放入file_list
    {
        // std::cerr << "enum error" << std::endl;
        Log(FATAL, "enum file error");
        return 1;
    }
    // std::cout << "enum file success" << std::endl;
    Log(NORMAL, "enum file success");

    std::vector<DocInfo_t> results;
    if (!ParseHtml(file_list, &results))    //解析htlm文件
    {
        // std::cerr << "parse error" << std::endl;
        Log(FATAL, "parse file error");
        return 2;
    }
    // std::cout << "parse file success" << std::endl;
    Log(NORMAL, "parse file success");

    if (!SaveHtml(results, output)) //保存解析的文件
    {
        // std::cerr << "save error" << std::endl;
        Log(FATAL, "save file error");
        return 3;
    }
    // std::cout << "save file success" << std::endl;
    Log(NORMAL, "save file success");

    return 0;
}

bool EnumFile(const std::string& src, std::vector<std::string>* file_list)
{
    namespace fs = boost::filesystem;
    fs::path root_path(src);
    if (!fs::exists(root_path)) //判断src源文件是否存在
    {
        // std::cerr << "not exist" << std::endl;
        Log(FATAL, src + " not exist");
        return false;
    }

    fs::recursive_directory_iterator end;   //创建迭代器枚举文件
    for (fs::recursive_directory_iterator it(root_path); it != end; ++it)
    {
        if (!fs::is_regular_file(*it))  //忽略非普通文件
        {
            continue;
        }
        if (it->path().extension() != ".html")  //忽略不以.html结束的文件
        {
            continue;
        }

        //  将目标文件地址放入file_list
        file_list->push_back(it->path().string());
        // std::cout << "Debug: " << it->path().string() << std::endl;
    }
    return true;
}


bool ParseTitle(const std::string& file, DocInfo_t* pDoc)
{
    size_t begin = file.find("<title>");
    begin += strlen("<title>");

    size_t end = file.find("</title>");

    if (begin >= end)
    {
        return false;
    }

    pDoc->title = file.substr(begin, end - begin);

    return true;
}
bool ParseContent(const std::string& file, DocInfo_t* pDoc)
{
    enum Status{
        LABLE,
        CONTENT
    };

    Status flag = LABLE;
    for (auto ch : file)
    {
        switch (flag)
        {
        case LABLE:
            if (ch == '>') flag = CONTENT;
            break;
        case CONTENT:
            if (ch == '<') flag = LABLE;
            else
            {
                if (ch == '\n')
                    ch = ' ';
                
                pDoc->content.push_back(ch);
            }
            break;
        default:
            return false;
            break;
        }
    }

    return true;
}
bool ParseUrl(const std::string& path, DocInfo_t* pDoc)
{
    const std::string url_head = "https://www.boost.org/doc/libs/1_79_0/doc/html/";
    std::string url_tail = path.substr(src.size());

    pDoc->url = url_head + url_tail;
    return true;
}

bool ParseHtml(const std::vector<std::string>& file_list, std::vector<DocInfo_t>* results)
{
    for (auto& file_path : file_list)
    {
        std::string target;
        if (!ns_util::Util::ReadFile(file_path, &target))
        {
            continue;
        }    // 将文件内容全部读取到target中

        DocInfo_t doc;
        if (!ParseTitle(target, &doc))  // 获得title
        {
            continue;
        }

        if (!ParseContent(target, &doc))    // 获得content
        {
            continue;
        }

        if (!ParseUrl(file_path, &doc)) // 获得url
        {
            continue;
        }

        // // for debug
        // std::cout << file_path << std::endl;
        // doc.Show();
        // break;

        results->push_back(std::move(doc));
    }
    return true;
}
bool SaveHtml(const std::vector<DocInfo_t>& results, const std::string& output)
{
    const char SEP = '\3'; // 分隔符

    std::ofstream out(output, std::ios::out | std::ios::binary);
    if (!out.is_open())
    {
        // std::cerr << "open " << output << " failed" << std::endl;
        Log(FATAL, "open " + output + " failed");
        return false;
    }

    // 将文件信息写入到output中
    for (auto& doc : results)
    {
        std::string out_string;
        out_string += doc.title;
        out_string += SEP;
        out_string += doc.content;
        out_string += SEP;
        out_string += doc.url;
        out_string += '\n';

        out.write(out_string.c_str(), out_string.size());
    }

    out.close();

    return true;
}