#include "vm_app.h"
//test to see if syslog is working in the most basic of ways
//straight out of the project write up!
int main() {
	char* p;
	p = (char*) vm_extend();
	vm_syslog(p, 5);
	p[0] = 'h';
	p[1] = 'e';
	p[2] = 'l';
	p[3] = 'l';
	p[4] = 'o';
	vm_syslog(p, 6);	
	vm_syslog(p, 5);
	char* d;
	d = (char*) vm_extend();
	d[0] = 't';
	d[1] = 'h';
	d[2] = 'e';
	d[3] = 'r';
	d[4] = 'o';
	vm_syslog(p, 5);
	vm_syslog(d, 5);
}
