#include "vm_app.h"
//test to see if syslog is working in the most basic of ways
//straight out of the project write up!
int main() {
	char* p;
	p = (char*) vm_extend();
	p[0] = 'h';
	p[1] = 'e';
	p[2] = 'l';
	p[3] = 'l';
	p[4] = 'o';
	vm_syslog(p, 5);
	vm_syslog(p, 1);
	vm_syslog(p, 1000000000);
	vm_syslog(p, -1);
}
