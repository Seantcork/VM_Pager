#include "vm_app.h"
//test to see if syslog is working in the most basic of ways
//straight out of the project write up!
int main() {
	char* a;
	char* b;
	char* c;
	a = (char*) vm_extend();
	b = (char*) vm_extend();
	c = (char*) vm_extend();
	vm_syslog(a, 1);
	vm_syslog(b, 1);
	vm_syslog(c, 1);	
	vm_syslog(b, 1);
	a[0] = 'h';


	char* d;
	char* e;
	char* f;

	d = (char*) vm_extend();
	e = (char*) vm_extend();
	f = (char*) vm_extend();
	vm_syslog(d, 1);
	vm_syslog(e, 1);
	vm_syslog(f, 1);
	e[0] = 'h';
	char* q;
	char* w;
	char* r;
	//six branches starting w write
	//#1
	q = (char*) vm_extend();
	w = (char*) vm_extend();
	r = (char*) vm_extend();

	q[0] = 'h';
	w[0] = 'h';
	r[0] = 'h';
	vm_syslog(q, 1);
	vm_syslog(w, 1);
	r[1] = '!';

	//#2
	char* u;
	char* i;
	char* o;
	u = (char*) vm_extend();
	i = (char*) vm_extend();
	o = (char*) vm_extend();

	u[0] = 'h';
	i[0] = 'h';
	o[0] = 'h';

	u[1] = 'h';
	i[1] = 'h';
	o[1] = 'h';
	//#3
	
	char* h;
	char* j;
	char* k;
	h = (char*) vm_extend();
	j = (char*) vm_extend();
	k = (char*) vm_extend();

	h[0] = 'h';
	j[0] = 'h';
	k[0] = 'h';

	vm_syslog(a, 1);
	//#4
	
	char* a1;
	char* b1;
	char* c1;
	a1 = (char*) vm_extend();
	b1 = (char*) vm_extend();
	c1 = (char*) vm_extend();

	a1[0] = 'h';
	b1[0] = 'h';
	c1[0] = 'h';
	
	vm_syslog(a1, 1);
	vm_syslog(c1, 1);
	vm_syslog(b1, 1);
	vm_syslog(a1, 1);
	vm_syslog(b1, 1);
	//#5
	char* a2;
	char* b2;
	char* c2;
	a2 = (char*) vm_extend();
	b2 = (char*) vm_extend();
	c2 = (char*) vm_extend();

	a2[0] = 'h';
	b2[0] = 'h';
	c2[0] = 'h';
	
	vm_syslog(a2, 1);
	vm_syslog(c2, 1);
	vm_syslog(b2, 1);
	vm_syslog(a2, 1);
	b2[1] = 'i';
}
