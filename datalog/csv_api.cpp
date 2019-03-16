#include "csv_api.h"
#include "csv_internal.h"
#include "debug_api.h"

CSVLine::CSVLine()
{
    impl = new CSVLineImpl();
#ifdef _DEBUG_API_H_
    if(impl == nullptr)
    {
        SHOW_MEMORY_ERROR();
        throw MEMORY_ALLOCATED_ERROR();
    }
#endif
}

CSVLine::~CSVLine()
{
    delete impl;
    impl = nullptr;
}

CSVLine::CSVLine(unsigned int initial_length, unsigned int increase_length)
{
    impl = new CSVLineImpl(initial_length, increase_length);
#ifdef _DEBUG_API_H_
    if(impl == nullptr)
    {
        SHOW_MEMORY_ERROR();
        throw MEMORY_ALLOCATED_ERROR();
    }
#endif
}

const char* CSVLine::read_line( std::ifstream& file_stream, bool is_strict )
{
    return impl->ReadNewLine(file_stream, is_strict);
}

unsigned int CSVLine::get_length() const
{
    return impl->GetLineLength();
}


CSVParser::CSVParser()
{
    impl = new CSVParserImpl();
#ifdef _DEBUG_API_H_
    if(impl == nullptr)
    {
        SHOW_MEMORY_ERROR();
        throw MEMORY_ALLOCATED_ERROR();
    }
#endif
}

CSVParser::CSVParser( unsigned int init_max_length )
{
    impl = new CSVParserImpl(init_max_length);
#ifdef _DEBUG_API_H_

    if(impl == nullptr)
    {
        SHOW_MEMORY_ERROR();
        throw MEMORY_ALLOCATED_ERROR();
    }
#endif
}

CSVParser::CSVParser(unsigned int init_line_length, unsigned int init_str_count)
{
        impl = new CSVParserImpl(init_line_length, init_str_count);
#ifdef _DEBUG_API_H_

    if(impl == nullptr)
    {
        SHOW_MEMORY_ERROR();
        throw MEMORY_ALLOCATED_ERROR();
    }
#endif
}

CSVParser::~CSVParser()
{
    delete impl;
    impl = nullptr;
}

unsigned int CSVParser::parse( const char* line, const char delimiter )
{
    return impl->ParseLine(line, delimiter);
}

const char* CSVParser::get_string( unsigned int index ) const
{
    return impl->GetString(index);
}

unsigned int CSVParser::get_count() const
{
    return impl->GetStringCount();
}

CSVWriter::~CSVWriter()
{

}

CSVWriter::CSVWriter(const char* str)
{
    if(str != nullptr) temp.assign(str);
}

void CSVWriter::write(std::ostream& os) const
{
    const std::string delims("\n,\"");
    if(temp.find_first_of(delims) !=  std::string::npos)
    {
        os<<"\"";
        int length = temp.length();
        for(int i = 0; i < length; i++)
        {
            char c = temp[i];
            if(c == '\"') os<<"\"\"";
            else os<<c;
        }
        os<<"\"";
    }
    else
    {
        os<<temp;
    }
}

std::ostream& operator << (std::ostream& os, const CSVWriter& str)
{
    str.write(os);
    return os;
}

