#ifndef UTIL_H
#define UTIL_H

#include <string>

/**
 * 函数功能: 将字符串中的所有特定子串置换为新的字符串
 * 函数输入: 
 *      str: 需要进行操作的字符串
 *      old_str: 旧的字符串
 *      new_str: 新的字符串
 * 
 * 函数输出: 置换完毕的字符串
 */
std::string& replace_all(std::string &str, std::string old_str, std::string new_str)
{
    while (1) {
        std::string::size_type pos(0);
        if ((pos = str.find(old_str)) != std::string::npos) {
            str.replace(pos, old_str.length(), new_str);
        } else {
            break;
        }
    }
    return str;
} 

#endif