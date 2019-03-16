#include <cstring>
#include "csv_internal.h"
#include "debug_api.h"
#include <stdio.h>


CSVLineImpl::CSVLineImpl()
{
    line_length = 0;
    init_length = 1024;
    inc_length = 512;
    total_length = init_length;
    a_line = new char[init_length];

#ifdef _DEBUG_API_H_
    if(!a_line)
    {
        SHOW_MEMORY_ERROR();
        throw MEMORY_ALLOCATED_ERROR();
    }
#endif
}

CSVLineImpl::CSVLineImpl( unsigned int initial_length, unsigned int increase_length )
{
    line_length = 0;
    init_length = initial_length;
    inc_length = increase_length;
    total_length = init_length;
    a_line = new char[init_length];

#ifdef _DEBUG_API_H_
    if(!a_line)
    {
        SHOW_MEMORY_ERROR();
        throw MEMORY_ALLOCATED_ERROR();
    }
#endif
}

unsigned int CSVLineImpl::GetLineLength() const
{
    return line_length;
}

CSVLineImpl::~CSVLineImpl()
{
    if(a_line)
    {
        delete [] a_line;
        a_line = nullptr;
    }
}

bool CSVLineImpl::RelocateNewLine()
{
    total_length += inc_length;
    char* a_new_line = new char[total_length];
    if(!a_new_line)
    {
#ifdef _DEBUG_API_H_
        SHOW_MEMORY_ERROR();
        throw MEMORY_ALLOCATED_ERROR();
#endif
        delete [] a_line;
        a_line = nullptr;
        return false;
    }
    std::memcpy(a_new_line, a_line, init_length);
    init_length = total_length;
    delete [] a_line;
    a_line = a_new_line;
    return true;
}

const char* CSVLineImpl::ReadNewLine( std::ifstream& file_stream, bool is_strict )
{
    const char quotation_mark = '\"';
    const char nil_char = '\0';
    const char newline_mark = '\n';
    const char return_mark = '\r';

    a_line[0] = nil_char;
    char current_char;
    bool inquotes = false;
    unsigned int char_count = 0;
    line_length = 0;

    if(!is_strict)
    {
        while(file_stream.good())
        {
            current_char = file_stream.get();
            if(current_char == newline_mark || current_char == return_mark)
            {
                char next_char = file_stream.peek();
                if(next_char == newline_mark || next_char == return_mark)
                {
                    file_stream.get();
                }
                break;
            }
            else
            {
                if(file_stream.eof()) break;
                // read one character
                a_line[char_count] = current_char;
                char_count++;
            }

            // reallocate memory of character array
            if(char_count >= init_length)
            {
                if(RelocateNewLine() == false)
                    return nullptr;
            }
        }
    }
    else
    {
        while(file_stream.good())
        {
            current_char = file_stream.get();
            // meet quotation mark
            if(current_char == quotation_mark && !inquotes)
            {
                inquotes = true;
                a_line[char_count] = quotation_mark;
                char_count++;
            }
            else if(current_char == quotation_mark && inquotes)
            {
                if(file_stream.peek() == quotation_mark)
                {
                    a_line[char_count] = quotation_mark;
                    char_count++;
                    a_line[char_count] = quotation_mark;
                    char_count++;
                    file_stream.get();
                }
                else
                {
                    inquotes = false;
                    a_line[char_count] = quotation_mark;
                    char_count++;
                }
            }
            else if(!inquotes && (current_char == newline_mark || current_char == return_mark))
            {
                char next_char = file_stream.peek();
                if(next_char == newline_mark || next_char == return_mark)
                {
                    file_stream.get();
                }
                break;
            }
            else if(inquotes && (current_char == newline_mark || current_char == return_mark))
            {
                a_line[char_count] = newline_mark;
                char_count++;
            }
            else
            {
                if(file_stream.eof()) break;
                // read one character
                a_line[char_count] = current_char;
                char_count++;
            }

            // reallocate memory of character array
            if(char_count >= init_length)
            {
                if(RelocateNewLine() == false)
                    return nullptr;
            }
        }
    }
    if(char_count == 0 )
    {
        return nullptr;
    }

    a_line[char_count] = nil_char;
    line_length = char_count;
    return a_line;
}


CSVParserImpl::CSVParserImpl()
{
    //str_array.clear();
    temp_string = nullptr;
    old_length = 0;
}

CSVParserImpl::~CSVParserImpl()
{
    str_array.clear();
    if(temp_string) delete [] temp_string;
    old_length = 0;
}

CSVParserImpl::CSVParserImpl(unsigned int line_length)
{
    //str_array.clear();
    //temp_string = nullptr;
    old_length = line_length;
    temp_string = new char [old_length];

#ifdef _DEBUG_API_H_
    if(temp_string == nullptr)
    {
        SHOW_MEMORY_ERROR();
        throw MEMORY_ALLOCATED_ERROR();
    }
#endif
}

CSVParserImpl::CSVParserImpl(unsigned int line_length, unsigned int str_count)
{
    str_array.reserve(str_count);
    old_length = line_length;
    temp_string = new char [old_length];

#ifdef _DEBUG_API_H_
    if(temp_string == nullptr)
    {
        SHOW_MEMORY_ERROR();
        throw MEMORY_ALLOCATED_ERROR();
    }
#endif
}

const char* CSVParserImpl::GetString( unsigned int index )
{
    if(index > str_array.size())
    {
#ifdef _DEBUG_API_H_
        SHOW_INDEX_ERROR(str_array.size(), index);
        throw INDEX_OVER_RANGE(str_array.size(), index);
#endif
        return nullptr;
    }
    else
    {
        return str_array[index].c_str();
    }
}

void CSVParserImpl::AppenString( const char* str )
{
    std::string temp(str);
    str_array.push_back(temp);
}

unsigned int CSVParserImpl::GetStringCount() const
{
    return str_array.size();
}

unsigned int CSVParserImpl::ParseLine( const char* a_line, const char delimiter )
{
    str_array.clear();
    if(!a_line)
    {
        return 0;
    }
    unsigned int line_length;           // length of a line
    unsigned int char_count = 0;        // count char of temp string
    unsigned int i = 0;

    char quotation_mark = '\"';
    char nil_char = '\0';

    line_length = strlen(a_line);
    if(line_length == 0)
    {
        return 0;
    }

    if(!temp_string)
    {
        temp_string = new char[line_length+1];
        if(!temp_string)
        {
#ifdef _DEBUG_API_H_
            if(temp_string == nullptr)
            {
                SHOW_MEMORY_ERROR();
                throw MEMORY_ALLOCATED_ERROR();
            }
#endif
            return 0;
        }
        old_length = line_length + 1;
        temp_string[line_length] = nil_char;
    }

    if(line_length >= old_length)
    {
        delete [] temp_string;
        temp_string = new char[line_length+1];
        if(!temp_string)
        {
#ifdef _DEBUG_API_H_
            if(temp_string == nullptr)
            {
                SHOW_MEMORY_ERROR();
                throw MEMORY_ALLOCATED_ERROR();
            }
#endif
            return 0;
        }
        old_length = line_length + 1;
        temp_string[line_length] = nil_char;
    }

    bool inquotes = false;
    char_count = 0;
    for(i = 0; i < line_length; i++)
    {
        char current_char = a_line[i];
        if(!inquotes && current_char == quotation_mark )
        {
            inquotes = true;
        }
        else if(inquotes && current_char == quotation_mark)
        {
            if((i < (line_length-1)) && (a_line[i+1] == quotation_mark))
            {
                temp_string[char_count] = quotation_mark;
                char_count++;
                i++;
            }
            else if(i == (line_length-1))
            {
                temp_string[char_count] = nil_char;
                AppenString(temp_string);
                char_count = 0;
                break;
            }
            else
            {
                inquotes = false;
            }
        }
        else if(!inquotes && current_char == delimiter)
        {
            temp_string[char_count] = nil_char;
            AppenString(temp_string);
            char_count = 0;
        }
        else
        {
            temp_string[char_count] = current_char;
            char_count++;
        }
    }

    if(char_count > 0 )
    {
        temp_string[char_count] = nil_char;
        AppenString(temp_string);
        char_count = 0;
    }
    return str_array.size();
}
