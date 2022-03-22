#include <pyhelper.hpp>

int main()
{
	CPyInstance pInstance;

	CPyObject p;
	p = PyLong_FromLong(50);
	printf_s("Value =  %ld\n", PyLong_AsLong(p));

	return 0;
}