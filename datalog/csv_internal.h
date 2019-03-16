#ifndef _CSV_INTERNAL_H_
#define _CSV_INTERNAL_H_

#include <iostream>
#include <fstream>
#include <vector>
#include <string>

class  CSVLineImpl
{
public:
    const char* ReadNewLine(std::ifstream& file_stream, bool is_strict = false);
    unsigned int GetLineLength() const;
    CSVLineImpl();
    CSVLineImpl(unsigned int initial_length, unsigned int increase_length);
    ~CSVLineImpl();
private:
    char* a_line;
    unsigned int line_length;
    unsigned int init_length;
    unsigned int inc_length;
    unsigned int total_length;
    bool RelocateNewLine();

    CSVLineImpl(const CSVLineImpl&);
    CSVLineImpl& operator=(const CSVLineImpl& );
};


class CSVParserImpl
{
public:
    unsigned int ParseLine(const char* a_line, const char delimiter);
    const char* GetString(unsigned int index);
    unsigned int GetStringCount() const;

    CSVParserImpl();
    CSVParserImpl(unsigned int line_length);
    CSVParserImpl(unsigned int line_length, unsigned int str_count);
    ~CSVParserImpl();
private:
    std::vector<std::string> str_array;
    void AppenString(const char* str);
    char *temp_string;
    unsigned int old_length;

    CSVParserImpl(const CSVParserImpl&);
    CSVParserImpl& operator=(const CSVParserImpl& );
};


















#endif//_CSV_INTERNAL_H_