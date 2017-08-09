msvc:
		cl /nologo /O2 /DTEST b2s_test.c b2s.c 
		cl /nologo /O2 /DTEST b2b_test.c b2b.c 
gnu:
		gcc -DTEST -Wall -O2 b2s_test.c b2s.c -ob2s_test	 
		gcc -DTEST -Wall -O2 b2b_test.c b2b.c -ob2b_test	 
clang:
		clang -DTEST -Wall -O2 b2s_test.c b2s.c -ob2s_test	 
		clang -DTEST -Wall -O2 b2b_test.c b2b.c -ob2b_test    