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
	char* d;
	d = (char*) vm_extend();
	d[0] = 't';
	d[1] = 'h';
	d[2] = 'e';
	d[3] = 'r';
	d[4] = 'o';
	vm_syslog(d, 5);
	char* e;
	e = (char*) vm_extend();
	e[0] = 'q';
	e[1] = 'w';
	e[2] = 'e';
	e[3] = 'r';
	e[4] = 't';
	char* t;
	t = (char*) vm_extend();
	t[0] = 'k';
	t[1] = 'j';
	t[2] = 'h';
	t[3] = 'g';
	t[4] = 'f';
	vm_syslog(t, 5);
	vm_syslog(p, 5);
}
