#ifndef COMMON_H
#define COMMON_H

#define BITS32(n, h, l) (((n) << (32-(h)-1)) >> (32-(h)-1+(l)))

//extern pc_rule *rule;  

void dump_ip(unsigned int ip)
{
	printf("%d.", BITS32(ip, 31, 24));
	printf("%d.", BITS32(ip, 23, 16));
	printf("%d.", BITS32(ip, 15, 8));
	printf("%d", BITS32(ip, 7, 0));
}



unsigned int log2(unsigned int n)
{
	int		k = 0;

	while (n >>= 1)
		k++;
	return	k;
}


#endif
