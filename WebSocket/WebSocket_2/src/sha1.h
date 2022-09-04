/*
 *  sha1.h
 *
 *  Copyright (C) 1998, 2009
 *  Paul E. Jones <paulej@packetizer.com>
 *  All Rights Reserved.
 *
 *****************************************************************************
 *  $Id: sha1.h 12 2009-06-22 19:34:25Z paulej $
 *****************************************************************************
 *
 *  Description:
 *      This class implements the Secure Hashing Standard as defined
 *      in FIPS PUB 180-1 published April 17, 1995.
 *
 *      Many of the variable names in this class, especially the single
 *      character names, were used because those were the names used
 *      in the publication.
 *
 *      Please read the file sha1.cpp for more information.
 *
 */

#ifndef _SHA1_H_
#define _SHA1_H_

class SHA1
{
    public:
        SHA1();
        virtual ~SHA1();

        // 重新初始化类
        void Reset();

        // 返回消息摘要
        bool Result(unsigned* message_digest_array);

        // 向SHA1提供输入
        void Input(const unsigned char* message_array, unsigned length);
        void Input(const char* message_array, unsigned length);
        void Input(unsigned char message_element);
        void Input(char message_element);

        SHA1& operator<<(const char* message_array);
        SHA1& operator<<(const unsigned char* message_array);
        SHA1& operator<<(const char message_element);
        SHA1& operator<<(const unsigned char message_element);

    private:
        // 处理消息的下 512 位
        void ProcessMessageBlock();

        // 将当前消息块填充到 512 位
        void PadMessage();

        // 执行循环左移操作
        inline unsigned CircularShift(int bits, unsigned word);

        unsigned H[5];                      // 消息摘要缓冲区
        unsigned Length_Low;                // 消息长度（以位为单位）
        unsigned Length_High;               // 消息长度（以位为单位）

        unsigned char Message_Block[64];    // 512-bit message blocks
        int Message_Block_Index;            // 消息块数组的索引

        bool Computed;                      // 是否计算摘要？
        bool Corrupted;                     // 消息摘要是否损坏？
};

#endif