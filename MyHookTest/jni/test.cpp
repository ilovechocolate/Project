#include <iostream>
#include <string.h>

using namespace std;

int main()
{
    cout<<"this is a qt test!"<<endl;

    char ch1[] = {"hello world!"};
    char ch2[] = {"welcome"};

    cout<<strlen(ch1)<<endl<<strlen(ch2)<<endl;
    cout<<ch1<<endl<<ch2<<endl;
    cout<<sizeof(ch1)<<endl<<sizeof(ch2)<<endl;
    cout<<&ch1<<endl<<&ch2<<endl;

    cout<<endl;

    cout<<strcpy(ch1, ch2)<<endl;
    cout<<&ch1<<endl<<&ch2<<endl;

    cout<<strlen(ch1)<<endl<<strlen(ch2)<<endl;
    cout<<ch1<<endl<<ch2<<endl;
    cout<<sizeof(ch1)<<endl<<sizeof(ch2)<<endl;



    cout<<endl<<endl<<endl;

    int ch3[] = {1,2,3,4,5};
    cout<<sizeof(ch3)<<endl;
    cout<<sizeof(ch3)/sizeof(int)<<endl;

    return 0;
}
