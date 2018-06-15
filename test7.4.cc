#include "vm_app.h"
//test to see if syslog is working in the most basic of ways
//straight out of the project write up!
int main() {
	char* p;
	p = (char*) vm_extend();
	char* r;
	char* s;
	char* q;
	char* i;
	r = (char*) vm_extend();
	s = (char*) vm_extend();
	q = (char*) vm_extend();
	i = (char*) vm_extend();
	p[0] = 'h';
	r[0] = 'e';
	s[0] = 'l';
	q[0] = 'l';
	i[0] = 'o';
	i[10] = '!';
	vm_syslog(p, VM_PAGESIZE*5);
	vm_syslog(r, VM_PAGESIZE*4);
	vm_syslog(s, VM_PAGESIZE*3);
	vm_syslog(q, VM_PAGESIZE*2);
	vm_syslog(i, VM_PAGESIZE*1);
}
