#ifndef _TEST_LOGSTREAM_H__
#define _TEST_LOGSTREAM_H__

#include "sh_sstream.h"
#include <stdlib.h>
#include <stdio.h>

void test_write_file(const char* title, const char* data)
{
    FILE* fp = fopen("test_sstream.log", "a+");
    fputs(title, fp);
    char buf[256];
    sprintf(buf, ", data_size=%d", strlen(data));
    fputs(buf, fp);
    printf("%s, data_size=%d", title, strlen(data));
    const char * line="\n====================================\n";
    fputs(line, fp);
    printf("%s", line);
    fputs(data, fp);
    fputs("\n\n\n", fp);
    printf("%s\n\n\n", data);
    fclose(fp);
}

template<int SIZE>
void test_data(share::sstream::StrStream<share::sstream::StreamBuf<SIZE> >& logos)
{
    // 测试能支持的所有类型,以及边界情况
    
    bool b1=true;
    logos<<"b1="<<b1<<"\n";

    bool b2=false;
    logos<<"b2="<<b2<<"\n";

    char c1 = 'A';
    logos<<"c1="<<c1<<"|min_char="<<std::numeric_limits<char>::min()<<"|max_char="<<std::numeric_limits<char>::max()<<"\n";

    unsigned char byte1=250;
    logos<<"byte1="<<byte1<<"|min_char="<<std::numeric_limits<unsigned char>::min()<<"|max_char="<<std::numeric_limits<unsigned char>::max()<<"\n";

    short short_1=12;
    logos<<"short_1="<<short_1<<"|"<<"min_short="<<std::numeric_limits<short>::min()<<"|max_short="<<std::numeric_limits<short>::max()<<"\n";

    unsigned short ushort_1=255;
    logos<<"ushort_1="<<ushort_1<<"|"<<"min_ushort="<<std::numeric_limits<unsigned short>::min()<<"|max_ushort="<<std::numeric_limits<unsigned short>::max()<<"\n";

    int int_1 = 0x12ab34ef;
    logos<<"int_1="<<int_1<<"|"<<"min_int="<<std::numeric_limits<int>::min()<<"|max_int="<<std::numeric_limits<int>::max()<<"\n";

    unsigned int uint_1 = 123456789;
    logos<<"uint_1="<<uint_1<<"|min_uint="<<std::numeric_limits<unsigned int>::min()<<"|max_uint="<<std::numeric_limits<unsigned int>::max()<<"\n";

    long long_1 = 987654321;
    logos<<"long_1="<<long_1<<"|min_long="<<std::numeric_limits<long>::min()<<"|max_long="<<std::numeric_limits<long>::max()<<"\n";

    unsigned long ulong_1=1234567890;
    logos<<"ulong_1="<<ulong_1<<"|min_ulong="<<std::numeric_limits<unsigned long>::min()<<"|max_ulong="<<std::numeric_limits<unsigned long>::max()<<"\n";

    long long ll_1 = 12345678900;
    logos<<"ll_1="<<ll_1<<"|min_long_long="<<std::numeric_limits<long long>::min()<<"|max_long_long="<<std::numeric_limits<long long>::max()<<"\n";

    unsigned long long ull_1 = 98765432100;
    logos<<"ull_1="<<ull_1<<"|min_ulong_long="<<std::numeric_limits<unsigned long long>::min()<<"|max_ulong_long="<<std::numeric_limits<unsigned long long>::max()<<"\n";

    struct _sNetModule
    {
        int a;
        int b;
    };
    _sNetModule* pobj=(_sNetModule*)0x12ab34ef;
    logos<<"sNetModule="<<pobj<<"\n";

    float f1=123.456f;
    logos<<"f1="<<f1<<"|min_float="<<std::numeric_limits<float>::min()<<"|max_float="<<std::numeric_limits<float>::max()<<"\n";

    double df1=123456.789;
    logos<<"df1="<<df1<<"|min_double="<<std::numeric_limits<double>::min()<<"|max_double="<<std::numeric_limits<double>::max()<<"\n";

    long double ldf1=123456789.123456789;
    logos<<"ldf1="<<ldf1<<"|min_double="<<std::numeric_limits<long double>::min()<<"|max_double="<<std::numeric_limits<long double>::max()<<"\n";

    const char *ps1="HELLO WORLD";
    logos<<"ps1="<<ps1<<"\n";

    int ps1_len = strlen(ps1);
    logos<<"ps1(str,len)="<<share::sstream::StrMark(ps1,ps1_len)<<"\n";

    std::string str1="FZF@163.com";
    logos<<"str1="<<str1<<"\n";

    test_write_file("test_data-all-data", logos.buffer().data());
    //test_write_file("test_data-all-to-string", logos.buffer().to_string().c_str());
}
void test_logstream ()
{
    test_write_file("BEGIN TEST", "+++++++++++++++++++++++++++++++++++++++++++++++++++++++");
    {// 测试栈缓冲区刚好足够的情况
        share::sstream::StrStream<share::sstream::StreamBuf<10> > logos;
        const char* ptest = "0123456789";
        logos<<ptest;
        test_write_file("stack size 10, wirte 10", logos.buffer().data());
    }

    {// 测试栈缓冲区充足的情况
        share::sstream::StrStream<share::sstream::StreamBuf<64> > logos;
        const char* ptest = "0123456789";
        logos<<ptest;
        test_write_file("stack size 64, wirte 10", logos.buffer().data());
    }

    {// 测试栈缓冲区不足的情况
        share::sstream::StrStream<share::sstream::StreamBuf<5> > logos;
        const char* ptest = "0123456789";
        logos<<ptest;
        test_write_file("stack size 5, wirte 10", logos.buffer().data());
    }

    {// 测试栈缓冲区为0的情况
        share::sstream::StrStream<share::sstream::StreamBuf<0> > logos; // 等价于std::ostringstream
        const char* ptest = "0123456789";
        logos<<ptest;
        test_write_file("stack size 0, wirte 10", logos.buffer().data());
    }
    {
        share::sstream::StrStream<share::sstream::StreamBuf<0> > logos; // 等价于std::ostringstream
        test_data(logos);
        test_write_file("stack size 0, wirte more", logos.buffer().data());
    }

    {
        share::sstream::StrStream<share::sstream::StreamBuf<128> > logos; // 前期使用栈空间,后来栈不足后,使用堆
        test_data(logos);
        test_write_file("stack size 128, wirte more", logos.buffer().data());
    }

    {
        share::sstream::StrStream<share::sstream::StreamBuf<1024> > logos; // 一直使用栈空间
        test_data(logos);
        test_write_file("stack size 1024, wirte more", logos.buffer().data());
    }

    test_write_file("END TEST", "+++++++++++++++++++++++++++++++++++++++++++++++++++++++");
}

#endif // _TEST_LOGSTREAM_H__

