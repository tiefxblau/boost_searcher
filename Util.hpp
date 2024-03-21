#pragma once
#include <iostream>
#include <string>
#include <vector>
#include <unordered_set>
#include <fstream>
#include <boost/algorithm/string.hpp>
#include "Cppjieba/Jieba.hpp"
#include "Log.hpp"

namespace ns_util
{
    class Util
    {
    public:
        // 读取文件到outStr
        static bool ReadFile(const std::string& file_path, std::string* outStr)
        {
            std::ifstream in(file_path);
            if (!in.is_open())
            {
                // std::cerr << "open " << file_path << " failed" << std::endl;
                Log(FATAL, "open " + file_path + " failed");
                // in.close(); 打开失败不需要关闭
                return false;
            }

            std::string line;
            while (std::getline(in, line))
            {
                (*outStr) += line;
            }

            in.close();

            return true;
        }

        // 按(sep)分割字符串
        static bool CutString(std::vector<std::string>* out, const std::string& target, const std::string& sep)
        {
            boost::split(*out, target, boost::is_any_of(sep), boost::token_compress_on);
            return true;
        }
    };

    const char* const DICT_PATH = "./dict/jieba.dict.utf8";
    const char* const HMM_PATH = "./dict/hmm_model.utf8";
    const char* const USER_DICT_PATH = "./dict/user.dict.utf8";
    const char* const IDF_PATH = "./dict/idf.utf8";
    const char* const STOP_WORD_PATH = "./dict/stop_words.utf8";
    class JiebaUtil
    {
    private:
        static cppjieba::Jieba jieba;
        static std::unordered_set<std::string> stop_words;
    public:
        // 加载停止词
        static bool LoadStopWords(const std::string& stopwords_path)
        {
            std::ifstream fin(stopwords_path);
            if (!fin.is_open())
            {
                Log(FATAL, "open " + stopwords_path + " failed");
                return false;
            }

            std::string line;
            while (std::getline(fin, line))
            {
                JiebaUtil::stop_words.insert(line);
            }

            fin.close();
            return true;
        }
        // jieba分词
        // 在使用该方法之前,请先使用Jieba::LoadStopWords
        static void SplitString(const std::string& target, std::vector<std::string>* out)
        {
            if (JiebaUtil::stop_words.empty())
            {
                Log(FATAL, "please use LoadStopWords before");
                return;
            }

            JiebaUtil::jieba.CutForSearch(target, *out);

            // 去除停止词
            auto it = out->begin();
            while (it != out->end())
            {
                if (JiebaUtil::stop_words.find(*it) != JiebaUtil::stop_words.end())
                {
                    it = out->erase(it);
                }
                else
                {
                    ++it;
                }
            }
        }
    };
    cppjieba::Jieba JiebaUtil::jieba(DICT_PATH, HMM_PATH, USER_DICT_PATH, IDF_PATH, STOP_WORD_PATH);
    std::unordered_set<std::string> JiebaUtil::stop_words;
} // namespace ns_util
