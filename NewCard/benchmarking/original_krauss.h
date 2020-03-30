#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

bool GeneralTextCompare(
        char* pTameText,
        char* pWildText
        )
{
    bool bCaseSensitive = false;
    char cAltTerminator = '\0';
    bool bMatch = true;
    char* pAfterLastWild = NULL;
    char t, w;
    while(1){
        t = *pTameText;
        w = *pWildText;
        if(!t || t == cAltTerminator){
            if(!w || w == cAltTerminator) {
                break;
            }
            else if(w == '*'){
                pWildText++;
                continue;
            }
            bMatch = false;
            break;
        }
        else {
            if(!bCaseSensitive){
                if( t >= 'A' && t <= 'Z'){
                    t+= ('a' - 'A');
                }
                if( w >= 'A' && w <= 'Z'){
                    w += ('a' - 'A');
                }
            }
            if(t != w){
                if(w == '*'){
                    pAfterLastWild = ++pWildText;
                    continue;
                }
                else if(pAfterLastWild){
                    pWildText = pAfterLastWild;
                    w = *pWildText;
                    if(!w || w == cAltTerminator){
                        break;
                    }
                    else if(t == w){
                        pWildText++;
                    }
                    pTameText++;
                    continue;
                }
                else{
                    bMatch = false;
                    break;
                }
            }
        }
        pTameText++;
        pWildText++;
    }
    return bMatch;
}