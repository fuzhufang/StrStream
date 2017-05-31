#ifndef __SH_STR_STREAM_H__
#define __SH_STR_STREAM_H__

#include <limits>
#include <string>
#include <assert.h>
#include <algorithm>
#ifdef __GNUC__ 
	#include <stdint.h>
#endif
#include <cstring>
#include <stdlib.h>
#include <stdio.h>


#ifndef __GNUC__ 
	#if  _MSC_VER >= 1900
	#else
		#ifndef snprintf
			#define snprintf _snprintf
		#endif
	#endif
#endif

namespace share
{
    namespace sstream
    {
        template<bool> struct STATIC_CHECK;
        template<>     struct STATIC_CHECK<true>{};

        //  (C) Copyright Beman Dawes 1999-2003. Distributed under the Boost
        //  Software License, Version 1.0. (See accompanying file
        //  LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

        //  See http://www.boost.org/libs/utility for documentation.
        //  Private copy constructor and copy assignment ensure classes derived from
        //  class noncopyable cannot be copied.
        //  Contributed by Dave Abrahams
        class noncopyable
        {
        protected:
            noncopyable() {}
            ~noncopyable() {}
        private:  // emphasize the following members are private
            noncopyable( const noncopyable& );
            const noncopyable& operator=( const noncopyable& );
        };

        template<typename Buffer> class StrStream;

        static const int MAX_NUMERIC_SIZE = 32;
        static STATIC_CHECK< (MAX_NUMERIC_SIZE - 12 > std::numeric_limits<double>::digits10) > __check3;
        static STATIC_CHECK< (MAX_NUMERIC_SIZE - 12 > std::numeric_limits<long double>::digits10) > __check4;
        static STATIC_CHECK< (MAX_NUMERIC_SIZE - 12 > std::numeric_limits<long>::digits10) > __check5;
        static STATIC_CHECK< (MAX_NUMERIC_SIZE - 12 > std::numeric_limits<long long>::digits10) > __check6;

        // 理论上 StreamBuf 支持字符串堆缓冲区最大长度 std::numeric_limits<int>::max()-1
        // 栈缓冲取就比较小. 一般在要求效率,并且不需要很大的缓冲区情况下,优先使用栈缓冲区
        template<int SIZE>
        class StreamBuf : public noncopyable
        {
            friend class StrStream<StreamBuf>;
        public:
            StreamBuf() : _heap_data(NULL)
            {
                _reset();
            }

            ~StreamBuf()
            {
                if (_heap_data) {
                    ::free(_heap_data);
                }
            }

            // 返回的字符串,会加上字符串结束符
            const char* data() const 
            {
                *(((StreamBuf*)this)->_cur)='\0';
                return _data;
            }

            inline int length() const
            {
                return static_cast<int>(_cur - _data);
            }

            inline int avail() const 
            {
                return static_cast<int>(_end() - _cur);
            }

            // return the string copy, for test 
            std::string to_string() const 
            {
                return std::string(_data, length()); 
            }

        private:
            void _append(const char* buf, int len)
            {
				assert(len>=0);
                if (avail() < len) {
                    if ( NULL == _malloc_heap(_cur_capacity()*2+len) ) {
                        return;
                    }
                }
                ::memcpy(_cur, buf, len);
                _cur += len;
            }

            inline char* _current() 
            {
                return _cur;
            }

            inline void _add(int len) 
            {
				assert(len>=0);
                _cur += len;                
            }

            void _reset() 
            { 
                _stack_data[0] = 0;
                ::free(_heap_data);
                _heap_data = NULL;
                _data = _stack_data;
                _cur = _data;
                _capacity = SIZE;
            }

            inline const char* _end() const
            {
                return _data + _cur_capacity();
            }

            inline int _cur_capacity()const 
            {
                return _capacity;
            }


            // 申请成功返回 not NULL, 否则返回NULL
            char* _malloc_heap(int new_size)
            {
                if (new_size <= _capacity) { // 申请新的空间应该要比原来的空间大
                    assert(false);
                    return NULL;
                }
                if (NULL == _heap_data) {
                    _heap_data = (char*)::malloc(new_size+1);
                    if (NULL == _heap_data) {
                        assert(false && "not enough memory");
                        return NULL;
                    }
                    _capacity = new_size;
                    int n = static_cast<int>(_cur - _data);
                    ::memcpy(_heap_data, _data, n);
                    _data = _heap_data;
                    _cur = _data + n;
                } else {
                    char* newbuf=(char*)::malloc(new_size+1);
                    if (NULL == newbuf){
                        assert(false && "not enough memory");
                        return NULL;
                    }
                    _capacity = new_size;
                    int n = static_cast<int>(_cur - _data);
                    ::memcpy(newbuf, _data, n);
                    _data = newbuf;
                    _cur = _data + n;
                    ::free(_heap_data);
                    _heap_data = newbuf;
                }
                return _data;
            }

        private:
            char* _data;
            char* _cur;

            char _stack_data[SIZE+1]; // 栈空间
            char*_heap_data; // 如果发生动态申请内存,这里指向申请到的内存首地址
            int  _capacity; // 当前的空间容纳量.
        };

        template<typename T>
        size_t convert(char buf[], T value)
        {
            static const char digits[] = "9876543210123456789";

            T i = value;
            char* p = buf;
            T lsd = 0;
            do {
                lsd = i % 10;
                i /= 10;
                *p++ = digits[lsd+9];
            } while (i != 0);

            if (value < 0) {
                *p++ = '-';
            }
            *p = '\0';
            std::reverse(buf, p);
            return p - buf;
        }

        static size_t convert2hex(char buf[], uintptr_t value)
        {
            static const char digitsHex[] = "0123456789abcdef";

            uintptr_t i = value;
            char* p = buf;
            uintptr_t lsd = 0;
            do {// 这里 i 为无符号整数时,可以用位运算代替除法,取模, 效率更高
                lsd = (i&(16-1)); //i % 16;
                i = (i>>4);//i /= 16;
                *p++ = digitsHex[lsd];
            } while (i != 0);
            *p = '\0';
            std::reverse(buf, p);
            return p - buf;
        }

        class StrMark
        {
        public:
            StrMark(const char* str, int len):_str(str), _len(len)
            {
                assert(::strlen(str) == _len);
            }
            const char* const _str;
            const size_t _len;
        };

        template<typename Buffer>
        class StrStream : public noncopyable
        {
        public:
            typedef StrStream self;

            self& operator<<(bool v)
            {
                _buffer._append(v ? "1" : "0", 1);
                return *this;
            }

            self& operator<<(char v)
            {
                _buffer._append(&v, 1);
                return *this;
            }

            self& operator<<(unsigned char v) 
            {
                *this << static_cast<unsigned int>(v);
                return *this;
            }

            self& operator<<(short v) 
            {
                *this << static_cast<int>(v);
                return *this;
            }

            self& operator<<(unsigned short v)
            {
                *this << static_cast<unsigned int>(v);
                return *this;
            }

            self& operator<<(int v)
            {
                _fmt_integer(v);
                return *this;
            }

            self& operator<<(unsigned int v)
            {
                _fmt_integer(v);
                return *this;
            }

            self& operator<<(long v)
            {
                _fmt_integer(v);
                return *this;
            }

            self& operator<<(unsigned long v)
            {
                _fmt_integer(v);
                return *this;
            }

            self& operator<<(long long v)
            {
                _fmt_integer(v);
                return *this;
            }

            self& operator<<(unsigned long long v)
            {
                _fmt_integer(v);
                return *this;
            }

            self& operator<<(const void* p)
            {
                uintptr_t v = reinterpret_cast<uintptr_t>(p);
                if (_buffer.avail() < MAX_NUMERIC_SIZE) {
                    if ( NULL == _buffer._malloc_heap(2*_buffer._cur_capacity()+MAX_NUMERIC_SIZE) ) {
                        assert(false);
                        return *this;
                    }
                }
                char* buf = _buffer._current();
                buf[0] = '0';
                buf[1] = 'x';
                _buffer._add(static_cast<int>(convert2hex(buf+2, v))+2);
                return *this;
            }

            self& operator<<(float v)
            {
                *this << static_cast<double>(v);
                return *this;
            }

            self& operator<<(double v)
            {
                if (_buffer.avail() < MAX_NUMERIC_SIZE) {
                    if ( NULL == _buffer._malloc_heap(2*_buffer._cur_capacity()+MAX_NUMERIC_SIZE) ) {
                        return *this;
                    }
                }
                int len = snprintf(_buffer._current(), MAX_NUMERIC_SIZE, "%.12g", v);
                _buffer._add(len);
                return *this;
            }

            self& operator<<(long double v)
            {
                if (_buffer.avail() < MAX_NUMERIC_SIZE) {
                    if ( NULL == _buffer._malloc_heap(2*_buffer._cur_capacity()+MAX_NUMERIC_SIZE) ) {
                        return *this;
                    }
                }
                int len = snprintf(_buffer._current(), MAX_NUMERIC_SIZE, "%.12g", v);
                _buffer._add(len);
                return *this;
            }

            self& operator<<(const StrMark& v)
            {
                _buffer._append(v._str, static_cast<int>(v._len));
                return *this;
            }

            self& operator<<(const char* v)
            {
                _buffer._append(v, static_cast<int>(strlen(v)));
                return *this;
            }

            self& operator<<(const std::string& v) 
            {
                _buffer._append(v.c_str(), static_cast<int>(v.size()));
                return *this;
            }

            void append(const char* data, int len) 
            { 
                _buffer._append(data, len); 
            }

            const Buffer& buffer() const 
            { 
                return _buffer;
            }

            void reset_buffer() 
            { 
                _buffer._reset();
            }

        private:
            template<typename T>
            void _fmt_integer(T v)
            {
                if (_buffer.avail() < MAX_NUMERIC_SIZE) {
                    if ( NULL == _buffer._malloc_heap(2*_buffer._cur_capacity()+MAX_NUMERIC_SIZE) ) {
                        return ;
                    }
                }
                _buffer._add(static_cast<int>(convert(_buffer._current(), v)));
            }

            Buffer _buffer;
        };
    }
}
#endif  // __SH_STR_STREAM_H__
