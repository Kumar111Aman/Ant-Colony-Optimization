#include <iostream>
#include<climits>
using namespace std;

int main() {
	int vm=50;
	long long totalCPU=0,totalMEM=0;
	int maxCPU=INT_MIN,maxMEM=INT_MIN;
	cout<<vm<<endl;
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
		cout<<cpu<<" "<<mem<<endl;
	}
	maxCPU*=2;
	maxMEM*=2;
	int server=(rand()%vm)+2;
	cout<<server<<endl;
	for(int i=0;i<server-1;i++)
	{
		int sCpu=((rand()%6)+1)+maxCPU;
		int sMem=((rand()%4)+1)+maxMEM;
		totalCPU-=sCpu;
		totalMEM-=sMem;
		cout<<sCpu<<" "<<sMem<<endl;
	}
	if(maxCPU>totalCPU)
	{
		int sCpu=((rand()%6)+1)+maxCPU;
		cout<<sCpu<<" ";
	}
	else
	{
		int sCpu=((rand()%6)+1)+totalCPU;
		cout<<sCpu<<" ";
	}
	if(maxMEM>totalMEM)
	{
		int sMem=((rand()%6)+1)+maxMEM;
		cout<<sMem<<endl;
	}
	else
	{
		int sMem=((rand()%6)+1)+totalMEM;
		cout<<sMem<<endl;
	}
	return 0;
}