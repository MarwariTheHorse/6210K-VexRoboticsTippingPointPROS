#include <stdio.h>
#include <Python.h>


int main()
{
	PyObject *pName, *pModule;
	PyObject *pFunc, *pValue;


	Py_Initialize();

	pName = PyUnicode_FromString("pyemb3");
	pModule = PyImport_Import(pName);
	Py_XDECREF(pName);

	if(pModule)
	{
		pFunc = PyObject_GetAttrString(pModule, "getInteger");
		if(pFunc && PyCallable_Check(pFunc))
		{
			pValue = PyObject_CallObject(pFunc, NULL);

			printf_s("C: getInteger() = %ld\n", PyLong_AsLong(pValue));
			Py_XDECREF(pValue);
		}
		else
		{
			printf("ERROR: function getInteger()\n");
		}

		Py_XDECREF(pFunc);
	}
	else
	{
		printf_s("ERROR: Module not imported\n");
	}

	Py_XDECREF(pModule);

	Py_Finalize();
	return 0;
}