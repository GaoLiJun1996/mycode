#include "StdAfx.h"
#include "Util.h"
//#define MY_DEBUG
Util::Util(void)
{
}
Util::~Util(void)
{
}
void __cdecl Util::LOG(const TCHAR *format, ...)
{
#ifdef MY_DEBUG
	TCHAR buf[4096], *p = buf;
	va_list args;
	va_start(args, format);
	int size = sizeof buf/2 - 1;
	p += _vsnwprintf(p, size, format, args);
	va_end(args);
	/*while ( p > buf  &&  iswspace(p[-1]))
	{
		*--p = '\0';
		*p++ = '\r';
		*p++ = '\n';
		*p   = '\0';
	}*/
	OutputDebugString(buf);
#endif
}
void Util::GetFileDirectory(TCHAR* fileDirectory)
{
	CString sPath;
	
	//获取主程序所在路径,存在sPath中
	GetModuleFileName(NULL,sPath.GetBufferSetLength (MAX_PATH+1),MAX_PATH);
	sPath.ReleaseBuffer ();
    int nPos;
	nPos=sPath.ReverseFind ('\\');
	sPath=sPath.Left (nPos);
	wcscpy(fileDirectory,sPath.GetBuffer());
	sPath.ReleaseBuffer();
}
void Util::GetDirectoryByFileName(const TCHAR* fileName,TCHAR* directory)
{
	CString path;
	path.Format(TEXT("%s"),fileName);
	int start = path.ReverseFind('\\');
	CString directory_str = path.Left(start);
	wcscpy(directory,directory_str.GetBuffer(1024));
	directory_str.ReleaseBuffer();
}
void Util::getUrl(TCHAR* url,TCHAR* q,TCHAR* from,TCHAR* to)
{
	CString tmp;
	tmp.Format(L"%s?client_id=%s&q=%s&from=%s&to=%s",
		API_URL,API_KEY,q,from,to);
	wcscpy(url,tmp.GetBuffer());
	tmp.ReleaseBuffer();
}

void Util::charTowchar(const char *chr, wchar_t *wchar, int size)  
{     
    MultiByteToWideChar( CP_ACP, 0, chr,  
        strlen(chr)+1, wchar, size/sizeof(wchar[0]));  
}

void Util::wcharTochar(const wchar_t *wchar, char *chr, int length)  
{  
    WideCharToMultiByte( CP_ACP, 0, wchar, -1,  
        chr, length, NULL, NULL );  
}  
int enc_get_utf8_size(const unsigned char pInput)
{
unsigned char c = pInput;
// 0xxxxxxx 返回0
// 10xxxxxx 不存在
// 110xxxxx 返回2
// 1110xxxx 返回3
// 11110xxx 返回4
// 111110xx 返回5
// 1111110x 返回6
if(c< 0x80) return 0;
if(c>=0x80 && c<0xC0) return -1;
if(c>=0xC0 && c<0xE0) return 2;
if(c>=0xE0 && c<0xF0) return 3;
if(c>=0xF0 && c<0xF8) return 4;
if(c>=0xF8 && c<0xFC) return 5;
if(c>=0xFC) return 6;
}
/***************************************************************************** 
 * 将一个字符的Unicode(UCS-2和UCS-4)编码转换成UTF-8编码. 
 * 
 * 参数: 
 *    unic     字符的Unicode编码值 
 *    pOutput  指向输出的用于存储UTF8编码值的缓冲区的指针 
 *    outsize  pOutput缓冲的大小 
 * 
 * 返回值: 
 *    返回转换后的字符的UTF8编码所占的字节数, 如果出错则返回 0 . 
 * 
 * 注意: 
 *     1. UTF8没有字节序问题, 但是Unicode有字节序要求; 
 *        字节序分为大端(Big Endian)和小端(Little Endian)两种; 
 *        在Intel处理器中采用小端法表示, 在此采用小端法表示. (低地址存低位) 
 *     2. 请保证 pOutput 缓冲区有最少有 6 字节的空间大小! 
 ****************************************************************************/  
int Util::enc_unicode_to_utf8_one(unsigned long unic, unsigned char *pOutput,  
        int outSize)  
{  
  
    if ( unic <= 0x0000007F )  
    {  
        // * U-00000000 - U-0000007F:  0xxxxxxx  
        *pOutput     = (unic & 0x7F);  
        return 1;  
    }  
    else if ( unic >= 0x00000080 && unic <= 0x000007FF )  
    {  
        // * U-00000080 - U-000007FF:  110xxxxx 10xxxxxx  
        *(pOutput+1) = (unic & 0x3F) | 0x80;  
        *pOutput     = ((unic >> 6) & 0x1F) | 0xC0;  
        return 2;  
    }  
    else if ( unic >= 0x00000800 && unic <= 0x0000FFFF )  
    {  
        // * U-00000800 - U-0000FFFF:  1110xxxx 10xxxxxx 10xxxxxx  
        *(pOutput+2) = (unic & 0x3F) | 0x80;  
        *(pOutput+1) = ((unic >>  6) & 0x3F) | 0x80;  
        *pOutput     = ((unic >> 12) & 0x0F) | 0xE0;  
        return 3;  
    }  
    else if ( unic >= 0x00010000 && unic <= 0x001FFFFF )  
    {  
        // * U-00010000 - U-001FFFFF:  11110xxx 10xxxxxx 10xxxxxx 10xxxxxx  
        *(pOutput+3) = (unic & 0x3F) | 0x80;  
        *(pOutput+2) = ((unic >>  6) & 0x3F) | 0x80;  
        *(pOutput+1) = ((unic >> 12) & 0x3F) | 0x80;  
        *pOutput     = ((unic >> 18) & 0x07) | 0xF0;  
        return 4;  
    }  
    else if ( unic >= 0x00200000 && unic <= 0x03FFFFFF )  
    {  
        // * U-00200000 - U-03FFFFFF:  111110xx 10xxxxxx 10xxxxxx 10xxxxxx 10xxxxxx  
        *(pOutput+4) = (unic & 0x3F) | 0x80;  
        *(pOutput+3) = ((unic >>  6) & 0x3F) | 0x80;  
        *(pOutput+2) = ((unic >> 12) & 0x3F) | 0x80;  
        *(pOutput+1) = ((unic >> 18) & 0x3F) | 0x80;  
        *pOutput     = ((unic >> 24) & 0x03) | 0xF8;  
        return 5;  
    }  
    else if ( unic >= 0x04000000 && unic <= 0x7FFFFFFF )  
    {  
        // * U-04000000 - U-7FFFFFFF:  1111110x 10xxxxxx 10xxxxxx 10xxxxxx 10xxxxxx 10xxxxxx  
        *(pOutput+5) = (unic & 0x3F) | 0x80;  
        *(pOutput+4) = ((unic >>  6) & 0x3F) | 0x80;  
        *(pOutput+3) = ((unic >> 12) & 0x3F) | 0x80;  
        *(pOutput+2) = ((unic >> 18) & 0x3F) | 0x80;  
        *(pOutput+1) = ((unic >> 24) & 0x3F) | 0x80;  
        *pOutput     = ((unic >> 30) & 0x01) | 0xFC;  
        return 6;  
    }  
  
    return 0;  
}  
int Util::enc_utf8_to_unicode_one(const unsigned char* pInput, unsigned long *Unic)  
{  
  
    // b1 表示UTF-8编码的pInput中的高字节, b2 表示次高字节, ...  
    char b1, b2, b3, b4, b5, b6;  
  
    *Unic = 0x0; // 把 *Unic 初始化为全零  
    int utfbytes = enc_get_utf8_size(*pInput);  
    unsigned char *pOutput = (unsigned char *) Unic;  
  
    switch ( utfbytes )  
    {  
        case 0:  
            *pOutput     = *pInput;  
            utfbytes    += 1;  
            break;  
        case 2:  
            b1 = *pInput;  
            b2 = *(pInput + 1);  
            if ( (b2 & 0xE0) != 0x80 )  
                return 0;  
            *pOutput     = (b1 << 6) + (b2 & 0x3F);  
            *(pOutput+1) = (b1 >> 2) & 0x07;  
            break;  
        case 3:  
            b1 = *pInput;  
            b2 = *(pInput + 1);  
            b3 = *(pInput + 2);  
            if ( ((b2 & 0xC0) != 0x80) || ((b3 & 0xC0) != 0x80) )  
                return 0;  
            *pOutput     = (b2 << 6) + (b3 & 0x3F);  
            *(pOutput+1) = (b1 << 4) + ((b2 >> 2) & 0x0F);  
            break;  
        case 4:  
            b1 = *pInput;  
            b2 = *(pInput + 1);  
            b3 = *(pInput + 2);  
            b4 = *(pInput + 3);  
            if ( ((b2 & 0xC0) != 0x80) || ((b3 & 0xC0) != 0x80)  
                    || ((b4 & 0xC0) != 0x80) )  
                return 0;  
            *pOutput     = (b3 << 6) + (b4 & 0x3F);  
            *(pOutput+1) = (b2 << 4) + ((b3 >> 2) & 0x0F);  
            *(pOutput+2) = ((b1 << 2) & 0x1C)  + ((b2 >> 4) & 0x03);  
            break;  
        case 5:  
            b1 = *pInput;  
            b2 = *(pInput + 1);  
            b3 = *(pInput + 2);  
            b4 = *(pInput + 3);  
            b5 = *(pInput + 4);  
            if ( ((b2 & 0xC0) != 0x80) || ((b3 & 0xC0) != 0x80)  
                    || ((b4 & 0xC0) != 0x80) || ((b5 & 0xC0) != 0x80) )  
                return 0;  
            *pOutput     = (b4 << 6) + (b5 & 0x3F);  
            *(pOutput+1) = (b3 << 4) + ((b4 >> 2) & 0x0F);  
            *(pOutput+2) = (b2 << 2) + ((b3 >> 4) & 0x03);  
            *(pOutput+3) = (b1 << 6);  
            break;  
        case 6:  
            b1 = *pInput;  
            b2 = *(pInput + 1);  
            b3 = *(pInput + 2);  
            b4 = *(pInput + 3);  
            b5 = *(pInput + 4);  
            b6 = *(pInput + 5);  
            if ( ((b2 & 0xC0) != 0x80) || ((b3 & 0xC0) != 0x80)  
                    || ((b4 & 0xC0) != 0x80) || ((b5 & 0xC0) != 0x80)  
                    || ((b6 & 0xC0) != 0x80) )  
                return 0;  
            *pOutput     = (b5 << 6) + (b6 & 0x3F);  
            *(pOutput+1) = (b5 << 4) + ((b6 >> 2) & 0x0F);  
            *(pOutput+2) = (b3 << 2) + ((b4 >> 4) & 0x03);  
            *(pOutput+3) = ((b1 << 6) & 0x40) + (b2 & 0x3F);  
            break;  
        default:  
            return 0;  
            break;  
    }  
  
    return utfbytes;  
}  

// 进行Url编码 UTF-8 
CString Util::UrlEncode(CString strUnicode)
{
  LPCWSTR unicode = T2CW(strUnicode);
  /*
  int WideCharToMultiByte(
UINT CodePage, //指定执行转换的代码页
DWORD dwFlags, //允许你进行额外的控制，它会影响使用了读音符号（比如重音）的字符
LPCWSTR lpWideCharStr, //指定要转换为宽字节字符串的缓冲区
int cchWideChar, //指定由参数lpWideCharStr指向的缓冲区的字符个数
LPSTR lpMultiByteStr, //指向接收被转换字符串的缓冲区
int cchMultiByte, //指定由参数lpMultiByteStr指向的缓冲区最大值
LPCSTR lpDefaultChar, //遇到一个不能转换的宽字符，函数便会使用pDefaultChar参数指向的字符
LPBOOL pfUsedDefaultChar //至少有一个字符不能转换为其多字节形式，函数就会把这个变量设为TRUE
);
  */
	 int len = WideCharToMultiByte(CP_UTF8, 0, unicode, -1, 0, 0, 0, 0);//获取utf8编码的字节长度
	 if (!len)
	  return strUnicode;
	 
	 char *utf8 = new char[len + 1];
	 char *utf8temp = utf8;
	 WideCharToMultiByte(CP_UTF8, 0, unicode, -1, utf8, len + 1, 0, 0);//转换成utf8编码


	 //转换成url编码——每个字节前面带%的字符串
	 utf8[len] = NULL;   
	 CString strTemp, strEncodeData; 
	 while (*utf8 != '\0') 
	 { 
	  strTemp.Format(_T("%%%2x"), (BYTE)*utf8); 
	  strEncodeData += strTemp; 
	  ++utf8; 
	 } 

	 delete []utf8temp;
	
	 return CString(strEncodeData);
}
bool Util::IsExistFile(LPCWSTR pszFileName)
{
	WIN32_FIND_DATA FindFileData;
	HANDLE hFind;

	hFind = FindFirstFile(pszFileName, &FindFileData);

	if (hFind == INVALID_HANDLE_VALUE) 
	return false;
	else
	{
	FindClose(hFind);
	return true;
	}
	return false;
}
