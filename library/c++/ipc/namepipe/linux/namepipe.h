#ifndef _NAMEPIPE_H_
#define _NAMEPIPE_H_

#ifdef __cplusplus
extern "C"
{
#endif

typedef void (*nap_callback_msg)(const char* name, long len, const char* buffer);

#ifdef __cplusplus
}
#endif

#endif  /* _NAMEPIPE_H_ */
