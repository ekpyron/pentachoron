#ifndef TIGER_H
#define TIGER_H

#include <stdint.h>

class Tiger2
{
public:
	 Tiger2 (void);
	 ~Tiger2 (void);
	 void reset (void);
	 void consume (const void *ptr, uint64_t len);
	 void finalize (void);
	 void get (uint64_t res[3]);
private:
	 uint64_t result[3];
	 uint8_t temp[64];
	 uint8_t templen;
	 uint64_t length;
};

#endif /* !defined TIGER_H */
