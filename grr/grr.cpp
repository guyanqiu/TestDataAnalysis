#include "grr.h"
#include <cmath>
#include <fstream>
#include <QString>

#include "xlsxdocument.h"
#include "xlsxdocument.h"
#include "xlsxabstractsheet.h"
#include "xlsxformat.h"

const double k_factor = 6.0; //99.73%
const unsigned int MAX_SUBGROUP_NUMBER = 21;//g
const unsigned int MAX_SUBGROUP_SIZE = 20;//m
const double d2[21][19] = /* Row Number: Number of Subgroups(g), Column Number: Subgroup Size(m) */
{
        /* g */ /* m: 2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20 */
        /*  1 */{1.41421, 1.91155, 2.23887, 2.48124, 2.67253, 2.82981, 2.96288, 3.07794, 3.17905, 3.26909, 3.35016, 3.42378, 3.49116, 3.55333, 3.61071, 3.66422, 3.71424, 3.76118, 3.80537},
        /*  2 */{1.27931, 1.80538, 2.15069, 2.40484, 2.60438, 2.76779, 2.90562, 3.02446, 3.12869, 3.22134, 3.30463, 3.38017, 3.44922, 3.51287, 3.57156, 3.62625, 3.67734, 3.72524, 3.77032},
        /*  3 */{1.23105, 1.76858, 2.12049, 2.37883, 2.58127, 2.74681, 2.88628, 3.00643, 3.11173, 3.20526, 3.28931, 3.36550, 3.43512, 3.49927, 3.55842, 3.61351, 3.66495, 3.71319, 3.75857},
        /*  4 */{1.20621, 1.74989, 2.10522, 2.36571, 2.56964, 2.73626, 2.87656, 2.99737, 3.10321, 3.19720, 3.28163, 3.35815, 3.42805, 3.49246, 3.55183, 3.60712, 3.65875, 3.70715, 3.75268},
        /*  5 */{1.19105, 1.73857, 2.09601, 2.35781, 2.56263, 2.72991, 2.87071, 2.99192, 3.09808, 3.19235, 3.27701, 3.35372, 3.42381, 3.48836, 3.54787, 3.60328, 3.65502, 3.70352, 3.74900},
        /*  6 */{1.18083, 1.73099, 2.08985, 2.35253, 2.55795, 2.72567, 2.86680, 2.98829, 3.09467, 3.18911, 3.27392, 3.35077, 3.42097, 3.48563, 3.54522, 3.60072, 3.65253, 3.70109, 3.74678},
        /*  7 */{1.17348, 1.72555, 2.08543, 2.34875, 2.55460, 2.72263, 2.86401, 2.98568, 3.09222, 3.18679, 3.27172, 3.34866, 3.41894, 3.48368, 3.54333, 3.59888, 3.65075, 3.69936, 3.74509},
        /*  8 */{1.16794, 1.72147, 2.08212, 2.34591, 2.55208, 2.72036, 2.86192, 2.98373, 3.09039, 3.18506, 3.27006, 3.34708, 3.41742, 3.48221, 3.54192, 3.59751, 3.64941, 3.69806, 3.74382},
        /*  9 */{1.16361, 1.71828, 2.07953, 2.34370, 2.55013, 2.71858, 2.86028, 2.98221, 3.08896, 3.18370, 3.26878, 3.34585, 3.41624, 3.48107, 3.54081, 3.59644, 3.64838, 3.69705, 3.74284},
        /* 10 */{1.16014, 1.71573, 2.07746, 2.34192, 2.54856, 2.71717, 2.85898, 2.98100, 3.08781, 3.18262, 3.26775, 3.34486, 3.41529, 3.48016, 3.53993, 3.59559, 3.64755, 3.69625, 3.74205},
        /* 11 */{1.15729, 1.71363, 2.07577, 2.34048, 2.54728, 2.71600, 2.85791, 2.98000, 3.08688, 3.18174, 3.26690, 3.34406, 3.41452, 3.47941, 3.53921, 3.59489, 3.64687, 3.69558, 3.74141},
        /* 12 */{1.15490, 1.71189, 2.07436, 2.33927, 2.54621, 2.71504, 2.85702, 2.97917, 3.08610, 3.18100, 3.26620, 3.34339, 3.41387, 3.47879, 3.53861, 3.59430, 3.64630, 3.69503, 3.74087},
        /* 13 */{1.15289, 1.71041, 2.07316, 2.33824, 2.54530, 2.71422, 2.85627, 2.97847, 3.08544, 3.18037, 3.26561, 3.34282, 3.41333, 3.47826, 3.53810, 3.59381, 3.64582, 3.69457, 3.74041},
        /* 14 */{1.15115, 1.70914, 2.07213, 2.33737, 2.54452, 2.71351, 2.85562, 2.97787, 3.08487, 3.17984, 3.26510, 3.34233, 3.41286, 3.47781, 3.53766, 3.59339, 3.64541, 3.69417, 3.74002},
        /* 15 */{1.14965, 1.70804, 2.07125, 2.33661, 2.54385, 2.71290, 2.85506, 2.97735, 3.08438, 3.17938, 3.26465, 3.34191, 3.41245, 3.47742, 3.53728, 3.59302, 3.64505, 3.69382, 3.73969},
        /* 16 */{1.14833, 1.70708, 2.07047, 2.33594, 2.54326, 2.71237, 2.85457, 2.97689, 3.08395, 3.17897, 3.26427, 3.34154, 3.41210, 3.47707, 3.53695, 3.59270, 3.64474, 3.69351, 3.73939},
        /* 17 */{1.14717, 1.70623, 2.06978, 2.33535, 2.54274, 2.71190, 2.85413, 2.97649, 3.08358, 3.17861, 3.26393, 3.34121, 3.41178, 3.47677, 3.53666, 3.59242, 3.64447, 3.69325, 3.73913},
        /* 18 */{1.14613, 1.70547, 2.06917, 2.33483, 2.54228, 2.71148, 2.85375, 2.97613, 3.08324, 3.17829, 3.26362, 3.34092, 3.41150, 3.47650, 3.53640, 3.59216, 3.64422, 3.69301, 3.73890},
        /* 19 */{1.14520, 1.70480, 2.06862, 2.33436, 2.54187, 2.71111, 2.85341, 2.97581, 3.08294, 3.17801, 3.26335, 3.34066, 3.41125, 3.47626, 3.53617, 3.59194, 3.64400, 3.69280, 3.73869},
        /* 20 */{1.14437, 1.70419, 2.06813, 2.33394, 2.54149, 2.71077, 2.85310, 2.97552, 3.08267, 3.17775, 3.26311, 3.34042, 3.41103, 3.47605, 3.53596, 3.59174, 3.64380, 3.69260, 3.73850},
        /* 21 */{1.12838, 1.69257, 2.05875, 2.32593, 2.53441, 2.70436, 2.84720, 2.97003, 3.07751, 3.17287, 3.25846, 3.33598, 3.40676, 3.47193, 3.53198, 3.58788, 3.64006, 3.68896, 3.73500}
};

double c4[26] = {0.0000,
                 0.0000, 0.7979, 0.8862, 0.9213, 0.9400,
                 0.9515, 0.9594, 0.9650, 0.9693, 0.9727,
                 0.9754, 0.9776, 0.9794, 0.9810, 0.9823,
                 0.9835, 0.9845, 0.9854, 0.9862, 0.9869,
                 0.9876, 0.9882, 0.9887, 0.9892, 0.9896};

//static std::wstring convert(const char* str)
//{
//    QString temp = QString::fromLocal8Bit(str);
//    return std::wstring(reinterpret_cast<const wchar_t *>(temp.utf16()));
//}

static double betacdf(double x, double a, double b)
{
    double myeps = 2.2204e-16;
    double tiny = 1.0e-30;
    if (x < 0.0 || x > 1.0) return 1.0/0.0;
    if (x > (a+1.0)/(a+b+2.0)) return (1.0-betacdf(1.0-x, b,a));

    double temp = lgamma(a)+lgamma(b)-lgamma(a+b);
    double y = exp(log(x)*a+log(1.0-x)*b-temp) / a;
    double f = 1.0, c = 1.0, d = 0.0;

    int i, m;
    for (i = 0; i <= 400; ++i)
    {
        m = i/2;
        double numerator;
        if (i == 0) numerator = 1.0;
        else if (i % 2 == 0) numerator = (m*(b-m)*x)/((a+2.0*m-1.0)*(a+2.0*m));
        else  numerator = -((a+m)*(a+b+m)*x)/((a+2.0*m)*(a+2.0*m+1));


        d = 1.0 + numerator * d;
        if (fabs(d) < tiny) d = tiny;
        d = 1.0 / d;
        c = 1.0 + numerator / c;
        if (fabs(c) < tiny) c = tiny;

        const double cd = c*d;
        f *= cd;

        if (fabs(1.0-cd) < myeps)  return y * (f-1.0);
    }
    return 1.0e35;
}

static  double betapdf(double x, double a, double b)
{
     double temp = lgamma(a+b) - lgamma(a) - lgamma(b);
     if(x>0 && x< 1 && a > 0 && b > 0 && (a!=1 || b!= 1))
     return exp((a-1)*log(x) + (b-1)*log(1-x) + temp);

     if(x == 0 && a == 1 && b > 0 && b != 1)
     return exp(temp);

     if(x==1 && b == 1 && a>0 && a!=1)
     return exp(temp);

     if(x >= 0 && x <= 1 && a==1 && b== 1)
     return 1.0;

     else return 1.0;
}

static double betainv(double x, double a, double b)
{
     double myeps = 2.2204e-16;
     double y = a / (a+b);

     if(y < myeps) y = sqrt(myeps);
     if(y > 1- myeps) y = 1 - sqrt(myeps);

     double y_new = y;
     double h = 0.0;
     double y_old = y_new;
     for(int i = 0; i <= 40; i++)
     {
        y_old = y_new;
        double cdf = betacdf(y_old, a, b);
        double pdf = betapdf(y_old,a,b);
        h = (cdf - x)/ pdf;
        y_new = y_old - h;
        if(y_new <= myeps)
            y_new = y_old / 10;
        if(y_new >= 1.0 - myeps)
            y_new = 1-(1-y_old)/10;
        h = y_old - y_new;
        if(fabs(h) < sqrt(myeps)) break;
     }
     return y_new;
}

static double finv(double x, int m, int n)
{
    return ((1.0/betainv(1-x, n/2.0, m/2.0)-1)*1.0*n/m);
}


TestItem_GRR::TestItem_GRR()
{
    m_mode = 0;
    m_site_count = 0;
    m_data_count = 0;
    m_number = 0;
    m_high_spec = 0.0;
    m_low_spec = 0.0;

    m_max_average = 0.0;
    m_min_average = 0.0;

    m_sum_range = 0.0;
    m_sum_variance = 0.0;

    m_highspec_valid = false;
    m_lowspec_valid = false;

    m_default_high_spec = 9999.9;
    m_default_low_spec = -9999.9;

    m_measure_guard_band = 0.0;
    m_tcs_error = 0.0;
    m_tcs = 0.0;

    m_anova_mse = 0.0;
    m_anova_msa = 0.0;
    m_anova_ev = 0.0;
    m_anova_av = 0.0;
    m_anova_f = 0.0;
    m_anova_ftest = 0.0;
    m_anova_alpha = 0.0;
}

bool TestItem_GRR::add_item(const TestItem* item)
{
    if(!item) return false;

    double average = item->get_average();
    double stdev = item->get_stdev();
    double range = item->get_range();
    unsigned int data_count = item->get_result_count();
    if(data_count <= 2) return false;

    if(m_site_count == 0)
    {
        m_label.assign(item->get_label());
        m_number = item->get_number();

        const char* str_unit = item->get_unit();
        if(str_unit) m_unit.assign(str_unit);

        if(item->lowlimit_valid())
        {
            m_low_spec = item->get_lowlimit();
            m_lowspec_valid = true;
        }
        if(item->highlimit_valid())
        {
            m_high_spec = item->get_highlimit();
            m_highspec_valid = true;
        }
        m_max_average = average;
        m_min_average = average;
        m_data_count = data_count;
    }
    else
    {
        if(data_count != m_data_count) return false;

        if(m_max_average < average) m_max_average = average;
        if(m_min_average > average) m_min_average = average;
    }
    m_sum_range += range;
    m_sum_variance += (stdev*stdev);
    m_site_count ++;

    return true;
}

void TestItem_GRR::set_calculate_mode(int mode)
{
    m_mode = mode;
}

unsigned int TestItem_GRR::get_number() const
{
    return m_number;
}
const char* TestItem_GRR::get_label() const
{
    return m_label.c_str();
}

const char* TestItem_GRR::get_unit() const
{
    if(m_unit.empty()) return nullptr;
    return m_unit.c_str();
}

void TestItem_GRR::set_default_lowspec(double low_spec)
{
       m_default_low_spec = low_spec;
}

void TestItem_GRR::set_default_highspec(double high_spec)
{
        m_default_high_spec = high_spec;
}

double TestItem_GRR::get_highspec() const
{
    if(m_highspec_valid)  return m_high_spec;
    else return m_default_high_spec;
}

double TestItem_GRR::get_lowspec() const
{
    if(m_lowspec_valid) return m_low_spec;
    else return m_default_low_spec;
}

unsigned int TestItem_GRR::get_site_count() const
{
    return m_site_count;
}

unsigned int TestItem_GRR::get_data_count() const
{
    return m_data_count;
}

double TestItem_GRR::get_tolerance() const
{
    double high_spec = get_highspec();
    double low_spec = get_lowspec();
    return (high_spec - low_spec);
}

double TestItem_GRR::get_max_average() const
{
    return m_max_average;
}

double TestItem_GRR::get_min_average() const
{
    return m_min_average;
}

double TestItem_GRR::get_average_diff() const
{
    return (m_max_average - m_min_average);
}

double TestItem_GRR::get_range_bar() const
{
    if(m_site_count == 0) return 0.0;
    else return (m_sum_range/m_site_count);
}

double TestItem_GRR::get_stdev_bar() const
{
    if(m_site_count == 0) return 0.0;
    else return std::sqrt(m_sum_variance/m_site_count);
}

double TestItem_GRR::get_repeatability() const
{
    if(m_site_count <= 1) return 0.0;

    if(m_mode == 0)
    {
        double stdev_avg = get_stdev_bar();
        return k_factor * stdev_avg;
    }
    else if(m_mode == 1)
    {
        unsigned int g = m_site_count;
        unsigned int m = m_data_count;
        if(g > MAX_SUBGROUP_NUMBER) g = MAX_SUBGROUP_NUMBER;
        if(m > MAX_SUBGROUP_SIZE) m = MAX_SUBGROUP_SIZE;
        double k1 = 1.0 / d2[g - 1][m - 2];
        double range_avg = get_range_bar();
        return (k_factor * k1 * range_avg);
    }
    else
    {
        return m_anova_ev;
    }
}

double TestItem_GRR::get_reproducibility() const
{
    if(m_site_count <= 1) return 0.0;

    if(m_mode == 0 || m_mode == 1)
    {
        unsigned int g = 1;
        unsigned int m = m_site_count;
        if(m > MAX_SUBGROUP_SIZE) m = MAX_SUBGROUP_SIZE;
        double k2 = 1.0/d2[g-1][m-2];

        double ev = get_repeatability();
        double avg_diff = get_average_diff();
        double av = (k_factor * avg_diff * k2) * (k_factor * avg_diff * k2) ;
        av = av - ev * ev / m_data_count;
        if(av < 0) return 0.0;
        else return std::sqrt(av);
    }
    else
    {
        return m_anova_av;
    }
}

double TestItem_GRR::get_r_r() const
{
    double av = get_reproducibility();
    double ev = get_repeatability();
    return std::sqrt(av * av + ev * ev);
}

double TestItem_GRR::get_gr_r() const
{
        double t = get_tolerance();
        if(t == 0) return 1.0;
        double rr = get_r_r();
        double gr_r = rr/t;
        if(gr_r > 1.0) gr_r = 1.0;
        return gr_r;
}

void TestItem_GRR::set_anova(double anova_mse, double anova_msa, double anova_alpha)
{
    m_anova_msa = anova_msa;
    m_anova_mse = anova_mse;
    m_anova_alpha = anova_alpha;
    if(m_anova_msa < 0) m_anova_msa = 0.0;
    if(m_anova_mse < 0) m_anova_mse = 0.0;
    m_anova_ev = k_factor * sqrt(m_anova_mse);
    m_anova_av = k_factor * sqrt(m_anova_msa/m_data_count);
    if(m_anova_mse == 0) m_anova_f = 0.0;
    else m_anova_f = m_anova_msa / m_anova_mse;
    m_anova_ftest = finv(1.0- m_anova_alpha, m_site_count-1, m_site_count*m_data_count - m_site_count);
}

double TestItem_GRR::get_anova_f() const
{
    return m_anova_f;
}
double TestItem_GRR::get_anova_ftest() const
{
    return m_anova_ftest;
}


void TestItem_GRR::set_tcs_error(double tcs_error)
{
    m_tcs_error = tcs_error;
    m_measure_guard_band = 3.0 * tcs_error;


    double t = 0.0;
    if(m_highspec_valid && m_lowspec_valid)
    {
        t = m_high_spec - m_low_spec;
        if(t == 0) m_tcs = 1.0;
        else m_tcs = 6.0 * tcs_error / t;
    }
    else if(m_highspec_valid)
    {
        t = fabs(m_high_spec);
        if(t == 0) m_tcs = 1.0;
        else m_tcs = 3.0 * tcs_error / t;
    }
    else if(m_lowspec_valid)
    {
        t = fabs(m_low_spec);
        if(t == 0) m_tcs = 1.0;
        else m_tcs = 3.0 * tcs_error / t;
    }
    else
    {
        m_tcs = 0.0;
    }
}

double TestItem_GRR::get_measure_guard_band() const
{
    return m_measure_guard_band;
}

double TestItem_GRR::get_tcs_error() const
{
    return m_tcs_error;
}
double TestItem_GRR::get_tcs() const
{
    return m_tcs;
}

TestItem_GRR::~TestItem_GRR()
{
}

TestSite_GRR::TestSite_GRR(double default_low_spec, double default_high_spec, int mode)
{
    m_high_spec = default_high_spec;
    m_low_spec = default_low_spec;
    m_mode = mode;
    m_site_count = 0;
    m_data_count = 0;
    m_calculate_ok = false;
    m_anova_alpha = 0.05;
}

TestSite_GRR::TestSite_GRR()
{
    m_high_spec = 9999.9;
    m_low_spec = -9999.9;
    m_mode = 0;
    m_site_count = 0;
    m_data_count = 0;
    m_calculate_ok = false;
    m_anova_alpha = 0.05;
}

TestSite_GRR::~TestSite_GRR()
{
    clear();
}

void TestSite_GRR::set_default_limit(double default_low_spec, double default_high_spec)
{
    m_high_spec = default_high_spec;
    m_low_spec = default_low_spec;
}

void TestSite_GRR::set_calculate_mode(int mode)
{
    m_mode = mode;
}

void TestSite_GRR::clear()
{
    m_site_list.clear();
    m_grr_list.clear();
    m_site_count = 0;
    m_data_count = 0;
    m_item_count = 0;
    m_calculate_ok = false;
}

GRR_CALCULATE_ERROR TestSite_GRR::add_site(const TestSite* site)
{
    if(!site) return GRR_SITE_NOT_EXIST;

    unsigned int item_count = site->get_item_count();
    if(item_count == 0) return GRR_ITEM_NOT_EXIST;

    unsigned int device_count = site->get_device_count();
    if(device_count <= 2) return GRR_DATACOUNT_TOO_FEW;


    for(unsigned int i = 0; i < item_count; i++)
    {
        TestItem* item = site->get_item(i);
        bool all_pass = item->is_all_pass();
        if(!all_pass) return GRR_DATA_NOT_ALL_PASS;

        unsigned int result_count = item->get_result_count();
        if(result_count != device_count) return GRR_NOT_ALL_ITEM_TESTED;
    }

    unsigned int number_not_same_count = 0;
    unsigned int label_not_same_count = 0;
    if(m_site_count == 0)
    {
        m_data_count = device_count;
        m_item_count = item_count;
        for(unsigned int i = 0; i < item_count; i++)
        {
            TestItem* item = site->get_item(i);
            std::string label(item->get_label());
            unsigned int number = item->get_number();

            m_number_list.push_back(number);
            m_label_list.push_back(label);
        }
    }
    else
    {
        if(m_data_count != device_count) return GRR_DATACOUNT_NOT_SAME;
        if(m_item_count != item_count) return GRR_ITEMCOUNT_NOT_SAME;

        for(unsigned int i = 0; i < item_count; i++)
        {
            TestItem* item = site->get_item(i);
            std::string label(item->get_label());
            unsigned int number = item->get_number();

            if(number != m_number_list[i]) number_not_same_count++;
            if(label != m_label_list[i]) label_not_same_count++;
        }
    }
    m_site_list.push_back(site);
    m_site_count++;

    if(number_not_same_count > 0 && label_not_same_count > 0)
        return GRR_LABEL_AND_NUMBER_NOT_SAME;
    else if(number_not_same_count > 0)
        return GRR_NUMBER_NOT_SAME;
    else if(label_not_same_count > 0)
        return GRR_LABEL_NOT_SAME;
    else
        return GRR_RESULT_IS_OK;
}

GRR_CALCULATE_ERROR TestSite_GRR::calculate()
{
    if(m_site_count < 2) return GRR_SITECOUNT_TOO_FEW;
    if(m_item_count == 0) return GRR_ITEM_NOT_EXIST;
    if(m_data_count < 2) return GRR_DATACOUNT_TOO_FEW;

    double c4_coeff = 1.0;
    if(m_site_count <= 25) c4_coeff = c4[m_site_count];

    for(unsigned int i = 0; i < m_item_count; i++)
    {
        TestItem_GRR grr;
        grr.set_calculate_mode(m_mode);
        grr.set_default_highspec(m_high_spec);
        grr.set_default_lowspec(m_low_spec);

        std::vector<double> sums(m_data_count, 0.0);
        std::vector<double> sums_sq(m_data_count, 0.0);

        double ANOVA_R = 0.0, ANOVA_P=0.0, ANOVA_Q=0.0;

        for(unsigned int s = 0; s < m_site_count; s++)
        {
            const TestSite* site = m_site_list[s];
            TestItem* item = site->get_item(i);
            grr.add_item(item);

            ANOVA_R += item->get_sum_squares();
            double sum_per_site = item->get_sum();
            ANOVA_Q += (sum_per_site*sum_per_site/m_data_count);
            ANOVA_P += sum_per_site;

            for(unsigned int d = 0; d <m_data_count; d++)
            {
                TestResult result = item->get_result(d);
                double temp_value = result.get_value();
                sums[d] += temp_value;
                sums_sq[d] += (temp_value*temp_value);
            }
        }
        ANOVA_P = (ANOVA_P*ANOVA_P)/(m_data_count*m_site_count);

        double sum_stdev = 0.0;
        for(unsigned int d = 0; d < m_data_count; d++)
        {
            double temp = sums_sq[d] - sums[d]*sums[d]/m_site_count;
            if(temp < 0.0) temp = 0.0;
            else temp = std::sqrt(temp/(m_site_count-1));
            sum_stdev += temp;
        }
        double tcs_error = sum_stdev / (m_data_count*c4_coeff);
        grr.set_tcs_error(tcs_error);

        double ANOVA_Se = ANOVA_R - ANOVA_Q;
        double ANOVA_Sa = ANOVA_Q - ANOVA_P;
        double ANOVA_MSe = ANOVA_Se/(m_data_count*m_site_count - m_site_count);
        double ANOVA_MSa = ANOVA_Sa /(m_site_count - 1);

        grr.set_anova(ANOVA_MSe,ANOVA_MSa,m_anova_alpha );

        m_grr_list.push_back(grr);
    }
    m_calculate_ok = true;

    return GRR_RESULT_IS_OK;
}

const TestSite* TestSite_GRR::get_site(unsigned int index) const
{
    return m_site_list[index];
}

TestItem_GRR TestSite_GRR::get_grr(unsigned int index) const
{
    return m_grr_list[index];
}

unsigned int TestSite_GRR::get_data_count() const
{
    return m_data_count;
}

unsigned int TestSite_GRR::get_site_count() const
{
    return m_site_count;
}

unsigned int TestSite_GRR::get_item_count() const
{
    return m_item_count;
}

bool TestSite_GRR::save_result_xls(const char* filename)
{
    if(!m_calculate_ok) return false;
    QXlsx::Document xlsx;
    xlsx.addSheet("GRR Result");
    QString qfilename = QString::fromLocal8Bit(filename);

    unsigned int row = 1, col = 1;
    xlsx.write(row, col++, "TestNumber");
    xlsx.write(row, col++, "TestLabel");
    xlsx.write(row, col++, "Unit");
    xlsx.write(row, col++, "HighSpec");
    xlsx.write(row, col++, "LowSpec");
    xlsx.write(row, col++, "MaxAvg");
    xlsx.write(row, col++, "MinAvg");
    xlsx.write(row, col++, "RxDiff");
    xlsx.write(row, col++, "StDevAvg");
    xlsx.write(row, col++, "EV");
    xlsx.write(row, col++, "AV");
    xlsx.write(row, col++, "R&R");
    xlsx.write(row, col++, "GR&R");
    xlsx.write(row, col++, "---");
    xlsx.write(row, col++, "TCS-Error");
    xlsx.write(row, col++, "MGB");
    xlsx.write(row, col++, "TCS");
    xlsx.write(row, col++, "---");
    xlsx.write(row, col++, "F(alpha=0.05)");
    xlsx.write(row, col++, "F");

    row = 2;
    for(unsigned int i = 0; i < m_item_count; i++)
    {
        TestItem_GRR grr = m_grr_list[i];
        QString label = QString::fromLocal8Bit(grr.get_label());
        QString unit = QString::fromLocal8Bit(grr.get_unit());
        col = 1;
        double grr_value = grr.get_gr_r();
        if(grr_value >= 1.0) grr_value = 1.0;
        xlsx.write(row, col++, grr.get_number());
        xlsx.write( row, col++, label);
        xlsx.write( row, col++, unit);
        xlsx.write(row, col++, grr.get_highspec());
        xlsx.write(row, col++, grr.get_lowspec());
        xlsx.write(row, col++, grr.get_max_average());
        xlsx.write(row, col++, grr.get_min_average());
        xlsx.write(row, col++, grr.get_average_diff());
        xlsx.write(row, col++, grr.get_stdev_bar());
        xlsx.write(row, col++, grr.get_repeatability());
        xlsx.write(row, col++, grr.get_reproducibility());
        xlsx.write(row, col++, grr.get_r_r());

        QXlsx::Format format_red;
        format_red.setPatternBackgroundColor(QColor(Qt::red));
        format_red.setNumberFormat("0.00%");

        QXlsx::Format format_green;
        format_green.setPatternBackgroundColor(QColor(Qt::green));
        format_green.setNumberFormat("0.00%");

        QXlsx::Format format_yellow;
        format_yellow.setPatternBackgroundColor(QColor(Qt::yellow));
        format_yellow.setNumberFormat("0.00%");

        QXlsx::Format format_yellow2;
        format_yellow2.setPatternBackgroundColor(QColor(Qt::yellow));
        format_yellow2.setNumberFormat("0.00");

        if(grr_value <= 0.1) xlsx.write(row, col++, grr_value, format_green);
        else if(grr_value > 0.3) xlsx.write(row, col++, grr_value, format_red);
        else xlsx.write(row, col++, grr_value, format_yellow);

        col++;
        xlsx.write(row, col++, grr.get_tcs_error());
        xlsx.write(row, col++, grr.get_measure_guard_band());

        double tcs_value = grr.get_tcs();
        if(tcs_value <= 0.1) xlsx.write(row, col++, tcs_value, format_green);
        else if(tcs_value > 0.3) xlsx.write(row, col++, tcs_value, format_red);
        else xlsx.write(row, col++, tcs_value, format_yellow);

        col++;
        double ftest = grr.get_anova_ftest();
        double f = grr.get_anova_f();
        xlsx.write(row, col++, ftest);
        if(f > ftest) xlsx.write(row, col++, f, format_yellow2 );
        else xlsx.write(row, col++, f);

        row++;
        col++;
    }
    for(unsigned int i = 0; i < m_site_count; i++)
    {
        const TestSite *site = m_site_list[i];
        QString wname = QString::fromLocal8Bit(site->get_name());

        int start_col = col;
        row = 1;
        xlsx.write(row, col++, wname);
        xlsx.write(row, col++, "Max");
        xlsx.write(row, col++, "Min");
        xlsx.write(row, col++, "Range");
        xlsx.write(row, col++, "Average");
        xlsx.write(row, col++, "Stdev");

        row = 2;

        QXlsx::Format formats[5];
        formats[0].setPatternBackgroundColor(QColor(147, 197, 251));
        formats[1].setPatternBackgroundColor(QColor(218, 254, 251));
        formats[2].setPatternBackgroundColor(QColor(249, 187, 248));
        formats[3].setPatternBackgroundColor(QColor(249, 249, 183));
        formats[4].setPatternBackgroundColor(QColor(203, 207, 214));

        for(unsigned int n = 0; n < m_item_count; n++)
        {
            const TestItem* item = site->get_item(n);

            col = start_col + 1;

            xlsx.write(row, col++, item->get_max(), formats[0]);
            xlsx.write(row, col++, item->get_min(), formats[1]);
            xlsx.write(row, col++, item->get_range(), formats[2]);
            xlsx.write(row, col++, item->get_average(), formats[3]);
            xlsx.write(row, col++, item->get_stdev(), formats[4]);


            row++;
        }
    }
    bool result = xlsx.saveAs(qfilename);
    return result;
}

bool TestSite_GRR::save_result_csv(const char* filename)
{
    if(!m_calculate_ok) return false;
    std::ofstream out(filename, std::ios::out );
    if(!out) return false;

    out<<"TestNumber,TestLabel,HighSpec,LowSpec,Tolerance,Unit,MaxAvg,MinAvg,Xdiff,StDevAvg,EV,AV,R&R,GR&R,,";
    out<<"TCS-Error,MGB,TCS,,";
    out<<"F(alpha=0.05),F"<<std::endl;
    for(unsigned int i = 0; i < m_item_count; i++)
    {
        TestItem_GRR grr = m_grr_list[i];
        out<<grr.get_number()<<",";
        out<<"\""<<grr.get_label()<<"\",";
        out<<grr.get_highspec()<<",";
        out<<grr.get_lowspec()<<",";
        out<<grr.get_tolerance()<<",";
        const char* str_unit = grr.get_unit();
        if(str_unit) out<<"\""<<str_unit<<"\",";
        else out<<",";
        out<< grr.get_max_average()<<",";
        out<<grr.get_min_average()<<",";
        out<<grr.get_average_diff()<<",";
        out<<grr.get_stdev_bar()<<",";
        out<<grr.get_repeatability()<<",";
        out<<grr.get_reproducibility()<<",";
        out<<grr.get_r_r()<<",";
        out<<100.0*grr.get_gr_r()<<"%"<<",,";
        out<<grr.get_tcs_error()<<",";
        out<<grr.get_measure_guard_band()<<",";
        out<<100.0*grr.get_tcs()<<"%"<<",,";
        out<<grr.get_anova_ftest()<<",";
        out<<grr.get_anova_f()<<std::endl;
    }

    out<<std::endl;

    for(unsigned int i = 0; i < m_site_count; i++)
    {
        const TestSite *site = m_site_list[i];
        out<< site->get_name()<<std::endl;
        out<<"TestNumber,TestLabel,Max,Min,Range,Average,Stdev"<<std::endl;

        for(unsigned int n = 0; n < m_item_count; n++)
        {
            const TestItem* item = site->get_item(n);
            out<<item->get_number()<<",";
            out<<"\""<<item->get_label()<<"\",";
            out<<item->get_max()<<",";
            out<<item->get_min()<<",";
            out<<item->get_range()<<",";
            out<<item->get_average()<<",";
            out<<item->get_stdev()<<std::endl;
        }
    }
    out.close();
    return true;
}
