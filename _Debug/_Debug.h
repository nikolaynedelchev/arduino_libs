#pragma once

#ifdef DEBUG_MODE_ON
#define DBG_F_PRINTLN(txt) Serial.println(F(txt));
#define DBG_F_PRINT(txt) Serial.print(F(txt));
#define DBG_PRINTLN(txt) Serial.println(txt);
#define DBG_PRINT(txt) Serial.print(txt);
#else
#define DBG_F_PRINTLN(txt) 
#define DBG_F_PRINT(txt) 
#define DBG_PRINTLN(txt)
#define DBG_PRINT(txt)
#endif