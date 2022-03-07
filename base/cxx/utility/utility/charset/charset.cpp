#include "charset.h"

#include <locale>
#include <string.h>

#include "charset_codes.h"

typedef unsigned short wchar;

namespace utility
{
//////////////////////////////////////////////////////////////////////
// Codes from iconv BEGIN
/* Return code if invalid input after a shift sequence of n bytes was read. (xxx_mbtowc) */
#define RET_SHIFT_ILSEQ(n) (-1 - 2 * (n))
/* Return code if invalid. (xxx_mbtowc) */
#define RET_ILSEQ RET_SHIFT_ILSEQ(0)
/* Return code if only a shift sequence of n bytes was read. (xxx_mbtowc) */
#define RET_TOOFEW(n) (-2 - 2 * (n))
/* Return code if invalid. (xxx_wctomb) */
#define RET_ILUNI (-1)
/* Return code if output buffer is too small. (xxx_wctomb, xxx_reset) */
#define RET_TOOSMALL (-2)

typedef unsigned int ucs4_t;

/* Specification: RFC 3629 */
static int utf8_mbtowc(ucs4_t* pwc, const unsigned char* s, int n)
{
    unsigned char c = s[0];
    if (c < 0x80)
    {
        *pwc = c;
        return 1;
    }
    else if (c < 0xc2)
    {
        return RET_ILSEQ;
    }
    else if (c < 0xe0)
    {
        if (n < 2)
        {
            return RET_TOOFEW(0);
        }
        if (!((s[1] ^ 0x80) < 0x40))
        {
            return RET_ILSEQ;
        }
        *pwc = ((ucs4_t)(c & 0x1f) << 6) | (ucs4_t)(s[1] ^ 0x80);
        return 2;
    }
    else if (c < 0xf0)
    {
        if (n < 3)
        {
            return RET_TOOFEW(0);
        }
        if (!((s[1] ^ 0x80) < 0x40 && (s[2] ^ 0x80) < 0x40 && (c >= 0xe1 || s[1] >= 0xa0)))
        {
            return RET_ILSEQ;
        }
        *pwc = ((ucs4_t)(c & 0x0f) << 12) | ((ucs4_t)(s[1] ^ 0x80) << 6) | (ucs4_t)(s[2] ^ 0x80);
        return 3;
    }
    else if (c < 0xf8 && sizeof(ucs4_t) * 8 >= 32)
    {
        if (n < 4)
        {
            return RET_TOOFEW(0);
        }
        if (!((s[1] ^ 0x80) < 0x40 && (s[2] ^ 0x80) < 0x40 && (s[3] ^ 0x80) < 0x40 && (c >= 0xf1 || s[1] >= 0x90)))
        {
            return RET_ILSEQ;
        }
        *pwc = ((ucs4_t)(c & 0x07) << 18) | ((ucs4_t)(s[1] ^ 0x80) << 12) | ((ucs4_t)(s[2] ^ 0x80) << 6) | (ucs4_t)(s[3] ^ 0x80);
        return 4;
    }
    else if (c < 0xfc && sizeof(ucs4_t) * 8 >= 32)
    {
        if (n < 5)
        {
            return RET_TOOFEW(0);
        }
        if (!((s[1] ^ 0x80) < 0x40 && (s[2] ^ 0x80) < 0x40 && (s[3] ^ 0x80) < 0x40 && (s[4] ^ 0x80) < 0x40 && (c >= 0xf9 || s[1] >= 0x88)))
        {
            return RET_ILSEQ;
        }
        *pwc = ((ucs4_t)(c & 0x03) << 24) | ((ucs4_t)(s[1] ^ 0x80) << 18) | ((ucs4_t)(s[2] ^ 0x80) << 12) | ((ucs4_t)(s[3] ^ 0x80) << 6)
               | (ucs4_t)(s[4] ^ 0x80);
        return 5;
    }
    else if (c < 0xfe && sizeof(ucs4_t) * 8 >= 32)
    {
        if (n < 6)
        {
            return RET_TOOFEW(0);
        }
        if (!((s[1] ^ 0x80) < 0x40 && (s[2] ^ 0x80) < 0x40 && (s[3] ^ 0x80) < 0x40 && (s[4] ^ 0x80) < 0x40 && (s[5] ^ 0x80) < 0x40
              && (c >= 0xfd || s[1] >= 0x84)))
        {
            return RET_ILSEQ;
        }
        *pwc = ((ucs4_t)(c & 0x01) << 30) | ((ucs4_t)(s[1] ^ 0x80) << 24) | ((ucs4_t)(s[2] ^ 0x80) << 18) | ((ucs4_t)(s[3] ^ 0x80) << 12)
               | ((ucs4_t)(s[4] ^ 0x80) << 6) | (ucs4_t)(s[5] ^ 0x80);
        return 6;
    }
    else
    {
        return RET_ILSEQ;
    }
}

static int utf8_wctomb(unsigned char* r, ucs4_t wc, int n) /* n == 0 is acceptable */
{
    int count;
    if (wc < 0x80)
    {
        count = 1;
    }
    else if (wc < 0x800)
    {
        count = 2;
    }
    else if (wc < 0x10000)
    {
        count = 3;
    }
    else if (wc < 0x200000)
    {
        count = 4;
    }
    else if (wc < 0x4000000)
    {
        count = 5;
    }
    else if (wc <= 0x7fffffff)
    {
        count = 6;
    }
    else
    {
        return RET_ILUNI;
    }
    if (n < count)
    {
        return RET_TOOSMALL;
    }
    switch (count) /* note: code falls through cases! */
    {
    case 6:
        r[5] = 0x80 | (wc & 0x3f);
        wc = wc >> 6;
        wc |= 0x4000000;
    case 5:
        r[4] = 0x80 | (wc & 0x3f);
        wc = wc >> 6;
        wc |= 0x200000;
    case 4:
        r[3] = 0x80 | (wc & 0x3f);
        wc = wc >> 6;
        wc |= 0x10000;
    case 3:
        r[2] = 0x80 | (wc & 0x3f);
        wc = wc >> 6;
        wc |= 0x800;
    case 2:
        r[1] = 0x80 | (wc & 0x3f);
        wc = wc >> 6;
        wc |= 0xc0;
    case 1:
        r[0] = (unsigned char)wc;
    }
    return count;
}
// Codes from iconv END
//////////////////////////////////////////////////////////////////////

static int utf8_to_unicode(const char* utf8str, int slen, wchar* outbuf, int osize)
{
    if (!utf8str || slen <= 0)
    {
        return 0;
    }
    int ret = -1;
    const unsigned char* p = (const unsigned char*)utf8str;
    int i = 0, cnt = 0, cb;
    wchar* op = outbuf;
    ucs4_t wc;
    do
    {
        if (0 == utf8str)
        {
            break;
        }
        if (slen < 0)
        {
            slen = 0;
            while (*p++)
            {
                ++slen;
            }
            p = (const unsigned char*)utf8str;
        }
        for (i = 0; i < slen;)
        {
            cb = utf8_mbtowc(&wc, p, slen - i);
            if (cb <= 0)
            {
                break;
            }
            i += cb;
            p += cb;
            if (op)
            {
                if (cnt + 2 <= osize)
                {
                    *op++ = (wchar)wc;
                }
                else
                {
                    break;
                }
            }
            cnt += 2;
        }
        ret = cnt;
    } while (0);
    return ret;
}

static int unicode_to_utf8(const wchar* wstr, int wlen, char* outbuf, int osize)
{
    if (!wstr || wlen <= 0)
    {
        return 0;
    }
    int ret = -1;
    const wchar* p = wstr;
    int cnt = 0, i = 0, cb;
    unsigned char* op = (unsigned char*)outbuf;
    ucs4_t wc;
    unsigned char tmp[4];
    do
    {
        if (0 == wstr)
        {
            break;
        }
        if (wlen < 0)
        {
            wlen = 0;
            while (*p++)
            {
                ++wlen;
            }
            p = wstr;
        }
        for (i = 0; i < wlen * 2; i += 2, ++p)
        {
            wc = *p;
            if (op)
            {
                cb = utf8_wctomb(op, wc, osize - cnt);
                if (cb <= 0)
                {
                    break;
                }
                op += cb;
            }
            else
            {
                cb = utf8_wctomb(tmp, wc, 4);
            }

            cnt += cb;
        }
        ret = cnt;
    } while (0);
    return ret;
}

static int gbk_to_unicode(const char* gbkstr, int slen, wchar* outbuf, int osize)
{
    if (!gbkstr || slen <= 0)
    {
        return 0;
    }
    int ret = -1;
    int cnt = 0, i = 0, cb;
    const unsigned char* p = (const unsigned char*)gbkstr;
    wchar* op = outbuf;
    unsigned char c1;
    wchar c, chr;
    do
    {
        if (0 == gbkstr)
        {
            break;
        }
        if (slen < 0)
        {
            slen = 0;
            while (*p++)
            {
                ++slen;
            }
            p = (const unsigned char*)gbkstr;
        }
        while (i < slen)
        {
            c1 = *p;
            if (c1 < 0x80)
            {
                chr = c1;
                cb = 1;
            }
            else
            {
                if (i + 1 >= slen)
                {
                    break;
                }
                c = c1 << 8 | *(p + 1);
                chr = gbk_2_unicode_codes[c];
                if (0 == chr)
                {
                    chr = '?';
                }
                cb = 2;
            }
            i += cb;
            p += cb;
            if (op)
            {
                if (cnt + 2 <= osize)
                {
                    *op++ = chr;
                }
                else
                {
                    break;
                }
            }
            cnt += 2;
        }
        ret = cnt;
    } while (0);
    return ret;
}

static int unicode_to_gbk(const wchar* wstr, int wlen, char* outbuf, int osize)
{
    if (!wstr || wlen <= 0)
    {
        return 0;
    }
    int ret = -1;
    int cnt = 0, i, cb;
    const wchar* p = wstr;
    char* op = outbuf;
    wchar c, chr;
    do
    {
        if (0 == wstr)
        {
            break;
        }
        if (wlen < 0)
        {
            wlen = 0;
            while (*p++)
            {
                ++wlen;
            }
            p = wstr;
        }
        for (i = 0; i < wlen; ++i, ++p)
        {
            c = *p;
            if (c < 0x80)
            {
                cb = 1;
                chr = c;
            }
            else
            {
                chr = unicode_2_gbk_codes[c];
                if (0 == chr)
                {
                    chr = '?';
                }
                cb = 2;
            }
            if (op)
            {
                if (cnt + cb <= osize)
                {
                    if (cb == 1)
                    {
                        *op++ = (char)chr;
                    }
                    else
                    {
                        *op++ = (char)(chr >> 8);
                        *op++ = (char)chr;
                    }
                }
                else
                {
                    break;
                }
            }
            cnt += cb;
        }
        ret = cnt;
    } while (0);
    return ret;
}

static bool is_gbk(const char* str, int len)
{
    /* 需要说明的是: 
       is_gbk是通过双字节是否落在GBK的编码范围内实现的, 而UTF8编码格式的每个字节都是落在GBK的编码范围内,
       所以只有先调用is_utf8先判断不是UTF8编码，再调用is_gbk才有意义. */
    if (str && len > 0)
    {
        unsigned int byteCount = 0; /* GBK可用1-2个字节编码, 中文2个, 英文1个 */
        for (size_t i = 0; i < len; ++i)
        {
            unsigned char ch = str[i];
            if ('\0' == ch)
            {
                break;
            }
            if (0 == byteCount)
            {
                if (ch >= 0x80)
                {
                    if (ch >= 0x81 && ch <= 0xFE)
                    {
                        byteCount = 2;
                    }
                    else
                    {
                        return false;
                    }
                    byteCount--;
                }
            }
            else
            {
                if (ch < 0x40 || ch > 0xFE)
                {
                    return false;
                }
                byteCount--;
            }
        }
        if (0 != byteCount) /* 违反GBK编码规则 */
        {
            return false;
        }
    }
    return true;
}

bool is_utf8(const char* str, int len)
{
    if (str && len > 0)
    {
        unsigned int byteCount = 0; /* UTF8可用1-6个字节编码, ASCII用1个字节 */
        for (size_t i = 0; i < len; ++i)
        {
            unsigned char ch = str[i];
            if ('\0' == ch)
            {
                break;
            }
            if (0 == byteCount)
            {
                if (ch >= 0x80) /* 如果不是ASCII码, 应该是多字节符, 计算字节数 */
                {
                    if (ch >= 0xFC && ch <= 0xFD)
                    {
                        byteCount = 6;
                    }
                    else if (ch >= 0xF8)
                    {
                        byteCount = 5;
                    }
                    else if (ch >= 0xF0)
                    {
                        byteCount = 4;
                    }
                    else if (ch >= 0xE0)
                    {
                        byteCount = 3;
                    }
                    else if (ch >= 0xC0)
                    {
                        byteCount = 2;
                    }
                    else
                    {
                        return false;
                    }
                    byteCount--;
                }
            }
            else
            {
                if (0x80 != (ch & 0xC0)) /* 多字节符的非首字节, 应为10xxxxxx */
                {
                    return false;
                }
                byteCount--; /* 减到为零为止 */
            }
        }
        if (0 != byteCount) /* 违反UTF8编码规则 */
        {
            return false;
        }
    }
    return true;
}

//////////////////////////////////////////////////////////////////////
// 字符集操作接口封装
//////////////////////////////////////////////////////////////////////
bool Charset::isAscii(const std::string& str)
{
    for (size_t i = 0; i < str.size(); ++i)
    {
        if (!isascii(str[i]))
        {
            return false;
        }
    }
    return true;
}

std::string Charset::getLocale()
{
    /**
     * (1)在Linux下, locale的命名格式为: language_area.charset, 其中:
     *    language - 表示语言, 例如: 英语 或 中文.
     *        area - 表示使用该语言的地区, 例如: 美国 或者 中国大陆.
     *     charset - 表示字符集编码, 例如: UTF-8 或者 GBK. 
     *               可以省略, 此时会选择当前语言的默认charset, Linux发行版大都使用UTF-8编码, 它是Unicode字符集的一种编码方式,
     *               能够支持世界上的所有语言, 省略charset也就意味使用UTF-8.
     *    例如: zh_CN.UTF-8
     *   ————————————————————————————————————————————————————
     *   | locale 名称 |              说  明                |
     *   ————————————————————————————————————————————————————
     *   |   af_ZA 	   | 南非语
     *   |   ar_AE     | 阿拉伯语(阿联酋)
     *   |   ar_BH     | 阿拉伯语(巴林)
     *   |   ar_DZ     | 阿拉伯语(阿尔及利亚)
     *   |   ar_EG     | 阿拉伯语(埃及)
     *   |   ar_IQ     | 阿拉伯语(伊拉克)
     *   |   ar_JO     | 阿拉伯语(约旦)
     *   |   ar_KW     | 阿拉伯语(科威特)
     *   |   ar_LB     | 阿拉伯语(黎巴嫩)
     *   |   ar_LY     | 阿拉伯语(利比亚)
     *   |   ar_MA     | 阿拉伯语(摩洛哥)
     *   |   ar_OM     | 阿拉伯语(阿曼)
     *   |   ar_QA     | 阿拉伯语(卡塔尔)
     *   |   ar_SA     | 阿拉伯语(沙特阿拉伯)
     *   |   ar_SY     | 阿拉伯语(叙利亚)
     *   |   ar_TN     | 阿拉伯语(突尼斯)
     *   |   ar_YE     | 阿拉伯语(也门)
     *   |   az_AZ     | 阿塞拜疆语(拉丁文)
     *   |   az_AZ     | 阿塞拜疆语(西里尔文)
     *   |   be_BY     | 比利时语
     *   |   bg_BG     | 保加利亚语
     *   |   bs_BA     | 波斯尼亚语(拉丁文, 波斯尼亚和黑塞哥维那)
     *   |   ca_ES     | 加泰隆语
     *   |   cs_CZ     | 捷克语
     *   |   cy_GB     | 威尔士语
     *   |   da_DK     | 丹麦语
     *   |   de_AT     | 德语(奥地利)
     *   |   de_CH     | 德语(瑞士)
     *   |   de_DE     | 德语(德国)
     *   |   de_LI     | 德语(列支敦士登)
     *   |   de_LU     | 德语(卢森堡)
     *   |   dv_MV     | 第维埃语
     *   |   el_GR     | 希腊语
     *   |   en_AU     | 英语(澳大利亚)
     *   |   en_BZ     | 英语(伯利兹)
     *   |   en_CA     | 英语(加拿大)
     *   |   en_CB     | 英语(加勒比海)
     *   |   en_GB     | 英语(英国)
     *   |   en_IE     | 英语(爱尔兰)
     *   |   en_JM     | 英语(牙买加)
     *   |   en_NZ     | 英语(新西兰)
     *   |   en_PH     | 英语(菲律宾)
     *   |   en_TT     | 英语(特立尼达)
     *   |   en_US     | 英语(美国)
     *   |   en_ZA     | 英语(南非)
     *   |   en_ZW     | 英语(津巴布韦)
     *   |   es_AR     | 西班牙语(阿根廷)
     *   |   es_BO     | 西班牙语(玻利维亚)
     *   |   es_CL     | 西班牙语(智利)
     *   |   es_CO     | 西班牙语(哥伦比亚)
     *   |   es_CR     | 西班牙语(哥斯达黎加)
     *   |   es_DO     | 西班牙语(多米尼加共和国)
     *   |   es_EC     | 西班牙语(厄瓜多尔)
     *   |   es_ES     | 西班牙语(传统)
     *   |   es_ES     | 西班牙语(国际)
     *   |   es_GT     | 西班牙语(危地马拉)
     *   |   es_HN     | 西班牙语(洪都拉斯)
     *   |   es_MX     | 西班牙语(墨西哥)
     *   |   es_NI     | 西班牙语(尼加拉瓜)
     *   |   es_PA     | 西班牙语(巴拿马)
     *   |   es_PE     | 西班牙语(秘鲁)
     *   |   es_PR     | 西班牙语(波多黎各(美))
     *   |   es_PY     | 西班牙语(巴拉圭)
     *   |   es_SV     | 西班牙语(萨尔瓦多)
     *   |   es_UY     | 西班牙语(乌拉圭)
     *   |   es_VE     | 西班牙语(委内瑞拉)
     *   |   et_EE     | 爱沙尼亚语
     *   |   eu_ES     | 巴士克语
     *   |   fa_IR     | 法斯语
     *   |   fi_FI     | 芬兰语
     *   |   fo_FO     | 法罗语
     *   |   fr_BE     | 法语(比利时)
     *   |   fr_CA     | 法语(加拿大)
     *   |   fr_CH     | 法语(瑞士)
     *   |   fr_FR     | 法语(法国)
     *   |   fr_LU     | 法语(卢森堡)
     *   |   fr_MC     | 法语(摩纳哥)
     *   |   gl_ES     | 加里西亚语
     *   |   gu_IN     | 古吉拉特语
     *   |   he_IL     | 希伯来语
     *   |   hi_IN     | 印地语
     *   |   hr_BA     | 克罗地亚语(波斯尼亚和黑塞哥维那)
     *   |   hr_HR     | 克罗地亚语
     *   |   hu_HU     | 匈牙利语
     *   |   hy_AM     | 亚美尼亚语
     *   |   id_ID     | 印度尼西亚语
     *   |   is_IS     | 冰岛语
     *   |   it_CH     | 意大利语(瑞士)
     *   |   it_IT     | 意大利语(意大利)
     *   |   ja_JP     | 日语
     *   |   ka_GE     | 格鲁吉亚语
     *   |   kk_KZ     | 哈萨克语
     *   |   kn_IN     | 卡纳拉语
     *   |   ko_KR     | 朝鲜语
     *   |   kok_IN    | 孔卡尼语
     *   |   ky_KG     | 吉尔吉斯语(西里尔文)
     *   |   lt_LT     | 立陶宛语
     *   |   lv_LV     | 拉脱维亚语
     *   |   mi_NZ     | 毛利语
     *   |   mk_MK     | 马其顿语(FYROM)
     *   |   mn_MN     | 蒙古语(西里尔文)
     *   |   mr_IN     | 马拉地语
     *   |   ms_BN     | 马来语(文莱达鲁萨兰)
     *   |   ms_MY     | 马来语(马来西亚)
     *   |   mt_MT     | 马耳他语
     *   |   nb_NO     | 挪威语(伯克梅尔)(挪威)
     *   |   nl_BE     | 荷兰语(比利时)
     *   |   nl_NL     | 荷兰语(荷兰)
     *   |   nn_NO     | 挪威语(尼诺斯克)(挪威)
     *   |   ns_ZA     | 北梭托语
     *   |   pa_IN     | 旁遮普语
     *   |   pl_PL     | 波兰语
     *   |   pt_BR     | 葡萄牙语(巴西)
     *   |   pt_PT     | 葡萄牙语(葡萄牙)
     *   |   qu_BO     | 克丘亚语(玻利维亚)
     *   |   qu_EC     | 克丘亚语(厄瓜多尔)
     *   |   qu_PE     | 克丘亚语(秘鲁)
     *   |   ro_RO     | 罗马尼亚语
     *   |   ru_RU     | 俄语
     *   |   sa_IN     | 梵文
     *   |   se_FI     | 北萨摩斯语(芬兰)
     *   |   se_FI     | 斯科特萨摩斯语(芬兰)
     *   |   se_FI     | 伊那里萨摩斯语(芬兰)
     *   |   se_NO     | 北萨摩斯语(挪威)
     *   |   se_NO     | 律勒欧萨摩斯语(挪威)
     *   |   se_NO     | 南萨摩斯语(挪威)
     *   |   se_SE     | 北萨摩斯语(瑞典)
     *   |   se_SE     | 律勒欧萨摩斯语(瑞典)
     *   |   se_SE     | 南萨摩斯语(瑞典)
     *   |   sk_SK     | 斯洛伐克语
     *   |   sl_SI     | 斯洛文尼亚语
     *   |   sq_AL     | 阿尔巴尼亚语
     *   |   sr_BA     | 塞尔维亚语(拉丁文, 波斯尼亚和黑塞哥维那)
     *   |   sr_BA     | 塞尔维亚语(西里尔文, 波斯尼亚和黑塞哥维那)
     *   |   sr_SP     | 塞尔维亚(拉丁)
     *   |   sr_SP     | 塞尔维亚(西里尔文)
     *   |   sv_FI     | 瑞典语(芬兰)
     *   |   sv_SE     | 瑞典语
     *   |   sw_KE     | 斯瓦希里语
     *   |   syr_SY    | 叙利亚语
     *   |   ta_IN     | 泰米尔语
     *   |   te_IN     | 泰卢固语
     *   |   th_TH     | 泰语
     *   |   tl_PH     | 塔加路语(菲律宾)
     *   |   tn_ZA     | 茨瓦纳语
     *   |   tr_TR     | 土耳其语
     *   |   tt_RU     | 鞑靼语
     *   |   uk_UA     | 乌克兰语
     *   |   ur_PK     | 乌都语
     *   |   uz_UZ     | 乌兹别克语(拉丁文)
     *   |   uz_UZ     | 乌兹别克语(西里尔文)
     *   |   vi_VN     | 越南语
     *   |   xh_ZA     | 班图语
     *   |   zh_CN     | 中文(简体)
     *   |   zh_HK     | 中文(香港)
     *   |   zh_MO     | 中文(澳门)
     *   |   zh_SG     | 中文(新加坡)
     *   |   zh_TW     | 中文(繁体)
     *   |   zu_ZA     | 祖鲁语
     *   ————————————————————————————————————————————————————
     * (2)在Windows下, locale的命名格式为: language_area.codepage, 其中:
     *    language - 表示语言, 例如: 英语 或 中文.
     *        area - 表示使用该语言的地区, 例如: 美国 或者 中国大陆.
     *    codepage - 表示代码页, 例如: GBK编码的代码页是936, 简体中文版Windows默认的编码. 
     *    可以省略area和codepage, 只写language, 此时会选择当前language的默认area和codepage.
     *    例如: chs_china.936
     *          chs是"hinese-simplified"的简写, 表示简体中文, china表示中国大陆, 936是GBK编码的代码页.
     *    链接: 语言(https://docs.microsoft.com/en-us/openspecs/windows_protocols/ms-lcid/a9eac961-e77d-41a6-90a5-ce1a8b0cdb9c)
     *          代码页(https://docs.microsoft.com/zh-cn/windows/win32/intl/code-page-identifiers)
     */
    char* currLocale = setlocale(LC_ALL, NULL); /* currLocale不需要释放内存 */
    if (currLocale)
    {
        if (0 == strcmp("C", currLocale) || 0 == strcmp("c", currLocale))
        {
            currLocale = setlocale(LC_ALL, "");
        }
    }
    return (currLocale ? currLocale : std::string());
}

bool Charset::setLocale(const std::string& locale)
{
    char* currLocale = setlocale(LC_ALL, locale.c_str());
    return (currLocale ? true : false);
}

Charset::Coding Charset::getCoding(const std::string& str)
{
    if (is_utf8(str.c_str(), str.size()))
    {
        return Coding::utf8;
    }
    else if (is_gbk(str.c_str(), str.size()))
    {
        return Coding::gbk;
    }
    return Coding::unknown;
}

std::wstring Charset::utf8ToUnicode(const std::string& str)
{
    if (str.empty())
    {
        return std::wstring();
    }
    int size = utf8_to_unicode(str.c_str(), str.size(), 0, 0);
    wchar* unicode = new wchar[size / 2 + 1];
    size = utf8_to_unicode(str.c_str(), str.size(), unicode, sizeof(wchar) * (size / 2 + 1));
    unicode[size / 2] = 0;
    std::wstring ret((wchar_t*)unicode, size / 2);
    delete[] unicode;
    return ret;
}

std::string Charset::unicodeToUtf8(const std::wstring& wstr)
{
    if (wstr.empty())
    {
        return std::string();
    }
    int size = unicode_to_utf8((wchar*)wstr.c_str(), wstr.size(), 0, 0);
    char* utf8 = new char[size + 1];
    size = unicode_to_utf8((wchar*)wstr.c_str(), wstr.size(), utf8, size + 1);
    utf8[size] = 0;
    std::string ret(utf8, size);
    delete[] utf8;
    return ret;
}

std::wstring Charset::gbkToUnicode(const std::string& str)
{
    if (str.empty())
    {
        return std::wstring();
    }
    int size = gbk_to_unicode(str.c_str(), str.size(), 0, 0);
    wchar* unicode = new wchar[size / 2 + 1];
    size = gbk_to_unicode(str.c_str(), str.size(), unicode, sizeof(wchar) * (size / 2 + 1));
    unicode[size / 2] = 0;
    std::wstring ret((wchar_t*)unicode, size / 2);
    delete[] unicode;
    return ret;
}

std::string Charset::unicodeToGbk(const std::wstring& wstr)
{
    if (wstr.empty())
    {
        return std::string();
    }
    int size = unicode_to_gbk((wchar*)wstr.c_str(), wstr.size(), 0, 0);
    char* gbk = new char[size + 1];
    size = unicode_to_gbk((wchar*)wstr.c_str(), wstr.size(), gbk, size + 1);
    gbk[size] = 0;
    std::string ret(gbk, size);
    delete[] gbk;
    return ret;
}

std::string Charset::gbkToUtf8(const std::string& str)
{
    return unicodeToUtf8(gbkToUnicode(str));
}

std::string Charset::utf8ToGbk(const std::string& str)
{
    return unicodeToGbk(utf8ToUnicode(str));
}
} // namespace utility
