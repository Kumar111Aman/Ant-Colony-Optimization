/* ***************************(( Project ACO ))****************************** */

#include<iostream>
#include<vector>
#include<algorithm>
#include<cmath>
#include<map>
#include<climits>
#include<fstream>
#include<stdio.h>

#define ll long long
#define MAX 50
#define NO_OF_ANTS 15
#define NO_OF_ITERATIONS 5
#define RHO_LOCAL 0.35
#define RHO_GLOBAL 0.35
#define ALPHA 0.45
#define Q0 0.8
#define REL_PARAMETER 0.5
#define E 0.0001
#define Pbusy 215
#define Pidle 162
using namespace std;

double T0;

typedef struct v
{
	ll CPU, MEM;
	int vmID;
}VM;
typedef struct s
{
	ll ThreshCPU, ThreshMEM;
	int serverID;
	double peakPower;
}Server;
typedef struct ss
{
	ll array[MAX], NO_OF_IT;
	double P,W;
	bool dominated;
}solution;
typedef struct o
{
	double desirability,probability;
}qualifiedVM;

/* **** Function Declaration ************************************************ */

bool check(VM vm[],ll n,Server server[],ll m);
bool getServer(Server a, Server b);
bool getVM(VM a, VM b);
void generateGreedySolution(solution& S0, Server server[], Server serverO[], ll m, VM vm[], VM vmO[],ll n);
bool ifIll(solution S0, int n);
void initializePheromone(double pheromoneTrail[][MAX],solution S0,Server b[],ll m, VM a[], ll n);
void randomSort(Server server[], ll len);
void setInitials(solution& a);
bool compatibleVM(solution antSol,int i,VM vm[],ll n,int j,Server s,map<ll,qualifiedVM> omega);
void prepare(qualifiedVM& t,int nextVM,solution antSol,VM vm[],ll n,int nextServer,Server server[],ll m,double pheromoneTrail[][MAX]);  //will use 1,2 to update t
double drawQ();
int exploitation(map<ll,qualifiedVM> omega);   //return maximum of probability
int exploration(map<ll,qualifiedVM> omega);    //return random number according to probability
void localPheromoneUpdate(double pheromoneTrail[][MAX], int i, int j);
void updatePandW(vector<solution>& CurrSol,VM vm[],ll n,Server server[],ll m);
void updateDomination(vector<solution>& ParetoSet,vector<solution>& CurrSol);
void updateParetoIteration(vector<solution>& ParetoSet);
void globalPheromoneUpdate(double pheromoneTrail[][MAX],solution s,ll t);
bool Isequal(solution a, solution b,int totalVM);
void deleteDuplicates(vector<solution>& ParetoSet,int totalVM);
void findIllSolution(vector<solution>& ParetoSet,int totalVM);
void printGreedySolution(solution s, int n, int m);
void printSolution(vector<solution> ParetoSet, int totalVM, int totalServers, solution S0);
//Supporting Functionss
void randomSortVM(VM vm[], ll len);
void viewVM(VM v[], ll n);
void viewServer(Server s[], ll m);

int main() {

	/* ********************* User Input ************************************* */

    FILE * fp;
    fp = fopen ("testcase.txt", "r");
    if (fp==NULL)
    {
        cout << "Unable to open file";
        return 0;
    }
    ll n,m;
    double vmM,vmC,serverM,serverC;

    // cout<<"Number of Virtual Machines: ";
    //cin>>n;
    fscanf(fp,"%lld",&n);
    //cout<<n<<endl;
    if(n>MAX)           // Putting Bounds
    {
        cout<<"Out of Bounds\n";
        return 0;
    }
    VM vm[n];
    VM vmClone[n];
    // cout<<"\nEnter CPU Demand and Memory Demand of each VM:";
    for(register int i=0;i<n;i++)
    {
        //cin>>vmC>>vmM;
        fscanf(fp,"%lf %lf",&vmC,&vmM);
        //cout<<vmC<<" "<<vmM<<endl;
        vm[i].CPU=(ll)(10000*vmC);
        vm[i].MEM=(ll)(10000*vmM);
        vm[i].vmID=i;
        vmClone[i].CPU=vm[i].CPU;
        vmClone[i].MEM=vm[i].MEM;
        vmClone[i].vmID=i;
    }
    // cout<<"\nNumber of Available Servers: ";
    //cin>>m;
    fscanf(fp,"%lld",&m);
    //cout<<m<<endl;
    if(m>MAX)           // Putting Bounds
    {
        cout<<"Out of Bounds\n";
        return 0;
    }
    Server server[m];
    Server serverClone[m];
    // cout<<"\nEnter Threshold CPU utilization and Threshold Memory utilization of each Server: ";
    for(register int i=0;i<m;i++)
    {
        //cin>>serverC>>serverM;
        fscanf(fp,"%lf %lf",&serverC,&serverM);
        //cout<<serverC<<" "<<serverM<<endl;
        server[i].ThreshCPU=(ll)(10000*serverC);
        server[i].ThreshMEM=(ll)(10000*serverM);
        server[i].peakPower=INT_MIN;
        server[i].serverID=i;
        serverClone[i].ThreshCPU=server[i].ThreshCPU;
        serverClone[i].ThreshMEM=server[i].ThreshMEM;
        serverClone[i].peakPower=INT_MIN;
        serverClone[i].serverID=i;
    }

    if(!check(vm,n,server,m))                    //Checking whether input data is in limit or not
    {
        cout<<"Input Data Error!!\n";
        return 0;
    }
    fclose(fp);

    /* *********************** Initialization ******************************* */

	vector<solution> ParetoSet;
	double pheromoneTrail[MAX][MAX];
	solution S0;
	setInitials(S0);
	generateGreedySolution(S0,serverClone,server,m,vmClone,vm,n);
	if(ifIll(S0,n))
	{
		cout<<"No Greedy Solution Found!!\n";
		return 0;
	}
	//printGreedySolution(S0,n,m);
	//randomSortVM(vm,n);
	initializePheromone(pheromoneTrail,S0,server,m,vm,n);

	/* *********************** Iterative Loop ******************************* */

	for(int it=1;it<=NO_OF_ITERATIONS;it++)
	{
		vector<solution> CurrentSol;
		for(register int k=1;k<=NO_OF_ANTS;k++)
		{
			randomSort(serverClone,m);
			solution antSol;
			setInitials(antSol);
			ll VMcount=0;
			for(int j=0;j<m;j++)
			{
				while(true)
				{
					map<ll,qualifiedVM> omega;
					for(register int i=0;i<n;i++)
					{
						if(compatibleVM(antSol,i,vm,n,serverClone[j].serverID,serverClone[j],omega))
						{
							qualifiedVM t;      		    			 		 //Create set of these VM's
							prepare(t,i,antSol,vm,n,server[j].serverID,server,m,pheromoneTrail);      //getdesirability();getprobability();
							omega[i]=t;
						}
					}

					if(omega.empty())						/*No remaining VM fits into server anymore OR Set is Empty */
					{
						break;
					}
					double q=drawQ();						//Draw q random number between 0 to 1
					int i;
					if(q<=Q0)
					{
						i=exploitation(omega);					//Exploitaion - Generate i(VM) to place in this host
						antSol.array[i]=serverClone[j].serverID;
					}
					else
					{
						i=exploration(omega);					//Exploration Get any number according to the probability distribution function formula
						antSol.array[i]=serverClone[j].serverID;
					}
					VMcount++;                              //Incrementing, since another VM got placed
					localPheromoneUpdate(pheromoneTrail,i,server[j].serverID);					//Apply Local updating Rule
				}
				if(VMcount==n)		/*If all VM's are placed */       //break of server choosing
				{
					break;			//a solution is created
				}
			}
			CurrentSol.push_back(antSol);
		}

		/*
		Evaluate the sets and update it accordingly
			1.Calculate values of P(S) and W(S) for each solution S
			2.Update the Pareto set according to dominations
		*/

		updatePandW(CurrentSol,vm,n,server,m);
		updateDomination(ParetoSet,CurrentSol);
		updateParetoIteration(ParetoSet);

		for(register int p=0;p<ParetoSet.size();p++)/* Each non-dominated solution in Pareto set */
		{
			globalPheromoneUpdate(pheromoneTrail,ParetoSet[p],it);						//Apply Global Updating
		}
	}
	deleteDuplicates(ParetoSet,n);
	findIllSolution(ParetoSet,n);
	printSolution(ParetoSet,n,m,S0);								//Print Pareto set
	/* Result Display Block */
	//putting solution in file
	ofstream solution ("DataFile.txt");
	if(solution.is_open())
    {
        solution<<n<<endl;
        for(int v=0;v<n;v++)
        {
            solution<<(double)vm[v].CPU/10000<<" "<<(double)vm[v].MEM/10000<<endl;
        }
        solution<<m<<endl;
        for(int v=0;v<m;v++)
        {
            solution<<(double)server[v].ThreshCPU/10000<<" "<<(double)server[v].ThreshMEM/10000<<endl;
        }
        if(ParetoSet.size()!=0) solution<<ParetoSet.size()<<endl;
        for(int v=0;v<ParetoSet.size();v++)
        {
            //solution<<v+1<<endl;
            solution<<endl;
            for(int q=0;q<n;q++)
            {
                solution<<q<<" "<<ParetoSet[v].array[q]<<endl;
            }
        }
        if(ParetoSet.size()==0)
        {
            solution<<1<<endl;
            for(int v=0;v<n;v++)
            {
                solution<<v<<" "<<S0.array[v]<<endl;
            }
        }
        solution.close();
    }
    else
    {
        cout<<"DataFile did not open\n";
    }
    ofstream solutionSet ("Solution.txt");
	if(solutionSet.is_open())
    {
        if(ParetoSet.size()!=0) solutionSet<<ParetoSet.size()<<endl;
        for(int v=0;v<ParetoSet.size();v++)
        {
            //solutionSet<<v+1<<endl;
            solutionSet<<endl;
            for(int q=0;q<n;q++)
            {
                solutionSet<<q<<" "<<ParetoSet[v].array[q]<<endl;
            }
        }
        if(ParetoSet.size()==0)
        {
            solutionSet<<1<<endl;
            for(int v=0;v<n;v++)
            {
                solutionSet<<v<<" "<<S0.array[v]<<endl;
            }
        }
        solutionSet.close();
    }
    else
    {
        cout<<"Solution File did not open\n";
    }
	/* Result Display Block */
	return 0;
}

/* **** Function Definitions ************************************************ */

bool check(VM vm[],ll n,Server server[],ll m)
{
	int maxCPU=INT_MIN,maxMEM=INT_MIN;
	ll vmCPUSum=0,vmMEMSum=0,serverCPUSum=0,serverMEMSum=0;
	for(register int i=0;i<n;i++)
	{
		if(vm[i].CPU>maxCPU)
		{
			maxCPU=vm[i].CPU;
		}
		if(vm[i].MEM>maxMEM)
		{
			maxMEM=vm[i].MEM;
		}
		vmCPUSum+=vm[i].CPU;
		vmMEMSum+=vm[i].MEM;
	}
	//cout<<vmCPUSum<<" "<<vmMEMSum<<endl;
	maxCPU*=2;
	maxMEM*=2;
	for(register int i=0;i<m;i++)
	{
		//cout<<server[i].ThreshCPU<<" "<<maxCPU<<endl;
		//cout<<server[i].ThreshMEM<<" "<<maxMEM<<endl;
		if(server[i].ThreshCPU<maxCPU)
		{
			return false;
		}
		if(server[i].ThreshMEM<maxMEM)
		{
			return false;
		}
		serverCPUSum+=server[i].ThreshCPU;
		serverMEMSum+=server[i].ThreshMEM;
	}
	//cout<<serverCPUSum<<" "<<serverMEMSum<<endl;
	if(serverCPUSum>vmCPUSum && serverMEMSum>vmMEMSum)
	return true;
	else return false;
}

bool getServer(Server a, Server b)				//compare function for randomSort function
{
	return ((((REL_PARAMETER)*(double)a.ThreshCPU)+((1-REL_PARAMETER)*(double)a.ThreshMEM))>(((REL_PARAMETER)*(double)b.ThreshCPU)+((1-REL_PARAMETER)*(double)b.ThreshMEM)));
}

bool getVM(VM a, VM b)							//compare function for randomSortVM function
{
	return ((((REL_PARAMETER)*(double)a.CPU)+((1-REL_PARAMETER)*(double)a.MEM))>(((REL_PARAMETER)*(double)b.CPU)+((1-REL_PARAMETER)*(double)b.MEM)));
}

void generateGreedySolution(solution& S0, Server server[], Server serverO[], ll m, VM vm[], VM vmO[],ll n)
{
	sort(server,server+m, getServer);
	// cout<<"servers :\n";
	// viewServer(server,m);
	sort(vm, vm+n, getVM);
	// cout<<"vm :\n";
	// viewVM(vm,n);
	ll j=0,remCPU=server[0].ThreshCPU,remMEM=server[0].ThreshMEM;
	double normalisedCPU=0,normalisedMEM=0,normalisedRemCPU=0,normalisedRemMEM=0;
	double resourceWastage=0,normalisedPowerConsumption=0;
	for(register int i=0;(i<n)&&(j<m);i++)
	{
		if(remCPU>=vm[i].CPU && remMEM>=vm[i].MEM)
		{
			S0.array[vm[i].vmID]=server[j].serverID;
			//S0.array[i]=j;
			remCPU-=(vm[i].CPU);remMEM-=(vm[i].MEM);
			continue;
		}
		else if(server[j].ThreshCPU!=remCPU)
		{
			normalisedCPU=(((server[j].ThreshCPU)-remCPU)/(double)server[j].ThreshCPU);
			normalisedMEM=(((server[j].ThreshMEM)-remMEM)/(double)server[j].ThreshMEM);
			normalisedRemCPU=(remCPU/(double)server[j].ThreshCPU);
			normalisedRemMEM=(remMEM/(double)server[j].ThreshMEM);
			resourceWastage+=((abs(normalisedRemCPU-normalisedRemMEM)+E)/((double)(normalisedCPU+normalisedMEM)));
			server[j].peakPower=(((Pbusy-Pidle)*(double)normalisedCPU)+Pidle);
			normalisedPowerConsumption+=((((Pbusy-Pidle)*(double)normalisedCPU)+Pidle)/server[j].peakPower);
			remCPU=server[j].ThreshCPU;remMEM=server[j].ThreshMEM;
		}
		j++;
		i--;
	}
	if(j<m && server[j].ThreshCPU!=remCPU)
	{
		normalisedCPU=(((server[j].ThreshCPU)-remCPU)/(double)server[j].ThreshCPU);
		normalisedMEM=(((server[j].ThreshMEM)-remMEM)/(double)server[j].ThreshMEM);
		normalisedRemCPU=(remCPU/(double)server[j].ThreshCPU);
		normalisedRemMEM=(remMEM/(double)server[j].ThreshMEM);
		resourceWastage+=((abs(normalisedRemCPU-normalisedRemMEM)+E)/((double)(normalisedCPU+normalisedMEM)));
		server[j].peakPower=(((Pbusy-Pidle)*(double)normalisedCPU)+Pidle);
		normalisedPowerConsumption+=((((Pbusy-Pidle)*(double)normalisedCPU)+Pidle)/server[j].peakPower);
	}

	S0.P=normalisedPowerConsumption;
	S0.W=resourceWastage;
}

bool ifIll(solution S0, int n)
{
	for(register int i=0;i<n;i++)
	{
		if(S0.array[i]==-1)
		{
			return true;
		}
	}
	return false;
}

void initializePheromone(double pheromoneTrail[][MAX],solution S0,Server b[],ll m, VM a[], ll n)
{
	double t0=1/(double)(n*(S0.P+S0.W));
	T0=t0;
	for(register int i=0;i<n;i++)
	{
		for(register int j=0;j<m;j++)
		{
			pheromoneTrail[i][j]=t0;
		}
	}
}

void randomSort(Server server[], ll len)
{
	for(register int i=len-1;i>=0;i--)
	{
		ll index=rand()%(i+1);
		Server temp=server[i];
		server[i]=server[index];
		server[index]=temp;
	}
}

void viewServer(Server s[], ll m)
{
	for(int i=0;i<m;i++)
	{
		cout<<s[i].serverID<<" "<<s[i].ThreshCPU<<" "<<s[i].ThreshMEM<<endl;
	}
}

void randomSortVM(VM vm[], ll len)
{
	for(register int i=len-1;i>=0;i--)
	{
		ll index=rand()%(i+1);
		VM temp=vm[i];
		vm[i]=vm[index];
		vm[index]=temp;
	}
}

void viewVM(VM v[], ll n)
{
	for(int i=0;i<n;i++)
	{
		cout<<v[i].vmID<<" "<<v[i].CPU<<" "<<v[i].MEM<<endl;
	}
}

void setInitials(solution& a)
{
	a.P=a.W=0;
	for(register int i=0;i<MAX;i++)
	{
		a.array[i]=-1;
	}
	a.dominated=false;
	a.NO_OF_IT=0;
}

bool compatibleVM(solution antSol,int i,VM vm[],ll n,int j,Server s,map<ll,qualifiedVM> omega)
{
	ll remCPU=s.ThreshCPU,remMEM=s.ThreshMEM;
	if(antSol.array[i]==-1 && (omega.find((ll)i)==omega.cend()))
	{
		for(register int k=0;k<n;k++)
		{
			if(antSol.array[k]!=-1 && antSol.array[k]==j)
			{
				remCPU-=(vm[k].CPU);
				remMEM-=(vm[k].MEM);
			}
		}
		if(remCPU>=vm[i].CPU && remMEM>=vm[i].MEM)
		{
			return true;
		}
		else return false;
	}
	return false;
}

void prepare(qualifiedVM& t,int nextVM,solution antSol,VM vm[],ll n,int nextServer,Server server[],ll m,double pheromoneTrail[][MAX])
{
	double sum[9][MAX];
	for(register int i=0;i<9;i++)
	{
		for(register int j=0;j<MAX;j++)
		{
			sum[i][j]=0;
		}
	}
	for(register int i=0;i<n;i++)
	{
		if(antSol.array[i]!=-1)
		{
			//Used CPU
			sum[0][antSol.array[i]]+=vm[i].CPU;
			//Used MEM
			sum[1][antSol.array[i]]+=vm[i].MEM;
		}
	}
	//For the CurrentVM on CurrentServer placement
	sum[0][nextServer]+=vm[nextVM].CPU;
	sum[1][nextServer]+=vm[nextVM].MEM;
	double summationP=0,summationW=0;
	for(register int i=0;i<MAX;i++)        //For each server
	{
		if(sum[0][i])                      //Here we can reduce the number of rows by taking variable for each row
		{
			//Used CPU to total CPU
			sum[2][i]=(sum[0][i]/(double)server[i].ThreshCPU);
			//Used MEM to total MEM
			sum[3][i]=(sum[1][i]/(double)server[i].ThreshMEM);
			//Remaining CPU to total MEM
			sum[4][i]=((server[i].ThreshCPU-sum[0][i])/(double)server[i].ThreshCPU);
			//Remaining MEM to total MEM
			sum[5][i]=(server[i].ThreshMEM-sum[1][i])/(double)server[i].ThreshMEM;
			//Calculating P
			sum[6][i]=((Pbusy-Pidle)*sum[2][i])+Pidle;
			if(sum[6][i]>server[i].peakPower)
			{
				server[i].peakPower=sum[6][i];
			}
			//Calculating normalised power of each server
			sum[7][i]=sum[6][i]/(double)server[i].peakPower;
			summationP+=sum[7][i];
			//Calculating W
			sum[8][i]=(abs(sum[4][i]-sum[5][i])+E)/(double)(sum[2][i]+sum[3][i]);
			summationW+=sum[8][i];
		}
	}
	t.desirability=/*for objective 1*/1/(double)(E+summationP)+/*for objective 2*/1/(double)(E+summationW);
	t.probability=(ALPHA*pheromoneTrail[nextVM][nextServer])+((1-ALPHA)*t.desirability);
}

double drawQ()
{
	return ((double)(rand()%101))/100;
}

int exploitation(map<ll,qualifiedVM> omega)
{
	map<ll,qualifiedVM>::iterator it;
	double max=INT_MIN;
	int index=-1;
	for(it=omega.begin();it!=omega.end();it++)
	{
		if(it->second.probability>max)
		{
			max=it->second.probability;
			index=it->first;
		}
	}
	return index;
}

int exploration(map<ll,qualifiedVM> omega)                  //This is not working properly
{
	double probabDistribution[2][MAX];
	for(int i=0;i<2;i++)
	{
		for(int j=0;j<MAX;j++)
		{
			probabDistribution[i][j]=0;
		}
	}
	int i=0;
	double sum=0;
	map<ll,qualifiedVM>:: iterator it=omega.begin();
	sum=probabDistribution[0][i]=(it->second.probability)*100;
	probabDistribution[1][i++]=(it->first);

	it++;
	for(it;it!=omega.end();it++)
	{
		probabDistribution[0][i]=probabDistribution[0][i-1]+(it->second.probability)*100;
		sum=probabDistribution[0][i];
		probabDistribution[1][i++]=(it->first);
	}
	ll num=rand()%((ll)(ceil(sum+1)));
	for(register int j=0;j<i;j++)
	{
		if(num<=probabDistribution[0][j])
		{
			return (int)probabDistribution[1][j];
		}
	}
}

void localPheromoneUpdate(double pheromoneTrail[][MAX], int i, int j)
{
	pheromoneTrail[i][j]=(double)(1-RHO_LOCAL)*pheromoneTrail[i][j] + (RHO_LOCAL*T0);
}

void updatePandW(vector<solution>& CurrSol,VM vm[],ll n,Server server[],ll m)
{
	for(register int x=0;x<CurrSol.size();x++)
	{
		double sum[9][MAX];
		for(register int i=0;i<9;i++)
		{
			for(register int j=0;j<MAX;j++)
			{
				sum[i][j]=0;
			}
		}
		for(register int i=0;i<n;i++)
		{
			if(CurrSol[x].array[i]!=-1)
			{
				//Used CPU
				sum[0][CurrSol[x].array[i]]+=vm[i].CPU;
				//Used MEM
				sum[1][CurrSol[x].array[i]]+=vm[i].MEM;
			}
		}
		double summationP=0,summationW=0;
		for(register int i=0;i<MAX;i++)        //For each server
		{
			if(sum[0][i])                      //Here we can reduce the number of rows by taking variable for each row
			{
				//Used CPU to total CPU
				sum[2][i]=(sum[0][i]/(double)server[i].ThreshCPU);
				//Used MEM to total MEM
				sum[3][i]=(sum[1][i]/(double)server[i].ThreshMEM);
				//Remaining CPU to total MEM
				sum[4][i]=((server[i].ThreshCPU-sum[0][i])/(double)server[i].ThreshCPU);
				//Remaining MEM to total MEM
				sum[5][i]=(server[i].ThreshMEM-sum[1][i])/((double)server[i].ThreshMEM);
				//Calculating P
				sum[6][i]=((Pbusy-Pidle)*sum[2][i])+Pidle;
				//Calculating normalised power of each server
				sum[7][i]=sum[6][i]/(double)server[i].peakPower;
				summationP+=sum[7][i];
				//Calculating W
				sum[8][i]=(abs(sum[4][i]-sum[5][i])+E)/(double)(sum[2][i]+sum[3][i]);
				summationW+=sum[8][i];
			}
		}
		CurrSol[x].P=summationP;
		CurrSol[x].W=summationW;
	}
}

void updateDomination(vector<solution>& ParetoSet,vector<solution>& CurrSol)
{
	if(CurrSol.empty())
	{
		cout<<"Empty Current Solution Set\n";
		return ;
	}
	if(!ParetoSet.empty())
	{
		for(register int i=0;i<ParetoSet.size();i++)               //All solutions of pareto sets are compared to all elements of currentset
		{
			for(register int j=0;j<CurrSol.size();j++)
			{
				if(abs(ParetoSet[i].P-1)<abs(CurrSol[j].P-1) || ParetoSet[i].W<CurrSol[j].W)
				{
					if(abs(ParetoSet[i].P-1)<=abs(CurrSol[j].P-1) && ParetoSet[i].W<=CurrSol[j].W)
					{
						CurrSol[j].dominated=true;
					}
				}
			}
		}
	}
	for(register int i=0;i<CurrSol.size();i++)                   //All elements of current set are compared to all elements of currentset
	{
		for(register int j=0;j<CurrSol.size();j++)
		{
			if(i!=j && !CurrSol[j].dominated)
			{
				if(abs(CurrSol[i].P-1)<abs(CurrSol[j].P-1) || CurrSol[i].W<CurrSol[j].W)
				{
					if(abs(CurrSol[i].P-1)<=abs(CurrSol[j].P-1) && CurrSol[i].W<=CurrSol[j].W)
					{
						CurrSol[j].dominated=true;
					}
				}
			}
		}
	}
	if(ParetoSet.empty())                          //If pareto set is empty, then simply adding solutions to it
	{
		for(register int i=0;i<CurrSol.size();i++)
		{
			if(!CurrSol[i].dominated)
			{
				ParetoSet.push_back(CurrSol[i]);
			}
		}
	}
	else
	{
		for(register int i=0;i<CurrSol.size();i++)
		{
			if(!CurrSol[i].dominated)			 //Otherwise comparing all non dominated solution to solution of paretoset
			{
				for(register int j=0;j<ParetoSet.size();j++)
				{
					if(abs(CurrSol[i].P-1)<abs(ParetoSet[j].P-1) || CurrSol[i].W<ParetoSet[j].W)
					{
						if(abs(CurrSol[i].P-1)<=abs(ParetoSet[j].P-1) && CurrSol[i].W<=ParetoSet[j].W)
						{
							ParetoSet.erase(ParetoSet.begin()+j);       //Erased Dominated solution from paretoset
						}
					}
				}
				ParetoSet.push_back(CurrSol[i]);
			}
		}
		for(register int i=0;i<ParetoSet.size();i++)
		{
			for(register int j=0;j<ParetoSet.size();j++)
			{
				if(i!=j && !ParetoSet[j].dominated)
				{
					if(abs(ParetoSet[i].P-1)<abs(ParetoSet[j].P-1) || ParetoSet[i].W<ParetoSet[j].W)
					{
						if(abs(ParetoSet[i].P-1)<=abs(ParetoSet[j].P-1) && ParetoSet[i].W<=ParetoSet[j].W)
						{
							ParetoSet.erase(ParetoSet.begin()+j);       //Erased Dominated solution from paretoset
						}
					}
				}
			}
		}
	}
}

void updateParetoIteration(vector<solution>& ParetoSet)
{
	for(register int i=0;i<ParetoSet.size();i++)
	{
		ParetoSet[i].NO_OF_IT++;
	}
}

void globalPheromoneUpdate(double pheromoneTrail[][MAX],solution s,ll t)
{
	double lembda=(double)NO_OF_ANTS/((double)(t-s.NO_OF_IT+1));
	double B=((RHO_GLOBAL*lembda)/((double)(s.P+s.W)));
	for(register int i=0;i<MAX;i++)
	{
		pheromoneTrail[i][s.array[i]]=(double)((1-RHO_GLOBAL)*pheromoneTrail[i][s.array[i]])+B;
	}
}

bool Isequal(solution a, solution b,int totalVM)
{
	if(a.P==b.P && a.W==b.W)
	{
		for(register int i=0;i<totalVM;i++)
		{
			if(a.array[i]!=b.array[i])
			return false;
		}
		return true;
	}
	else return false;
}

void deleteDuplicates(vector<solution>& ParetoSet,int totalVM)
{
	vector<int> toDelete;
	for(register int i=0;i<ParetoSet.size();i++)
	{
		for(register int j=i+1;j<ParetoSet.size();j++)
		{
			if(Isequal(ParetoSet[i],ParetoSet[j],totalVM))
			{
				toDelete.push_back(j);
			}
		}
		while(!toDelete.empty())
		{
			int x=toDelete.back();
			toDelete.pop_back();
			ParetoSet.erase(ParetoSet.begin()+x);
		}
	}
}

void findIllSolution(vector<solution>& ParetoSet,int totalVM)
{
	vector<int> toDelete;
	for(register int i=0;i<ParetoSet.size();i++)
	{
		for(register int j=0;j<totalVM;j++)
		{
			if(ParetoSet[i].array[j]==-1)
			{
				toDelete.push_back(i);
				break;
			}
		}
	}
	while(!toDelete.empty())
	{
		int x=toDelete.back();
		toDelete.pop_back();
		ParetoSet.erase(ParetoSet.begin()+x);
	}
}
void printGreedySolution(solution s, int n, int m)
{
	for(int i=0;i<n;i++)
	{
		cout<<i<<"-"<<s.array[i]<<endl;
	}
	cout<<"Power: "<<s.P<<"\t"<<"Memory: "<<s.W<<endl;
}

void printSolution(vector<solution> ParetoSet, int totalVM, int totalServers, solution S0)
{
	if(ParetoSet.empty())
	{
		cout<<"No Solution Found!!\n";
		cout<<"Greedy Solution is:\n";
		printGreedySolution(S0,totalVM,totalServers);
		return;
	}
	for(register int i=0;i<ParetoSet.size();i++)
	{
		cout<<"Solution "<<i+1<<":"<<endl;
		for(register int j=0;j<totalVM;j++)
		{
			cout<<j<<" - "<<ParetoSet[i].array[j]<<endl;
		}
		cout<<"Power Wastage: "<<ParetoSet[i].P<<"\t"<<"Memory Wastage: "<<ParetoSet[i].W<<endl;
	}
}
