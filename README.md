# StrStream
A fast string stream

## 应用场景 ## 
在平时的开发中，我们会遇到类似这样的场景：我们要构建一个由各种类型变量的值合理串接在一起的字符串，而这个字符串的总长度在百分之八九十的情况下都是比较短的，例如小于或等于1024个字节,而只有百分之十左右甚至更低的情况下才会超过这个长度。如果使用类似snprintf这类c函数，就不得不因为这低概率才出现的长度，而一开始申请一个比较大的栈空间或者堆空间，并且还要承受输入格式%d,%f,%s之类的笔误风险。而使用std::ostringstream，虽然不用考虑一开始定义的长度和输入格式带来的风险，但却由于频繁动态申请释放堆空间，牺牲了不少性能。 在我个人工作中，就遇到过类似的场景，例如设计一个日志模块，例如构建一些SQL语句，等等。StrStream 正是适合上述的场景。


## 如何使用 ##
使用 StrStream 只需要引用一个头文件sh_sstream.h，没有其他任何依赖。

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


## 例如编译测试用例 ##
g++ test_main.cc -o test