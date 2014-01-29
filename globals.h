#ifndef INC_GLOBALS_H_
#define INC_GLOBALS_H_

#define clamp(value, lb, ub) max( lb, min( ub, value ))
#define M_PI     3.14159265359
#define M_PI_2     1.57079632679489661923

#define sign(x) (((x) < 0) ? -1.0f : 1.0f )
#define getrandom(min, max) ((rand()%(int)(((max) + 1)-(min)))+ (min))
#define APP_CLASS "GAME"
#define APP_NAME "Path of Shadows"

void dbg( const char *fmt, ... );
int fatal( const char *fmt, ... );
void fatalErrorWindow(const char* err_message);

#endif
