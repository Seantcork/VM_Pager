#include "vm_app.h"
//test to see if syslog is working in the most basic of ways
//straight out of the project write up!
int main() {
	for (int i = 0; i < 3000; i ++) {
		vm_extend();
	}
}
