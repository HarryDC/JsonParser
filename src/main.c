
#include "vector.h"
#include "json.h"

int main(int arc, const char* argv[])
{

#ifdef  _DEBUG
	vector_test_all();
	json_test_all();
#endif 

	return 0;
}