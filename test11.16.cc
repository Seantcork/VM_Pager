#include "vm_app.h"
//test to see if syslog is working in the most basic of ways
//straight out of the project write up!
int main() {
	char* p;
	char* e;
	char* d;
	char* t;
	p = (char*) vm_extend();
	e = (char*) vm_extend();
	d = (char*) vm_extend();
	t = (char*) vm_extend();
	char* m;
	char* n;
	char* b;
	char* v;
	m = (char*) vm_extend();
	n = (char*) vm_extend();
	b = (char*) vm_extend();
	v = (char*) vm_extend();
	char* g;
	char* h;
	char* j;
	char* k;
	g = (char*) vm_extend();
	h = (char*) vm_extend();
	j = (char*) vm_extend();
	k = (char*) vm_extend();
	vm_syslog(p, 5);
	vm_syslog(e, 5);
	vm_syslog(d, 5);
	vm_syslog(t, 5);
	vm_syslog(m, 5);
	vm_syslog(n, 5);
	vm_syslog(b, 5);
	vm_syslog(v, 5);
	vm_syslog(g, 5);
	vm_syslog(h, 5);
	vm_syslog(j, 5);
	vm_syslog(k, 5);
	p[0] = 'h';
	p[1] = 'e';
	p[2] = 'l';
	p[3] = 'l';
	p[4] = 'o';
	vm_syslog(p, 5);
	d[0] = 't';
	d[1] = 'h';
	d[2] = 'e';
	d[3] = 'r';
	d[4] = 'o';
	vm_syslog(d, 5);
	e[0] = 'q';
	e[1] = 'w';
	e[2] = 'e';
	e[3] = 'r';
	e[4] = 't';
	vm_syslog(e, 5);
	t[0] = 'k';
	t[1] = 'j';
	t[2] = 'h';
	t[3] = 'g';
	t[4] = 'f';
	vm_syslog(t, 5);
	m[0] = 'k';
	m[1] = 'j';
	m[2] = 'h';
	m[3] = 'g';
	m[4] = 'f';
	vm_syslog(m, 5);
	n[0] = 'k';
	n[1] = 'j';
	n[2] = 'h';
	n[3] = 'g';
	n[4] = 'f';
	vm_syslog(n, 5);
	b[0] = 'k';
	b[1] = 'j';
	b[2] = 'h';
	b[3] = 'g';
	b[4] = 'f';
	vm_syslog(b, 5);
	v[0] = 'k';
	v[1] = 'j';
	v[2] = 'h';
	v[3] = 'g';
	v[4] = 'f';
	vm_syslog(v, 5);
	g[0] = 'k';
	g[1] = 'j';
	g[2] = 'h';
	g[3] = 'g';
	g[4] = 'f';
	vm_syslog(g, 5);
	h[4] = 'f';
	j[4] = 'f';
	k[4] = 'f';
}
