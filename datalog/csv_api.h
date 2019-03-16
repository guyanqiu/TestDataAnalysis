#ifndef _CSV_API_H_
#define _CSV_API_H_
#include <fstream>

class CSVLine
{
public:
    const char* read_line(std::ifstream& file_stream, bool is_strict = false);
    unsigned int get_length() const;
    CSVLine(unsigned int initial_length, unsigned int increase_length);
    CSVLine();
    ~CSVLine();

private:
    typedef class CSVLineImpl Impl;
    Impl *impl;
    CSVLine(const CSVLine&);
    CSVLine& operator=(const CSVLine&);
};

class CSVParser
{
public:
    unsigned int parse(const char* line, const char delimiter);
    const char* get_string(unsigned int index) const;
    unsigned int get_count() const;

    CSVParser();
    CSVParser(unsigned int init_line_length);
    CSVParser(unsigned int init_line_length, unsigned int init_str_count);
    ~CSVParser();
private:
    typedef class CSVParserImpl Impl;
    Impl *impl;
    CSVParser(const CSVParser&);
    CSVParser& operator=(const CSVParser&);
};

class CSVWriter
{
public:
    explicit CSVWriter(const char* str);
    ~CSVWriter();
    void write(std::ostream& os) const;
    friend std::ostream& operator << (std::ostream& os, const CSVWriter& str);

private:
    std::string temp;
    CSVWriter(const CSVWriter&);
    CSVWriter& operator=(const CSVWriter&);
};

#endif//_CSV_API_H_
