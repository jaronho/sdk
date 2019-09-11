#ifndef _NAMEDPIPE_H_
#define _NAMEDPIPE_H_

#ifdef __cplusplus
extern "C"
{
#endif

typedef struct namedpipe_st {
    char* name;             /* pipe name, e.g. "test.in" or "/home/pipe.rd" */
    int fd;                 /* pip fd */
    int rd_flag;            /* whether pipe for read or write: 0.read, 1.write */
} namedpipe_st;

/*
 * Brief:	open a named pipe
 * Param:	name - pipe name, e.g. "test.in" or "/home/pipe.rd"
 *			rdFlag - whether pipe for read or write: 0.read, 1.write
 * Return:	namedpipe_st*
 */
extern namedpipe_st* namedpipe_open(const char* name, int rdFlag);

/*
 * Brief:	close a named pipe
 * Param:	np - named pipe
 * Return:	void
 */
extern void namedpipe_close(namedpipe_st* np);

/*
 * Brief:	get a pipe name
 * Param:	np - named pipe
 * Return:	const char*
 */
extern const char* namedpipe_name(namedpipe_st* np);

/*
 * Brief:	get a pipe fd
 * Param:	np - named pipe
 * Return:	int
 */
extern int namedpipe_fd(namedpipe_st* np);

/*
 * Brief:	read data from pipe, need called for loop
 * Param:	np - named pipe
 *          buffer - to place the read data
 *          count - current read bytes number
 * Return:	0.ok
            1.np is null
            2.np->fd = -1 is invalid
            3.buffer is null or count <= 0
            4.read no data form pipe, need to wait next called
 */
extern int namedpipe_read(namedpipe_st* np, void* buffer, unsigned int count);

/*
 * Brief:	write data to pipe
 * Param:	np - named pipe
 *          buffer - data to write
 *          count - current write bytes number
 * Return:	0.ok
            1.np is null
            2.np->fd = -1 is invalid
            3.buffer is null or count <= 0
            4.write fail
 */
extern int namedpipe_write(namedpipe_st* np, const void* buffer, unsigned int count);

#ifdef __cplusplus
}
#endif

#endif  /* _NAMEDPIPE_H_ */
