#include "sqlite/sqlite3.h"

#include <iostream>
#include <string>
#include <cstring>
#include <sstream>
#include <ctime>
#include <fstream>
#include <locale>
#include <ShlObj.h>
#include <algorithm>
#include <vector>
#include <filesystem>

#pragma comment(lib, "sqlite3.lib")

#define MAXN 200
#define MAXP 10000

using namespace std;

// Base64 encode and decode from http://www.adp-gmbh.ch/cpp/common/base64.html
static const std::string base64_chars = "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
"abcdefghijklmnopqrstuvwxyz""0123456789+/";
static inline bool is_base64(unsigned char c) {
    return (isalnum(c) || (c == '+') || (c == '/'));
}
std::string base64_encode(unsigned char const* bytes_to_encode, unsigned int in_len) {
    std::string ret;
    int i = 0;
    int j = 0;
    unsigned char char_array_3[3];
    unsigned char char_array_4[4];

    while (in_len--) {
        char_array_3[i++] = *(bytes_to_encode++);
        if (i == 3) {
            char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
            char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
            char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);
            char_array_4[3] = char_array_3[2] & 0x3f;

            for (i = 0; (i < 4); i++)
                ret += base64_chars[char_array_4[i]];
            i = 0;
        }
    }

    if (i)
    {
        for (j = i; j < 3; j++)
            char_array_3[j] = '\0';

        char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
        char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
        char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);
        char_array_4[3] = char_array_3[2] & 0x3f;

        for (j = 0; (j < i + 1); j++)
            ret += base64_chars[char_array_4[j]];

        while ((i++ < 3))
            ret += '=';

    }

    return ret;

}
std::string base64_decode(std::string const& encoded_string) {
    int in_len = encoded_string.size();
    int i = 0;
    int j = 0;
    int in_ = 0;
    unsigned char char_array_4[4], char_array_3[3];
    std::string ret;

    while (in_len-- && (encoded_string[in_] != '=') && is_base64(encoded_string[in_])) {
        char_array_4[i++] = encoded_string[in_]; in_++;
        if (i == 4) {
            for (i = 0; i < 4; i++)
                char_array_4[i] = base64_chars.find(char_array_4[i]);

            char_array_3[0] = (char_array_4[0] << 2) + ((char_array_4[1] & 0x30) >> 4);
            char_array_3[1] = ((char_array_4[1] & 0xf) << 4) + ((char_array_4[2] & 0x3c) >> 2);
            char_array_3[2] = ((char_array_4[2] & 0x3) << 6) + char_array_4[3];

            for (i = 0; (i < 3); i++)
                ret += char_array_3[i];
            i = 0;
        }
    }

    if (i) {
        for (j = i; j < 4; j++)
            char_array_4[j] = 0;

        for (j = 0; j < 4; j++)
            char_array_4[j] = base64_chars.find(char_array_4[j]);

        char_array_3[0] = (char_array_4[0] << 2) + ((char_array_4[1] & 0x30) >> 4);
        char_array_3[1] = ((char_array_4[1] & 0xf) << 4) + ((char_array_4[2] & 0x3c) >> 2);
        char_array_3[2] = ((char_array_4[2] & 0x3) << 6) + char_array_4[3];

        for (j = 0; (j < i - 1); j++) ret += char_array_3[j];
    }

    return ret;
}

// Base convert from https://www.geeksforgeeks.org/convert-base-decimal-vice-versa/
int val(char c)
{
    if (c >= '0' && c <= '9')
        return (int)c - '0';
    else
        return (int)c - 'a' + 10;
}
int toDecimal(string str, int base)
{
    int len = str.length();
    int power = 1;
    int num = 0;
    int i;
    for (i = len - 1; i >= 0; i--)
    {
        num += val(str[i]) * power;
        power = power * base;
    }
    return num;
}

// String to Wstring  
wstring s2ws(string str)
{
    wstring result;
    int len = MultiByteToWideChar(CP_ACP, 0, str.c_str(), str.size(), NULL, 0);
    TCHAR* buffer = new TCHAR[len + 1];
    MultiByteToWideChar(CP_ACP, 0, str.c_str(), str.size(), buffer, len);
    buffer[len] = '\0';
    result.append(buffer);
    delete[] buffer;
    return result;
}
// Wstring to String  
string ws2s(wstring wstr)
{
    string result;
    int len = WideCharToMultiByte(CP_ACP, 0, wstr.c_str(), wstr.size(), NULL, 0, NULL, NULL);
    char* buffer = new char[len + 1];
    WideCharToMultiByte(CP_ACP, 0, wstr.c_str(), wstr.size(), buffer, len, NULL, NULL);
    buffer[len] = '\0';
    result.append(buffer);
    delete[] buffer;
    return result;
}
string i2s(int n)
{
    stringstream ss;
    ss.imbue(std::locale("C"));
    ss << n;
    return ss.str();
}

// https://stackoverflow.com/questions/875249/how-to-get-current-directory
std::wstring ExePath()
{
    TCHAR buffer[MAX_PATH] = { 0 };
    GetModuleFileName(NULL, buffer, MAX_PATH);
    std::wstring::size_type pos = std::wstring(buffer).find_last_of(L"\\/");
    return std::wstring(buffer).substr(0, pos);
}

sqlite3* db;
char* zErrMsg = 0;
string _CALLBACK_;
static int callback(void* data, int argc, char** argv, char** azColName) 
{
    if (argc > 1)
    {
        wcout << "错误：预想的查询结果只有一条，而实际查询得到了两个或以上结果" << endl;
        system("pause");
        exit(0);
    }
    else if (argc < 0)
    {
        wcout << "错误：预想的查询结果只有一条，而实际查询没有结果" << endl;
        system("pause");
        exit(0);
    }
    _CALLBACK_ = argv[0];
    return 0;
}
string exec(string cmd)
{
    int rc = sqlite3_exec(db, cmd.c_str(), callback, NULL, &zErrMsg);
    if (rc != SQLITE_OK)
    {
        fprintf(stderr, "SQL error: %s\n", zErrMsg);
        system("pause");
        exit(0);
    }
    string tmp = _CALLBACK_;
    _CALLBACK_.clear();
    return tmp;
}


// Table from https://gamewith.jp/uma-musume/article/show/279308
int basicPoint[] = { 0,1,1,2,2,3,3,4,4,5,5,6,6,7,7,8,8,9,9,10,10,11,11,12,12,13,13,
14,14,15,15,16,16,17,17,18,18,19,19,20,20,21,21,22,22,23,23,24,24,25,25,26,27,28,29,
29,30,31,32,33,33,34,35,36,37,37,38,39,40,41,41,42,43,44,45,45,46,47,48,49,49,50,51,
52,53,53,54,55,56,57,57,58,59,60,61,61,62,63,64,65,66,67,68,69,70,71,72,73,74,75,76,
77,78,79,80,81,82,83,84,85,86,87,88,89,90,91,92,93,94,95,96,97,98,99,100,101,102,103,
104,105,106,107,108,109,110,111,112,113,114,115,116,117,118,120,121,122,124,125,126,
128,129,130,131,133,134,135,137,138,139,141,142,143,144,146,147,148,150,151,152,154,
155,156,157,159,160,161,163,164,165,167,168,169,170,172,173,174,176,177,178,180,181,
183,184,186,188,189,191,192,194,196,197,199,200,202,204,205,207,208,210,212,213,215,
216,218,220,221,223,224,226,228,229,231,232,234,236,237,239,240,242,244,245,247,248,
250,252,253,255,256,258,260,261,263,265,267,269,270,272,274,276,278,279,281,283,285,
287,288,290,292,294,296,297,299,301,303,305,306,308,310,312,314,315,317,319,321,323,
324,326,328,330,332,333,335,337,339,341,342,344,346,348,350,352,354,356,358,360,362,
364,366,368,371,373,375,377,379,381,383,385,387,389,392,394,396,398,400,402,404,406,
408,410,413,415,417,419,412,423,425,427,429,431,434,436,438,440,442,444,446,448,450,
452,455,457,459,462,464,467,469,471,474,476,479,481,483,486,488,491,493,495,498,500,
503,505,507,510,512,515,517,519,522,524,527,529,531,534,536,539,541,543,546,548,551,
553,555,558,560,563,565,567,570,572,575,577,580,582,585,588,590,593,595,598,601,603,
606,608,611,614,616,619,621,624,627,629,632,634,637,640,642,645,647,650,653,655,658,
660,663,666,668,671,673,676,679,681,684,686,689,692,694,697,699,702,705,707,710,713,
716,719,721,724,727,730,733,735,738,741,744,747,749,752,755,758,761,763,766,769,772,
775,777,780,783,786,789,791,794,797,800,803,805,808,811,814,817,819,822,825,828,831,
833,836,839,842,845,847,850,853,856,859,862,865,868,871,874,876,879,882,885,888,891,
894,897,900,903,905,908,911,914,917,920,923,926,929,931,934,937,940,943,946,949,952,
955,958,961,963,966,969,972,975,978,981,984,987,990,993,996,999,1002,1005,1008,1011,
1014,1017,1020,1023,1026,1029,1032,1035,1038,1041,1044,1047,1050,1053,1056,1059,1062,
1065,1068,1071,1074,1077,1080,1083,1086,1089,1092,1095,1098,1101,1104,1107,1110,1113,
1116,1119,1122,1125,1128,1131,1134,1137,1140,1143,1146,1149,1152,1155,1158,1161,1164,
1167,1171,1174,1177,1180,1183,1186,1189,1192,1195,1198,1202,1205,1208,1211,1214,1217,
1220,1223,1226,1229,1233,1236,1239,1242,1245,1248,1251,1254,1257,1260,1264,1267,1270,
1273,1276,1279,1282,1285,1288,1291,1295,1298,1301,1304,1308,1311,1314,1318,1321,1324,
1328,1331,1334,1337,1341,1344,1347,1351,1354,1357,1361,1364,1367,1370,1374,1377,1380,
1384,1387,1390,1394,1397,1400,1403,1407,1410,1413,1417,1420,1423,1427,1430,1433,1436,
1440,1443,1446,1450,1453,1456,1460,1463,1466,1470,1473,1477,1480,1483,1487,1490,1494,
1497,1500,1504,1507,1511,1514,1517,1521,1524,1528,1531,1534,1538,1541,1545,1548,1551,
1555,1558,1562,1565,1568,1572,1575,1579,1582,1585,1589,1592,1596,1596,1602,1606,1609,
1613,1616,1619,1623,1626,1630,1633,1637,1640,1644,1647,1651,1654,1658,1661,1665,1668,
1672,1675,1679,1682,1686,1689,1693,1696,1700,1703,1707,1710,1714,1717,1721,1724,1728,
1731,1735,1738,1742,1745,1749,1752,1756,1759,1763,1766,1770,1773,1777,1780,1784,1787,
1791,1794,1798,1801,1805,1808,1812,1816,1820,1824,1828,1832,1836,1840,1844,1847,1851,
1855,1859,1863,1867,1871,1875,1879,1883,1886,1890,1894,1898,1902,1906,1910,1914,1918,
1922,1925,1929,1933,1937,1941,1945,1949,1953,1957,1961,1964,1968,1972,1976,1980,1984,
1988,1992,1996,2000,2004,2008,2012,2016,2020,2024,2028,2032,2036,2041,2045,2049,2053,
2057,2061,2065,2069,2073,2077,2082,2086,2090,2094,2098,2102,2106,2110,2114,2118,2123,
2127,2131,2135,2139,2143,2147,2151,2155,2159,2164,2168,2172,2176,2180,2184,2188,2192,
2196,2200,2205,2209,2213,2217,2221,2226,2230,2234,2238,2242,2247,2251,2255,2259,2263,
2268,2272,2276,2280,2284,2289,2293,2297,2301,2305,2310,2314,2318,2322,2326,2331,2335,
2339,2343,2347,2352,2356,2360,2364,2368,2373,2377,2381,2385,2389,2394,2398,2402,2406,
2410,2415,2419,2423,2427,2432,2436,2440,2445,2449,2453,2458,2462,2466,2470,2475,2479,
2483,2488,2492,2496,2501,2505,2509,2513,2518,2522,2526,2531,2535,2539,2544,2548,2552,
2556,2561,2565,2569,2574,2578,2582,2587,2591,2595,2599,2604,2608,2612,2617,2621,2625,
2630,2635,2640,2645,2650,2656,2661,2666,2671,2676,2682,2687,2692,2697,2702,2708,2713,
2718,2723,2728,2734,2739,2744,2749,2754,2760,2765,2770,2775,2780,2786,2791,2796,2801,
2806,2812,2817,2822,2827,2832,2838,2843,2848,2853,2858,2864,2869,2874,2879,2884,2890,
2895,2901,2906,2912,2917,2923,2928,2934,2939,2945,2950,2956,2961,2967,2972,2978,2983,
2989,2994,3000,3005,3011,3016,3022,3027,3033,3038,3044,3049,3055,3060,3066,3071,3077,
3082,3088,3093,3099,3104,3110,3115,3121,3126,3132,3137,3143,3148,3154,3159,3165,3171,
3178,3184,3191,3198,3204,3211,3217,3224,3231,3237,3244,3250,3257,3264,3283,3277,3283,
3290,3297,3303,3310,3316,3323,3330,3336,3343,3349,3356,3363,3369,3376,3382,3389,3396,
3402,3409,3415,3422,3429,3435,3442,3448,3455,3462,3468,3475,3481,3488,3495,3501,3508,
3515,3522,3529,3535,3542,3549,3556,3563,3569,3576,3583,3590,3597,3603,3610,3617,3624,
3631,3637,3644,3651,3658,3665,3671,3678,3685,3692,3699,3705,3712,3719,3726,3733,3739,
3746,3753,3760,3767,3773,3780,3787,3794,3801,3807,3814,3821,3828,3835,3841 };
wstring rk = L"SABCDEFG";

string shareCode;
int prop[17];
/*
*  0-4: 5-dimension data
*  5,6: ground fitness
*  7-10 distance fitness
*  11-14 running style fitness
*  15: character star
*  16: rank of inherent skill
*/

int inherit = -1, skillPoint;
bool ifGlobalDiscount = false;


struct skill {
    string name;
    int id;
    int cost;
    int value;
    int rarity;
    int disp_order;
    int isML;   // 0: Normal, 1: ML skill, 2: Gold ML skill, 3: Normal Gold skill
    int inferior_id;
} skillList[MAXN];
int sidx = 0;

int dp[MAXP], w[MAXN], v[MAXN], pw[MAXN][2], pv[MAXN][2];
/*
*  To normal skill, w[i] store the pre-skill's cost, pw[i][0] store the gold skill's cost,
*  no meaning for pw[i][1]
*
*  To ML skill, w[i] store the single-circle one's cost, pw[i][0] store the double-cirlcle one's cost,
*  and pw[i][1] store the gold one's cost
*
*  Same concept to the v[i], pv[i][0], pv[i][1], while they store value point
*/
int idLog[MAXN*3];    // idLog[i] = skill id
bool vis[1011111];
vector<short> Log[MAXP];


double coeffi[] = { 1.1,0.9,0.8,0.7 };
double propCoeffi(int p)
{
    if (p == 0 or p == 1)   return coeffi[0];   // S,A
    if (p == 2 or p == 3)   return coeffi[1];   // B,C
    if (p == 4 or p == 5 or p == 6)    return coeffi[2];    // D,E,F
    if (p == 7)   return coeffi[3]; // G
    return 1;
}

int skillcon(string s, string c)
{
    int pos = s.find(c);
    if (pos != s.npos)
        if (s[pos + c.length() + 2] >= '0'
            and s[pos + c.length() + 2] <= '9')
        {
            return s[pos + c.length() + 2] - '0';
        }
    return -1;
}

skill SKILL_EMPTY= { "", 0, 0, 0, 0, 0, false, 0 };
skill* appendSkill(int id)
{
    skill s;
    stringstream ss;
    ss.imbue(std::locale("C"));

    s.id = id;

    ss << "SELECT grade_value FROM skill_data WHERE `id`=" << id;
    string str = exec(ss.str());
    if (str.empty())
    {
        // If the gold skill has no pre-skill
        return &SKILL_EMPTY;
    }
    s.value = stoi(str);
    ss.clear(); ss.str("");

    ss << "SELECT text FROM text_data WHERE `category`=47 AND `index`=" << id;
    s.name = exec(ss.str());
    ss.clear(); ss.str("");

    ss << "SELECT need_skill_point FROM single_mode_skill_need_point WHERE `id`=" << id;
    s.cost = stoi(exec(ss.str()));
    ss.clear(); ss.str("");

    ss << "SELECT rarity FROM skill_data WHERE `id`=" << id;
    s.rarity = stoi(exec(ss.str()));
    ss.clear(); ss.str("");
    
    ss << "SELECT disp_order FROM skill_data WHERE `id`=" << id;
    s.disp_order = stoi(exec(ss.str()));
    ss.clear(); ss.str("");

    ss << "SELECT condition_1 FROM skill_data WHERE `id`=" << id;
    string c = exec(ss.str()) + "&";

    int t = skillcon(c, "ground_type");
    if (t != -1)
        s.value = round(s.value * propCoeffi(prop[t + 4]));

    t = skillcon(c, "distance_type");
    if (t != -1)
        s.value = round(s.value * propCoeffi(prop[t + 6]));

    t = skillcon(c, "running_style");
    if (t != -1)
        s.value = round(s.value * propCoeffi(prop[t + 10]));

    s.isML = 0;
    s.inferior_id = 0;
    skillList[sidx++] = s;
    return &skillList[sidx - 1];
}

int findSkillInList(int id)
{
    for (int i = 0; i < sidx; i++)
        if (skillList[i].id == id)
            return i;
}

string doubleC;
bool isML(string s)
{
    if (doubleC.empty())
    {
        stringstream ss;
        doubleC = exec("SELECT text FROM text_data WHERE `category`=47 AND `index`=200011");
        doubleC = doubleC.substr(doubleC.length() - 3);
    }
    return s.substr(s.length() - 3) == doubleC;
}

void handleArgvError(wstring info)
{
    wcout << info << endl;
    system("pause");
    exit(0);
}
bool isCustomDBfile = false;
bool debugMode = false;
string customDBfile;
int main(int argc, char* argv[])
{
    // Initalize
    std::locale::global(std::locale("en_US.UTF-8"));    // I hate it
    for (int i = 0; i < 2; i++)
    {
        for (int k = 0; k < MAXN; k++)
        {
            pw[k][i] = 77777777;
            pv[k][i] = -1;
        }
    }

    // Handle argument
    if (argc > 1)
    {
        for (int i = 1; i < argc; i++)
        {
            if (string(argv[i]) == "-db")
            {
                i++;
                if (i < argc)
                    if (string(argv[i]).find("master.mdb") != string::npos)
                    {
                        isCustomDBfile = true;
                        customDBfile = string(argv[i]);
                    }
                    else handleArgvError(L"Error: 无效的 master.mdb 文件位置");
                else handleArgvError(L"Error: `-db` 选项后面应输入 master.mdb 文件位置");
            }
            else if (string(argv[i]) == "-debug")
                debugMode = true;
            else wprintf(L"Waring: 未知的参数 `%s`，程序将会无视\n", s2ws(string(argv[i])).c_str());
        }
    }

    // Prase share code
    while (1)
    {
        wprintf(L"输入Bwiki生成的分享码:");
        getline(cin, shareCode);
        if (shareCode.substr(0, 4) == "TRSC")
        {
            shareCode = base64_decode(shareCode.substr(4));
            bool ifvalid = (shareCode[0] == 'E') and (shareCode[1] == 'E')
                and (shareCode.substr(shareCode.length() - 4, 4) == string("alvn"));
            if (ifvalid)
            {
                shareCode = shareCode.substr(2, shareCode.length() - 6) + "Z";
                break;
            }
        }
        wprintf(L"无效的分享码，请重新输入\n");
    }


    // 5-dimension data
    string data = shareCode;
    int index = 0;
    for (int i = 0; i < 5; index++)
        if (data[index] == 'Z')
        {
            prop[i++] = toDecimal(data.substr(0, index), 32);
            data = data.substr(index + 1);
            index = 0;
        }


    // Inherited skill amount
    for (; inherit < 0; index++)
        if (data[index] == 'Z')
            inherit = stoi(data.substr(0, data.length() - index));
    sidx = inherit;


    // Horse property
    for (int i = 5; i < 17; index++)
        if (data[index] == 'Z')
            prop[i++] = data[index - 1] - 48;


    bool isDefaultData = true;
    for (int i = 0; i < 5; i++)    isDefaultData = isDefaultData and prop[i] == 100;
    system("cls");
    wprintf(L"\n| 基础属性:\n|\n");
    wprintf(L"| 角色星级: %ws　　　　　固有技能等级: Lv%d\n", (prop[15] == 0 ? L"1-2" : L"3-5"), ++prop[16]);
    wprintf(L"| 芝：%c　　　ダート：%c\n", rk[prop[5]], rk[prop[6]]);
    wprintf(L"| 短距離：%c　マイル：%c　中距離：%c　長距離：%c\n",
        rk[prop[7]], rk[prop[8]], rk[prop[9]], rk[prop[10]]);
    wprintf(L"| 逃げ：%c　　先行：%c　　差し：%c　　追込：%c\n",
        rk[prop[11]], rk[prop[12]], rk[prop[13]], rk[prop[14]]);
    if (not isDefaultData)
        wprintf(L"|\n| スピード：%d\n| スタミナ：%d\n| パワー　：%d\n| 根性　　：%d\n| 賢さ　　：%d\n|\n",
            prop[0], prop[1], prop[2], prop[3], prop[4]);
    wprintf(L"| 继承的固有技能数量：%d\n", inherit);
    wprintf(L"\n");


    // Open master.mdb
    wstring mdbLocate = L"\\AppData\\LocalLow\\Cygames\\umamusume\\master\\master.mdb";
    // https://stackoverflow.com/questions/5920853/how-to-open-a-folder-in-appdata-with-c
    TCHAR szPath[MAX_PATH];
    SUCCEEDED(SHGetFolderPath(NULL, CSIDL_PROFILE | CSIDL_FLAG_CREATE, NULL, 0, szPath));
    mdbLocate = szPath + mdbLocate;
    if (isCustomDBfile)
    {
        mdbLocate = s2ws(customDBfile);
        wcout << L"- 计算器将使用第三方 master.mdb 文件：" << mdbLocate << endl << endl;
    }
    while (1)
    {
        ifstream ifs(mdbLocate);
        if (ifs.is_open())
        {
            // Search mdb in game folder
            int rc = sqlite3_open(ws2s(mdbLocate).c_str(), &db);
            if (rc)
            {
                fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(db));
            }
            else break;
        }
        else
        {
            // Search mdb in current folder
            ifstream ifs2(ExePath() + L"\\master.mdb");
            if (ifs2.is_open())
            {
                int rc = sqlite3_open("master.mdb", &db);
                if (rc)
                {
                    fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(db));
                }
                else
                {
                    wcout << L"- 计算器将使用第三方 master.mdb 文件：" << ExePath() + L"\\master.mdb" << endl << endl;
                    break;
                }
            }
        }
        wprintf(L"错误：无法打开 master.mdb ，请检查游戏目录\n");
        wprintf(L"如果 master.mdb 不在游戏目录下或无法打开，请手动复制并输入文件地址:");
        getline(wcin, mdbLocate);
    }


    // Load skill basic data
    data = data.substr(index);
    index = 0;
    for (; index < data.length(); index++)
    {
        if (data[index] == 'Z')
        {
            int id = toDecimal(data.substr(0, index), 32);
            data = data.substr(index + 1);
            index = 0;

            skill* s = appendSkill(id);
            if (s->value < 0)
            {
                wprintf(L"计算器不支持紫色技能的计算，请移除后重新输入。");
                system("pause");
                return 0;
            }

            // Seperate skill value for dp
            skill* tmp;
            if (s->rarity == 2 and (id - 1) % 10 == 0)
            {
                // Normal gold skill
                tmp = appendSkill(id + 1);
                s->value -= tmp->value;
                s->inferior_id = tmp->id;
                s->isML = 3;
            }
            else if (s->rarity == 2 and (id - 4) % 10 == 0)
            {
                // ML gold skill
                skill* pre_s = appendSkill(id - 2);
                skill* pre_d = appendSkill(id - 3);
                s->value -= pre_d->value;
                s->isML = 2;
                s->inferior_id = pre_d->id;
                pre_d->value -= pre_s->value;
                pre_d->isML = 1;
                pre_d->inferior_id = pre_s->id;
                pre_s->isML = 0;
            }
            else
            {
                string r = exec("SELECT text FROM text_data WHERE `category`=47 AND `index`=" + i2s(id - 1));
                if (r.length() > 3)
                {
                    if (isML(r))
                    {
                        // ML normal skill
                        tmp = appendSkill(id - 1);
                        tmp->value -= s->value;
                        tmp->isML = 1;
                        tmp->inferior_id = s->id;
                    }
                }
            }
        }
    }
    sqlite3_close(db);
    sort(skillList + inherit, skillList + sidx,
        [](skill x, skill y) {return x.disp_order < y.disp_order; });

    if (debugMode)
    {
        printf("[DEBUG] Skill Basic:\n\n");
        for (int i = 0; i < sidx; i++)
        {
            int c = 0, v = 0;
            skill tmp = skillList[i];
            printf("[DEBUG] >%d display_order: %d\n[DEBUG] Name: ", i + 1, tmp.disp_order);    cout<<tmp.name;
            printf(" ID: %d Rarity: %d\n\[DEBUG] cost: %d value: %d\n",
                tmp.id, tmp.rarity, tmp.cost, tmp.value);
            c += tmp.cost, v += tmp.value;
            if (tmp.isML)
            {
                tmp = skillList[findSkillInList(tmp.inferior_id)];
                printf("[DEBUG] | inf_skill: 1 display_order: %d\n[DEBUG] | Name: ", tmp.disp_order);    cout << tmp.name;
                printf(" ID: %d Rarity: %d\n\[DEBUG] | cost: %d value: %d\n",
                    tmp.id, tmp.rarity, tmp.cost, tmp.value);
                c += tmp.cost, v += tmp.value;
                if (tmp.isML)
                {
                    tmp = skillList[findSkillInList(tmp.inferior_id)];
                    printf("[DEBUG] | | inf_skill: 2 display_order: %d\n[DEBUG] | | Name: ", tmp.disp_order);    cout << tmp.name;
                    printf(" ID: %d Rarity: %d\n\[DEBUG] | | cost: %d value: %d\n",
                        tmp.id, tmp.rarity, tmp.cost, tmp.value);
                    c += tmp.cost, v += tmp.value;
                }
                printf("[DEBUG] Total cost: %d Total Value: %d\n", c, v);
            }
            cout << endl;
        }
    }

    // Input
    while (1)
    {
        string _tmp;
        wprintf(L"- 是否具有`切れ者`(能人)属性? (Y/N):");
        getline(cin, _tmp);
        if (_tmp == "Y" or _tmp == "y") goto exit1;
        else if (_tmp == "N" or _tmp == "n")    goto exit2;

        wprintf(L"无效输入，请输入 Y或y 或者 N或n.\n");
    }
    exit1:
        ifGlobalDiscount = true;
    exit2:

    wprintf(L"- 请输入技能点: ");
    cin >> skillPoint;

    // Input discount
    int hintArray[] = { 0, 10, 20, 30, 35, 40 };
    bool useMsgPack = false;
    while (false)
    {
        string _tmp;
        wprintf(L"Would you like to input hint manually(M) or use msgpack to input(P)? (M/P):");
        getline(cin, _tmp);
        if (_tmp == "M" or _tmp == "m") break;
        else if (_tmp == "P" or _tmp == "p") { useMsgPack = true; break; };

        wprintf(L"Invalid input, please enter M/m (to enter manually) or P/p (to use msgpack).\n");
    }

    if (useMsgPack)
    {

    }
    else 
    {
        wprintf(L"\n- 请输入技能的Hint值（只需输入0到5的整数）\n");
        wprintf(L"- 注意，金技能的Hint和其下位技能的Hint没有关系，请分别输入，不要混淆\n\n");
        bool ML = false;
        for (int i = inherit, hint = 0; i < sidx; i++)
        {
            skill* s = &skillList[i];

            wprintf(L"| 种类: %s | 技能名: ",
                (s->rarity == 2 ? L"金技能　" : L"普通技能"));
            printf("%s | Hint: ",s->name.c_str());
            if (!ML)  cin >> hint;
            else cout << hint << endl;

            while (hint < 0 or hint > 5)
            {
                wprintf(L"无效的Hint值，Hint值应为大于等于0，小于等于5的整数,请重新输入:");
                cin >> hint;
            }

            s->cost = round(s->cost * ((1 - 0.01 * hintArray[hint] - 0.00001) - 0.1 * (ifGlobalDiscount ? 1 : 0)));
            // 0.00001: Create precision fault manually

            wprintf(L"| 此技能消耗 %d Pt, 技能评价 %d Pt. \n|\n", s->cost, s->value);

            if (s->isML == 1) ML = true;
            else ML = false;
        }
    }

    if (inherit > 0)   wprintf(L"\n- 请依次输入继承技能的Hint:\
        \n- 如果最终计算结果中有继承的技能，将会以 `IS.Lv`+x 的形式出现，表示Hint为x的继承技能\n\n");
    for (int i = 0; i < inherit; i++)
    {
        int hint = 0;
        wprintf(L"| 继承技能%d的Hint: ", i + 1);
        cin >> hint;
        skill tmp = { "IS.Lv" + i2s(hint), i + 1 ,
            200 * ((1 - 0.01 * hintArray[hint]) - 0.1 * (ifGlobalDiscount ? 1 : 0)) , 180, 1, i, false, 0};
        skillList[i] = tmp;
    }

    // Restore data to write dp easier
    int didx = 0;
    for (int i = 0; i < sidx; i++)
    {
        skill tmp = skillList[i];
        if (vis[tmp.id])    continue;
        vis[tmp.id] = true;
    
        if (tmp.inferior_id != 0)
        {
            skill tmp2 = skillList[findSkillInList(tmp.inferior_id)];
            if (tmp2.inferior_id != 0)
            {
                skill tmp3 = skillList[findSkillInList(tmp2.inferior_id)];
                idLog[didx + 2 * MAXN] = tmp.id;
                pw[didx][1] = tmp.cost;
                pv[didx][1] = tmp.value;
                vis[tmp.id] = true;

                idLog[didx + MAXN] = tmp2.id;
                pw[didx][0] = tmp2.cost;
                pv[didx][0] = tmp2.value;
                vis[tmp2.id] = true;

                idLog[didx] = tmp3.id;
                w[didx] = tmp3.cost;
                v[didx] = tmp3.value;
            }
            else
            {
                idLog[didx] = tmp2.id;
                w[didx] = tmp2.cost;
                v[didx] = tmp2.value;
                vis[tmp2.id] = true;

                idLog[didx + MAXN] = tmp.id;
                pw[didx][0] = tmp.cost;
                pv[didx][0] = tmp.value;
                
            }
        }
        else 
        {
            idLog[didx] = tmp.id;
            w[didx] = tmp.cost;
            v[didx] = tmp.value;
        }
        didx++;
    }

    if (debugMode)
    {
        printf("[DEBUG] Skill List:\n\n");
        for (int i = 0; i < sidx; i++)
        {
            int c = 0, v = 0;
            skill tmp = skillList[i];
            printf("[DEBUG] >%d display_order: %d\n[DEBUG] Name: ", i + 1, tmp.disp_order);    cout << tmp.name;
            printf(" ID: %d Rarity: %d\n\[DEBUG] cost: %d value: %d\n",
                tmp.id, tmp.rarity, tmp.cost, tmp.value);
            c += tmp.cost, v += tmp.value;
            if (tmp.isML)
            {
                tmp = skillList[findSkillInList(tmp.inferior_id)];
                printf("[DEBUG] | inf_skill: 1 display_order: %d\n[DEBUG] | Name: ", tmp.disp_order);    cout << tmp.name;
                printf(" ID: %d Rarity: %d\n\[DEBUG] | cost: %d value: %d\n",
                    tmp.id, tmp.rarity, tmp.cost, tmp.value);
                c += tmp.cost, v += tmp.value;
                if (tmp.isML)
                {
                    tmp = skillList[findSkillInList(tmp.inferior_id)];
                    printf("[DEBUG] | | inf_skill: 2 display_order: %d\n[DEBUG] | | Name: ", tmp.disp_order);    cout << tmp.name;
                    printf(" ID: %d Rarity: %d\n\[DEBUG] | | cost: %d value: %d\n",
                        tmp.id, tmp.rarity, tmp.cost, tmp.value);
                    c += tmp.cost, v += tmp.value;
                }
                printf("[DEBUG] Total cost: %d Total Value: %d\n", c, v);
            }
            cout << endl;
        }
    }

    // dp
    for (int i = 0; i < didx; i++)
    {
        for (int j = skillPoint; j >= w[i]; j--)
        {
            int choice[] = {
                dp[j],
                dp[j - w[i]] + v[i],

                j - w[i] - pw[i][0] >= 0 ?
                    dp[j - w[i] - pw[i][0]] + v[i] + pv[i][0] :
                    -1,

                j - w[i] - pw[i][0] - pw[i][1] >= 0 ?
                    dp[j - w[i] - pw[i][0] - pw[i][1]] + v[i] + pv[i][0] + pv[i][1] :
                    -1
            };

            auto ismax = [choice](int idx) 
            {
                bool _ISMAX_ = true;
                for (int i = 0; i < 4; i++)
                    _ISMAX_ = choice[idx] >= choice[i] and _ISMAX_;
                return _ISMAX_;
            };

            if (ismax(0))
            {
                dp[j] = choice[0];
            }
            else if (ismax(1))
            {
                dp[j] = choice[1];
                Log[j] = Log[j - w[i]];
                Log[j].push_back(i);
            }
            else if (ismax(2))
            {
                dp[j] = choice[2];
                Log[j] = Log[j - w[i] - pw[i][0]];
                Log[j].push_back(i);
                Log[j].push_back(i + MAXN);
            }
            else if (ismax(3))
            {
                dp[j] = choice[3];
                Log[j] = Log[j - w[i] - pw[i][0] - pw[i][1]];
                Log[j].push_back(i);
                Log[j].push_back(i + MAXN);
                Log[j].push_back(i + 2 * MAXN);
            }
        }
    }

    wprintf(L"\n\n| 结果:\n|\n| | 技能名   消耗Pt   评价Pt\n|\n");

    skill result[MAXN];
    int ridx = 0;
    for (int i = 0; i < Log[skillPoint].size(); i++)
    {
        result[ridx++] = skillList[findSkillInList(idLog[Log[skillPoint][i]])];
    }
    sort(result, result + ridx,
        [](skill x, skill y) {return x.disp_order < y.disp_order; });
    
    int costAll = 0;
    for (int i = 0; i < ridx; i++)  costAll += result[i].cost;
    for (int i = 0; i < ridx; i++)
    {
        if (result[i].isML == 0)
            printf("|   %s %d %d\n", result[i].name.c_str(), result[i].cost, result[i].value);
        if (result[i].isML == 1 or result[i].isML == 3)
        {
            printf("| / %s %d %d\n", result[i].name.c_str(), result[i].cost, result[i].value);
            printf("| \\ %s %d %d\n", result[i+1].name.c_str(), result[i+1].cost, result[i+1].value);
            i++;
        }
        if (result[i].isML == 2)
        {
            printf("| / %s %d %d\n", result[i].name.c_str(), result[i].cost, result[i].value);
            printf("| | %s %d %d\n", result[i + 1].name.c_str(), result[i + 1].cost, result[i + 1].value);
            printf("| \\ %s %d %d\n", result[i + 2].name.c_str(), result[i + 2].cost, result[i + 2].value);
            i += 2;
        }
    }
    wcout << L"|\n| 技能总消耗: " << costAll << endl;
    wcout << L"| 技能总评分: " << dp[skillPoint] << endl;

    if (not isDefaultData)
    {
        int maxRankPoint = 0;
        for (int i = 0; i < 5; i++)    maxRankPoint += basicPoint[prop[i]];
        maxRankPoint += (prop[15] == 1 ? prop[16] * 170 : prop[16] * 120) + dp[skillPoint];
        wcout << L"|\n| 理论能达到的最大评分：" << maxRankPoint << endl;
        if (maxRankPoint > 19800)  wprintf(L"|\n| \033[31;1mUG及以上评价确定\033[0m\n");
    }

    wcout << endl;
    system("pause");
    return 0;
}

/*
*  Currently the normal gold skill id is ****01,
*  and its pre-skill id is ****02, with the same pre-4 digits.
*  When it comes to multiple-level(ML) skill, it is 01 -> double circle,
*  02 -> single cirle, 03 -> purple , 04 -> gold
*
*  Caution: if normal gold skill exists, enter only gold skill
*  if ML skill exists, enter only gold skill(if exist), or single cirle,
*  don't enter double circle one.
*
* 
*  Test:
*  shareCode = "TRSCRUUzNFozNFozNFozNFozNFoyWjBaMVoyWjNaNFo1WjFaMlozWjRaMFowWjYzYWVaNjNhbVo2M2tiWjYzczZaNjNzZlo2NHFwWjY1YTNaNmQyclo2ZDNmWjZkM3BhbHZu";
*  shareCode = "TRSCRUUzNFozNFozNFozNFozNFozWjBaMVoyWjNaNFo1WjFaMlozWjRaMFowWjYzYWNaNjNmMlo2M2wwWjYzbzRaNjQ1N1o2NDhjWjY0OG1aNjRhOFo2NGFzWjY0ZDFaNjRkYlo2NGV1WjY0ZjhaNjRncVo2NGg0WjY0ajBaNjRqYVo2NGxnWjY0bHFaNjRubVo2NG8wYWx2bg==";
*/
