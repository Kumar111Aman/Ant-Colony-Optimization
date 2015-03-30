#include <iostream>
#include<climits>
#include<math.h>
#include<stdlib.h>
#include<fstream>
using namespace std;

int main() {
    ofstream testcase ("testcase.txt");
    if(testcase.is_open())
    {
        int vm=5;
        long long totalCPU=0,totalMEM=0;
        int maxCPU=INT_MIN,maxMEM=INT_MIN;
        testcase<<vm<<endl;
        for(int i=0;i<vm;i++)
        {
            int cpu=(rand()%6)+1;
            if(cpu>maxCPU)
            maxCPU=cpu;
            int mem=(rand()%4)+1;
            if(mem>maxMEM)
            maxMEM=mem;
            totalCPU+=cpu;
            totalMEM+=mem;
            testcase<<cpu<<" "<<mem<<endl;
        }
        maxCPU*=2;
        maxMEM*=2;
        int server=(rand()%vm)+2;
        testcase<<server<<endl;
        for(int i=0;i<server-1;i++)
        {
            int sCpu=((rand()%6)+1)+maxCPU;
            int sMem=((rand()%4)+1)+maxMEM;
            totalCPU-=sCpu;
            totalMEM-=sMem;
            testcase<<sCpu<<" "<<sMem<<endl;
        }
        if(maxCPU>totalCPU)
        {
            int sCpu=((rand()%6)+1)+maxCPU;
            testcase<<sCpu<<" ";
        }
        else
        {
            int sCpu=((rand()%6)+1)+totalCPU;
            testcase<<sCpu<<" ";
        }
        if(maxMEM>totalMEM)
        {
            int sMem=((rand()%4)+1)+maxMEM;
            testcase<<sMem<<endl;
        }
        else
        {
            int sMem=((rand()%4)+1)+totalMEM;
            testcase<<sMem<<endl;
        }
        testcase.close();
    }
    else
    {
        cout<<"File did not open\n";
    }
    return 0;
}
