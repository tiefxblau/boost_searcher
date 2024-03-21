#pragma once
#include <string>
#include <vector>
#include <unordered_map>
#include <algorithm>
#include "Index.hpp"
#include "Log.hpp"
#include <jsoncpp/json/json.h>

namespace ns_searcher
{
    struct InvertedElemSet
    {
        uint64_t id;
        std::vector<std::string> words;   // 关键词组
        int weight_sum = 0; // weight之和
    };

    const std::string src_path = "./data/raw_html/raw"; // 网页数据路径
    const std::string stopwords_path = "./dict/stop_words.utf8";  // 停止词路径
    class Searcher
    {
    private:
        ns_index::Index *index;

    public:
        Searcher()
        {
            ns_util::JiebaUtil::LoadStopWords(stopwords_path);

            index = ns_index::Index::GetInstance();
            // std::cout << "正在建立索引..." << std::endl;
            Log(NORMAL, "正在建立索引...");
            index->BuildIndex(src_path);
            // std::cout << "索引建立完成" << std::endl;
            Log(NORMAL, "索引建立完成");
        }

        // query: 搜索关键词
        void Search(const std::string& query, std::string *json_string)
        {
            std::vector<std::string> words;
            ns_util::JiebaUtil::SplitString(query, &words); // 对query进行分词

            // for debug
            // for (auto& e : words)
            // {
            //     std::cout << e << std::endl;
            // }

            std::vector<InvertedElemSet> inverted_list_group; // 获得的全部倒排拉链
            std::unordered_map<uint64_t, InvertedElemSet> tokens_map;   // 用于去重
            for (auto word : words)
            {
                boost::to_lower(word);  // 将query转为小写

                ns_index::InvertedList *p_inverted_list = index->GetInvertedIndex(word);
                if (p_inverted_list == nullptr)
                {
                    continue;
                }

                // 对重复id的倒排元素进行合并
                for (auto& elem : *p_inverted_list)
                {
                    auto& item = tokens_map[elem.id];
                    item.id = elem.id;
                    item.weight_sum += elem.weight;
                    item.words.push_back(elem.word);
                }
            }

            // 将去重后倒排拉链插入到总倒排拉链
            for (auto& pair : tokens_map)
            {
                inverted_list_group.push_back(pair.second);
            }

            // 根据weight进行倒序排序
            std::sort(inverted_list_group.begin(), inverted_list_group.end(),
                      [](const InvertedElemSet &ie1, const InvertedElemSet &ie2)
                      {
                          return ie1.weight_sum > ie2.weight_sum;
                      });
            
            // 构建json串
            Json::Value root;
            for (auto& elem : inverted_list_group)
            {
                ns_index::DocInfo* pdoc = index->GetForwardIndex(elem.id);

                Json::Value partRoot;   // 子root
                partRoot["title"] = pdoc->title;
                partRoot["description"] = GetDescription(pdoc->content, elem.words[0]); // 
                partRoot["url"] = pdoc->url;
                // for debug
                // partRoot["id"] = static_cast<int>(pdoc->id);
                // partRoot["weight"] = elem.weight_sum;

                root.append(partRoot);
            }

            // Json::StyledWriter writer;
            Json::FastWriter writer;
            
            *json_string = writer.write(root); // 序列化
        }
    private:
        std::string GetDescription(const std::string& content, const std::string& word)
        {
            // description范围
            const int begin_step = 50;
            const int end_step = 100;

            // 匹配content中的word
            auto iter = std::search(content.begin(), content.end(), word.begin(), word.end(), [](int ch1, int ch2){
                return std::tolower(ch1) == std::tolower(ch2);
            }); // content中会出现大写, string中的find()不能用
            if (iter == content.end())
            {
                return "None";
            }

            int pos = std::distance(content.begin(), iter);

            int begin = 0;
            int end = content.size();

            // 控制description的范围
            if (pos - begin > begin_step) begin = pos - begin_step;
            if (end - pos > end_step) end = pos + end_step;

            if (begin >= end)
            {
                return "None";
            }

            return content.substr(begin, end - begin) + "...";
        }
    };
}