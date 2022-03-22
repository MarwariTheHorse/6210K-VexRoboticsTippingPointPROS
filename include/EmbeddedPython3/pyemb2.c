#include <stdio.h>
#include <Python.h>

int main()
{
	PyObject* pValue;

	Py_Initialize();
	
	pValue = PyLong_FromLong(5);
	if(!pValue)
	{
		printf("ERROR\n");
	}
	else
	{
		printf("Value = %ld\n", PyLong_AsLong(pValue));
		Py_DECREF(pValue);
	}

	Py_Finalize();
	return 0;
}