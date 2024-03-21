#pragma once
#include <iostream>
#include <vector>
#include <string>
#include <unordered_map>
#include <fstream>
#include <utility>
#include <mutex>
#include "Util.hpp"
#include "Log.hpp"

namespace ns_index
{
    struct DocInfo
    {
        uint64_t id;    
        std::string title;  // 标题
        std::string content; // 内容
        std::string url;    // url
    };

    struct InvertedElem
    {
        uint64_t id;
        std::string word;   // 关键词
        int weight = 0;
    };

    typedef std::vector<InvertedElem> InvertedList;

    struct word_cnt
    {
        int title_cnt = 0;
        int content_cnt = 0;
    };

    class Index // 单例模式
    {
    private:
        std::vector<DocInfo> forward_index;
        std::unordered_map<std::string, InvertedList> inverted_index;

        static Index* ins;  // 实例指针
        static std::mutex mtx;

        Index() = default;
        Index(const Index& copy) = delete;
        Index& operator=(const Index& index) = delete;
    public:
        static Index* GetInstance() // 获取实例
        {
            if (Index::ins == nullptr)
            {
                std::unique_lock<std::mutex> lock(Index::mtx);
                if (Index::ins == nullptr)
                {
                    Index::ins = new Index;
                }
            }

            return Index::ins;
        }

        DocInfo* GetForwardIndex(uint64_t id)   // 根据id找到找到文档内容
        {
            if (id >= forward_index.size())
            {
                // std::cerr << "doc's id out of range" << std::endl;
                Log(FATAL, "doc's id out of range");
                return nullptr;
            }

            return &forward_index[id];
        }
        InvertedList* GetInvertedIndex(const std::string& word) // 根据关键字string，获得倒排拉链
        {
            auto it = inverted_index.find(word);
            if (it == inverted_index.end())
            {
                // std::cerr << word << ": can't find inverted index" << std::endl;
                // Log(WARNING, word + ": can't find inverted index");
                return nullptr;
            }

            return &(it->second);
        }
        bool BuildIndex(const std::string& src_path)    // 建立索引
        {
            std::ifstream fin(src_path);
            if (!fin.is_open())
            {
                // std::cerr << src_path << "open failed" << std::endl;
                Log(FATAL, src_path + "open failed");
                return false;
            }

            std::string line;   // 按行读取html文件信息
            int cnt = 0;    // 进度计数
            while (getline(fin, line))
            {
                DocInfo* doc = BuildForwardIndex(line);
                if (doc == nullptr)
                {
                    // std::cout << "Bug?: doc == nullptr" << std::endl;
                    // Log(DEBUG, "Bug?: doc == nullptr");
                    continue;
                }
                
                if (!BuildInvertedIndex(*doc))
                {
                    // std::cout << "Bug?: return false" << std::endl;
                    // Log(DEBUG, "Bug?: return false");
                    continue;
                }

                if (++cnt % 100 == 0)
                {
                    Log(NORMAL, "已建立索引: " + std::to_string(cnt));
                }
            }

            return true;
        }
    private:
        DocInfo* BuildForwardIndex(const std::string& string_doc)   // 建立正排索引
        {
            const std::string sep = "\3";

            std::vector<std::string> split_str; // 存放分割后的字符串
            ns_util::Util::CutString(&split_str, string_doc, sep);
            if (split_str.size() != 3)
            {
                // // for debug
                // for (auto& e : split_str)
                // {
                //     std::cout << e << std::endl;
                // }
                // std::cout << "Bug?: ns_util::Util::CutString" << std::endl;
                Log(FATAL, "split_str.size() != 3");

                return nullptr;
            }

            // 将信息填入doc
            DocInfo doc;
            doc.id = forward_index.size();
            doc.title = split_str[0];
            doc.content = split_str[1];
            doc.url = split_str[2];

            // 放入正排索引
            forward_index.push_back(std::move(doc));

            return &forward_index.back();
        }
        bool BuildInvertedIndex(const DocInfo& doc) // 建立倒排索引
        {
            std::unordered_map<std::string, word_cnt> word_map; // 词语与出现次数的映射

            // 对title进行处理
            std::vector<std::string> split_title;
            ns_util::JiebaUtil::SplitString(doc.title, &split_title);   // Jieba分词
            for (auto part : split_title)
            {
                boost::to_lower(part);  // 不区分大小写,全部转为小写
                word_map[part].title_cnt++;
            }

            // 对content进行处理
            std::vector<std::string> split_content;
            ns_util::JiebaUtil::SplitString(doc.content, &split_content);
            for (auto part : split_content)
            {
                boost::to_lower(part);
                word_map[part].content_cnt++;
            }

const int TITLE_VALUE = 10; // 默认title中词语的权值
const int CONTENT_VALUE = 1;// 默认content中词语的权值
            for (auto& word_pair : word_map)
            {
                // 构建倒排索引元素
                InvertedElem elem;
                elem.id = doc.id;
                elem.word = word_pair.first;
                elem.weight = TITLE_VALUE * word_pair.second.title_cnt + CONTENT_VALUE * word_pair.second.content_cnt;  // 权值的简单处理

                // 放入倒排索引
                inverted_index[word_pair.first].push_back(std::move(elem));
            }
            
            return true;
        }
    };
    
    Index* Index::ins = nullptr;
    std::mutex Index::mtx;
}