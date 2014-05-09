#include "lock.h"


#define SECONDSINDAY 86400


void _hidedigits(nschar* string);
void _showdigits(nschar* string);


#define _LOCK_FORMAT \
NS_MAKE_FMT( NS_FMT_FLAG_ZERO_FILL "10", NS_FMT_TYPE_UINT )\
NS_MAKE_FMT( NS_FMT_FLAG_ZERO_FILL "10", NS_FMT_TYPE_UINT )\
NS_MAKE_FMT( NS_FMT_FLAG_ZERO_FILL "10", NS_FMT_TYPE_UINT )\
NS_MAKE_FMT( NS_FMT_FLAG_ZERO_FILL "10", NS_FMT_TYPE_UINT )


// compute number of days left for license
nsint GetLockDays(nschar* string)
{
    nsint daysleft;
    nschar *p,startstring[11],currentstring[11],lengthstring[11],sumstring[11];
    nsuint32 start,current,length,sum;
    nsuint32 t;
    nssize len;

    //make sure that string has 40 characters
    len = ns_ascii_strlen(string);
    if(len!=40) return 0;


    //verify that all characters are uppercase
    p = string;
    while(*p != '\0')
    {
        if(!ns_ascii_isupper(*p)) return 0;
        p++;
    }


    //copy the substrings
    ns_ascii_strncpy(startstring,string,10);
    startstring[10] = '\0';
    ns_ascii_strncpy(currentstring,string+10,10);
    currentstring[10] = '\0';
    ns_ascii_strncpy(lengthstring,string+20,10);
    lengthstring[10] = '\0';
    ns_ascii_strncpy(sumstring,string+30,10);
    sumstring[10] = '\0';

    //show digits in strings
    _showdigits(startstring);
    _showdigits(currentstring);
    _showdigits(lengthstring);
    _showdigits(sumstring);

    //scan the integers out of strings
    _ns_sscan(startstring,"%u",&start);
    _ns_sscan(currentstring,"%u",&current);
    _ns_sscan(lengthstring,"%u",&length);
    _ns_sscan(sumstring,"%u",&sum);

    //verify sum
    if(sum!=(start+current+length)) return 0;

    //verify that clock has not been reset
    t = ( nsuint32 )_ns_time();
    if(t<current) return 0;

    // report days left in license
    daysleft = (start/SECONDSINDAY)+(length/SECONDSINDAY)-(t/SECONDSINDAY);
    if(daysleft<0) daysleft = 0;

    return daysleft;
}


// validates and updates a lock based on time and DAYSTOEND
// return 1 on success or 0 if invalid or expired lock
nsint UpdateLock(nschar* string)
{
    nschar *p,startstring[11],currentstring[11],lengthstring[11],sumstring[11];
    nsuint32 start,current,length,sum;
    nsuint32 t;
    nssize len;


    //make sure that string has 40 characters
    len = ns_ascii_strlen(string);
    if(len!=40) return 0;


    //verify that all characters are uppercase
    p = string;
    while(*p != '\0')
    {
        if(!ns_ascii_isupper(*p)) return 0;
        p++;
    }


    //copy the substrings
    ns_ascii_strncpy(startstring,string,10);
    startstring[10] = '\0';
    ns_ascii_strncpy(currentstring,string+10,10);
    currentstring[10] = '\0';
    ns_ascii_strncpy(lengthstring,string+20,10);
    lengthstring[10] = '\0';
    ns_ascii_strncpy(sumstring,string+30,10);
    sumstring[10] = '\0';

    //show digits in strings
    _showdigits(startstring);
    _showdigits(currentstring);
    _showdigits(lengthstring);
    _showdigits(sumstring);

    //scan the integers out of strings
    _ns_sscan(startstring,"%u",&start);
    _ns_sscan(currentstring,"%u",&current);
    _ns_sscan(lengthstring,"%u",&length);
    _ns_sscan(sumstring,"%u",&sum);

    //verify sum
    if(sum!=(start+current+length)) return 0;

    //verify that clock has not been reset
    t = ( nsuint32 )_ns_time();
    if(t<current) return 0;

    //verify that current time is within license term
    if(t>(start+length)) return 0;

    // update lock and return
    ns_sprint(string,_LOCK_FORMAT,start,t,length,start+t+length);
    _hidedigits(string);

    return 1;
}



void _hidedigits(nschar* string)
{
    nschar* p;
    nsuint16 r;
    static nsint ____lock_init=0;

    // seed random number on first pass
    if(!____lock_init)
    {
        ns_srand( ( nsuint32 )_ns_time() );
        ____lock_init = 1;
    }

    p = string;
    while(*p != '\0')
    {
        if(*p<'6')
        {
            r = ns_rand()%3;
            switch(r)
            {
                case 0:
                    *p = *p + 'A'-'0';
                    break;
                case 1:
                    *p = *p + 'K'-'0';
                    break;
                case 2:
                    *p = *p + 'Q'-'0';
                    break;
            }
        }
        else
        {
            r = ns_rand()%2;
            switch(r)
            {
                case 0:
                    *p = *p + 'A'-'0';
                    break;
                case 1:
                    *p = *p + 'Q'-'0';
                    break;
            }
        }
        p++;
    }
}

void _showdigits(nschar* string)
{
    nschar* p;

    p = string;
    while(*p != '\0')
    {
        if(*p<'K') *p = *p -'A'+'0';
        else if (*p<'Q') *p = *p -'K'+'0';
        else *p = *p -'Q'+'0';
        p++;
    }
}

