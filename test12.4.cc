#include "vm_app.h"
//test to see if syslog is working in the most basic of ways
//straight out of the project write up!
int main() {
	char* k;
	k = (char*) vm_extend();
	vm_syslog(k, 5);
	k = 0x0;
	vm_syslog(k, 1);
	vm_syslog(k, 1);
}
