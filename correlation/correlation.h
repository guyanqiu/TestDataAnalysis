#ifndef _MY_CORR_CALCUALTE_H_
#define _MY_CORR_CALCUALTE_H_

#include "../datalog/datalog.h"
#include <vector>
#include <string>

enum CORR_CALCULATE_ERROR : int
{
    CORR_RESULT_IS_OK = 0,
    CORR_LABEL_NOT_SAME = 1,
    CORR_NUMBER_NOT_SAME = 2,
    CORR_LABEL_AND_NUMBER_NOT_SAME = 3,
	CORR_DATA_NOT_ALL_PASS = 4,
    CORR_DATACOUNT_NOT_SAME = 5,
    CORR_ITEM_NOT_EXIST = -1,
    CORR_DATACOUNT_TOO_FEW = -2,
    CORR_REF_SITE_NOT_SET = -3,
    CORR_SITE_NOT_EXIST = -4,
    CORR_ITEMCOUNT_NOT_SAME = -5,
    CORR_SITECOUNT_TOO_FEW = -6,
    CORR_NOT_ALL_ITEM_TESTED = -7,
};

class TestItem_CORR
{
private:
    int m_calculate_mode;
    bool m_set_ref_item;
	double m_corr_spec;
	unsigned int m_site_count;
    unsigned int m_data_count;

    double m_high_spec;
    double m_low_spec;
    bool m_highspec_valid;
    bool m_lowspec_valid;

    unsigned int m_number;
	double m_ref_average;
    std::string m_label;
    std::string m_unit;

	std::vector<double> m_average_list;
    std::vector<double> m_delta_value_list;
public:
    void set_calculate_mode(int mode);
	bool set_ref_item(const TestItem* item, double default_lowlimit, double default_highlimit);
    bool add_item(const TestItem* item);
    double get_highspec() const;
    double get_lowspec() const;
    unsigned int get_site_count() const;
    unsigned int get_data_count() const;
    double get_tolerance() const;
    double get_corr_spec() const;
    double get_ref_average() const;
    double get_corr_average(unsigned int index) const;
    double get_corr_value(unsigned int index) const;
    double get_delta_value(unsigned int index) const;
    unsigned int get_number() const;
    const char* get_label() const;
    const char* get_unit() const;
    bool set_corr_sepc(double spec_value);
public:
    TestItem_CORR();
    ~TestItem_CORR();
};

class TestSite_CORR
{
private:
	bool m_add_ref_site;
    int m_calculate_mode;
    unsigned int m_site_count;
    unsigned int m_item_count;
    unsigned int m_data_count;

    double m_low_spec;
    double m_high_spec;
    double m_corr_spec;
    bool m_calculate_ok;
	const TestSite* m_ref_site;

    std::vector<const TestSite*> m_site_list;
    std::vector<TestItem_CORR*> m_corr_list;
    std::vector<std::string> m_label_list;
    std::vector<unsigned int> m_number_list;

public:
    void set_calcualte_mode(int mode);
	CORR_CALCULATE_ERROR set_ref_site(const TestSite* site, double default_low_spec, double default_high_spec);
    CORR_CALCULATE_ERROR add_site(const TestSite* site);
    CORR_CALCULATE_ERROR calculate();
    const TestSite* get_site(unsigned int index) const;
    TestItem_CORR* get_corr(unsigned int index) const;
    unsigned int get_data_count() const;
    unsigned int get_site_count() const;
    unsigned int get_item_count() const;
    bool save_result_csv(const char* filename);
    bool save_result_xls(const char* filename);
    void clear();
    bool set_corr_spec(double spec_value);
    double get_corr_spec() const;

public:
    TestSite_CORR();
    ~TestSite_CORR();
private:
    TestSite_CORR& operator=(const TestSite_CORR& );
    TestSite_CORR(const TestSite_CORR& );
};



#endif//_MY_CORR_CALCUALTE_H_
