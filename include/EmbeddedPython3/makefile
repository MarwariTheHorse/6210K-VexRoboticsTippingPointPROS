CL_ARGS=/I "include/" /I "inc/" lib\python34.lib
CPP_ARGS=/EHsc /I "include/" /I "inc/" lib\python34.lib

all:
	copy lib\python3.dll python3.dll

	cl pyemb1.c $(CL_ARGS)
	cl pyemb2.c $(CL_ARGS)
	cl pyemb3.c $(CL_ARGS)	
	
	cl pyemb4.cpp $(CPP_ARGS)
	cl pyemb5.cpp $(CPP_ARGS)
	cl pyemb6.cpp $(CPP_ARGS)
	
	cl pyemb7.c $(CL_ARGS)

	
	
