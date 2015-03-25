
/* ***************************(( Project ACO ))****************************** */

#include <iostream>
#include <vector>
#include<algorithm>
#include<cmath>
#include<map>
#include<climits>

#define ll long long
#define MAX 100
#define NO_OF_ANTS 10
#define NO_OF_ITERATIONS 50
#define RHO_LOCAL 20
#define RHO_GLOBAL 30
#define ALPHA 0.5
#define Q0 0.3
#define REL_PARAMETER 0.5
#define E 0.0001
#define Pbusy 215
#define Pidle 162
using namespace std;

double T0;

typedef struct v
{
	ll CPU, MEM;
}VM;
typedef struct s
{
	ll ThreshCPU, ThreshMEM;
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

/* Done */ bool getServer(Server a, Server b);
/* Done */ bool getVM(VM a, VM b);
/* Done */ void generateGreedySolution(solution& S0, Server server[],ll m, VM vm[],ll n);
/* Done */ void initializePheromone(double pheromoneTrail[][MAX],solution S0,Server b[],ll m, VM a[], ll n);
/* Done */ void randomSort(Server server[], ll len);
/* Done */ void randomSortVM(VM vm[], ll len);
/* Done */ void setInitials(solution& a);
/* Done */ bool compatibleVM(solution antSol,int i,VM vm[],ll n,int j,Server s,map<ll,qualifiedVM> omega);
/* Done */ void prepare(qualifiedVM& t,int nextVM,solution antSol,VM vm[],ll n,int nextServer,Server server[],ll m,double pheromoneTrail[][MAX]);  //will use 1,2 to update t
/* Done */ double drawQ();
/* Done */ int exploitation(map<ll,qualifiedVM> omega);   //return maximum of probability
/* Done */ int exploration(map<ll,qualifiedVM> omega);    //return random number according to probability
/* Done */ void localPheromoneUpdate(double pheromoneTrail[][MAX], int i, int j);
/* Done */ void updatePandW(vector<solution>& CurrSol,VM vm[],ll n,Server server[],ll m);
/* Done */ void updateDomination(vector<solution>& ParetoSet,vector<solution>& CurrSol);
/* Done */ void updateParetoIteration(vector<solution>& ParetoSet);
/* Done */ void globalPheromoneUpdate(double pheromoneTrail[][MAX],solution s,ll t);
/* Done */ void printSolution(vector<solution> ParetoSet,int totalVM);

int main() {
	
	/* ********************* User Input ************************************* */
	
	ll n,m;
	// cout<<"Number of Virtual Machines: ";
	cin>>n;
	// cout<<"\nNumber of Available Servers: ";
	cin>>m;
	
	if(n>MAX || m>MAX)           // Putting Bounds
	{
		cout<<"Out of Bounds\n";
		return 0;
	}
	
	VM vm[n];
	Server server[m];
	// cout<<"\nEnter CPU Demand and Memory Demand of each VM:";
	for(register int i=0;i<n;i++) cin>>vm[i].CPU>>vm[i].MEM;
	// cout<<"\nEnter Threshold CPU utilization and Threshold Memory utilization of each Server: ";
	for(register int i=0;i<m;i++) {	cin>>server[i].ThreshCPU>>server[i].ThreshMEM; server[i].peakPower=INT_MIN; }
	
	/* *********************** Initialization ******************************* */
	
	vector<solution> ParetoSet;
	double pheromoneTrail[MAX][MAX];
	solution S0;
	setInitials(S0);
	generateGreedySolution(S0,server,m,vm,n);
	randomSortVM(vm,n);
	initializePheromone(pheromoneTrail,S0,server,m,vm,n);
	
	/* *********************** Iterative Loop ******************************* */
	ll it=1;
	while(it<=NO_OF_ITERATIONS)
	{
		vector<solution> CurrentSol;
		for(register int k=1;k<=NO_OF_ANTS;k++)
		{
			randomSort(server,m);
			solution antSol;
			setInitials(antSol);
			ll VMcount=0;
			int j=-1;
			while(true && j<m)
			{
				j++;										//choose a server - j
				while(true)
				{
					map<ll,qualifiedVM> omega;
					for(register int i=0;i<n;i++)
					{
						if(compatibleVM(antSol,i,vm,n,j,server[j],omega))
						{
							qualifiedVM t;      		    			 		 //Create set of these VM's
							prepare(t,i,antSol,vm,n,j,server,m,pheromoneTrail);      //getdesirability();getprobability();
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
						antSol.array[i]=j;
					}
					else
					{
						i=exploration(omega);					//Exploration Get any number according to the probability distribution function formula
						antSol.array[i]=j;
					}
					VMcount++;                              //Incrementing, since another VM got placed
					localPheromoneUpdate(pheromoneTrail,i,j);					//Apply Local updating Rule
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
		it++;                                               //For outer loop
	}
	printSolution(ParetoSet,n);								//Print Pareto set
	/* Result Display Block */
	//
	//                                                   //Amandeep's part
	//
	//
	/* Result Display Block */
	return 0;
}

/* **** Function Definitions ************************************************ */

bool getServer(Server a, Server b)				//compare function for randomSort function
{
	return ((((REL_PARAMETER)*a.ThreshCPU)+((1-REL_PARAMETER)*a.ThreshMEM))>(((REL_PARAMETER)*b.ThreshCPU)+((1-REL_PARAMETER)*b.ThreshMEM)));
}

bool getVM(VM a, VM b)							//compare function for randomSortVM function
{
	return ((((REL_PARAMETER)*a.CPU)+((1-REL_PARAMETER)*a.MEM))<(((REL_PARAMETER)*b.CPU)+((1-REL_PARAMETER)*b.MEM)));
}

void generateGreedySolution(solution& S0, Server server[], ll m, VM vm[], ll n)
{
	sort(server,server+m, getServer);
	sort(vm, vm+n, getVM);
	ll j=0,remCPU=server[0].ThreshCPU,remMEM=server[0].ThreshMEM;
	double normalisedCPU=0,normalisedMEM=0,normalisedRemCPU=0,normalisedRemMEM=0;
	double resourceWastage=0,normalisedPowerConsumption=0;
	for(register int i=0;i<n;i++)
	{
		if(remCPU>=vm[i].CPU && remMEM>=vm[i].MEM)
		{
			S0.array[i]=j;
			remCPU-=vm[i].CPU;remMEM-=vm[i].MEM;
		}
		else{
			normalisedCPU=(((server[j].ThreshCPU)-remCPU)/(double)server[j].ThreshCPU);
			normalisedMEM=(((server[j].ThreshMEM)-remMEM)/(double)server[j].ThreshMEM);
			normalisedRemCPU=(remCPU/(double)server[j].ThreshCPU);
			normalisedRemMEM=(remMEM/(double)server[j].ThreshMEM);
			resourceWastage+=((abs(normalisedRemCPU-normalisedRemMEM)+E)/(double)(normalisedCPU+normalisedMEM));
			server[j].peakPower=((Pbusy-Pidle)*(double)normalisedCPU+Pidle);
			normalisedPowerConsumption+=(((Pbusy-Pidle)*(double)normalisedCPU)+Pidle/server[j].peakPower);
			j++;
			remCPU=server[j].ThreshCPU;remMEM=server[j].ThreshMEM;
		}
	}
	normalisedCPU=(((server[j].ThreshCPU)-remCPU)/(double)server[j].ThreshCPU);
	normalisedMEM=(((server[j].ThreshMEM)-remMEM)/(double)server[j].ThreshMEM);
	normalisedRemCPU=(remCPU/(double)server[j].ThreshCPU);
	normalisedRemMEM=(remMEM/(double)server[j].ThreshMEM);
	resourceWastage+=((abs(normalisedRemCPU-normalisedRemMEM)+E)/(double)(normalisedCPU+normalisedMEM));
	server[j].peakPower=((Pbusy-Pidle)*(double)normalisedCPU+Pidle);
	normalisedPowerConsumption+=(((Pbusy-Pidle)*(double)normalisedCPU)+Pidle/server[j].peakPower);

	S0.P=normalisedPowerConsumption;
	S0.W=resourceWastage;
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
	ll remCPU=s.ThreshMEM,remMEM=s.ThreshCPU;
	if(antSol.array[i]==-1 && (omega.find((ll)i)==omega.cend()))
	{
		for(register int k=0;k<n;k++)
		{
			if(antSol.array[k]==j)
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
			sum[2][i]=(sum[0][i]/server[i].ThreshCPU);
			//Used MEM to total MEM
			sum[3][i]=(sum[1][i]/server[i].ThreshMEM);
			//Remaining CPU to total MEM
			sum[4][i]=((server[i].ThreshCPU-sum[0][i])/server[i].ThreshCPU);
			//Remaining MEM to total MEM
			sum[5][i]=(server[i].ThreshMEM-sum[1][i])/server[i].ThreshMEM;
			//Calculating P
			sum[6][i]=((Pbusy-Pidle)*sum[2][i])+Pidle;
			if(sum[6][i]>server[i].peakPower)
			{
				server[i].peakPower=sum[6][i];
			}
			//Calculating normalised power of each server
			sum[7][i]=sum[6][i]/server[i].peakPower;
			summationP+=sum[7][i];
			//Calculating W
			sum[8][i]=(abs(sum[4][i]-sum[5][i])+E)/(sum[2][i]+sum[3][i]);
			summationW+=sum[8][i];
		}
	}
	t.desirability=/*for objective 1*/1/(E+summationP)+/*for objective 2*/1/(E+summationW);
	t.probability=(ALPHA*pheromoneTrail[nextVM][nextServer])+((1-ALPHA)*t.desirability);
}

double drawQ()
{
	return (rand()%101)/100;
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

int exploration(map<ll,qualifiedVM> omega)
{
	map<ll,qualifiedVM>:: iterator it;
	ll probabDistribution[2][MAX];
	for(register int i=0;i<MAX;i++)
	{
		for(register int j=0;j<2;j++)
		{
			probabDistribution[i][j]=0;
		}
	}
	int i=0;
	ll sum=0;
	probabDistribution[0][i]=(ll)(it->second.probability)*100;
	probabDistribution[1][i++]=(it->first);
	for(it=omega.begin();it!=omega.end();it++)
	{
		probabDistribution[0][i]=probabDistribution[0][i-1]+(ll)(it->second.probability)*100;
		sum=probabDistribution[0][i];
		probabDistribution[1][i++]=(it->first);
	}
	ll num=rand()%(sum+1);
	for(register int j=0;j<i;j++)
	{
		if(num<=probabDistribution[0][i])
		return (int)probabDistribution[1][i];
	}
}

void localPheromoneUpdate(double pheromoneTrail[][MAX], int i, int j)
{
	pheromoneTrail[i][j]=(1-RHO_LOCAL)*pheromoneTrail[i][j] + (RHO_LOCAL*T0);
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
				sum[2][i]=(sum[0][i]/server[i].ThreshCPU);
				//Used MEM to total MEM
				sum[3][i]=(sum[1][i]/server[i].ThreshMEM);
				//Remaining CPU to total MEM
				sum[4][i]=((server[i].ThreshCPU-sum[0][i])/server[i].ThreshCPU);
				//Remaining MEM to total MEM
				sum[5][i]=(server[i].ThreshMEM-sum[1][i])/server[i].ThreshMEM;
				//Calculating P
				sum[6][i]=((Pbusy-Pidle)*sum[2][i])+Pidle;
				//Calculating normalised power of each server
				sum[7][i]=sum[6][i]/server[i].peakPower;
				summationP+=sum[7][i];
				//Calculating W
				sum[8][i]=(abs(sum[4][i]-sum[5][i])+E)/(sum[2][i]+sum[3][i]);
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
				if(ParetoSet[i].P>CurrSol[j].P || ParetoSet[i].W>CurrSol[j].W)
				{
					if(ParetoSet[i].P>=CurrSol[j].P && ParetoSet[i].W>=CurrSol[j].W)
					{
						CurrSol[j].dominated=true;
					}
				}
			}
		}
	}
	for(register int i=0;i<CurrSol.size();i++)                   //All elements of current set are compared to all elements of paretoset
	{
		for(register int j=0;j<CurrSol.size();j++)
		{
			if(i!=j && !CurrSol[j].dominated)
			{
				if(CurrSol[i].P>CurrSol[j].P || CurrSol[i].W>CurrSol[j].W)
				{
					if(CurrSol[i].P>=CurrSol[j].P && CurrSol[i].W>=CurrSol[j].W)
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
					if(CurrSol[i].P>ParetoSet[j].P || CurrSol[i].W>ParetoSet[j].W)
					{
						if(CurrSol[i].P>=ParetoSet[j].P && CurrSol[i].W>=ParetoSet[j].W)
						{
							ParetoSet.erase(ParetoSet.begin()+j);       //Erased Dominated solution from paretoset
						}
					}
				}
				ParetoSet.push_back(CurrSol[i]);
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
	double lembda=NO_OF_ANTS/(t-s.NO_OF_IT+1);
	double B=((RHO_GLOBAL*lembda)/(s.P+s.W));
	for(register int i=0;i<MAX;i++)
	{
		pheromoneTrail[i][s.array[i]]=((1-RHO_GLOBAL)*pheromoneTrail[i][s.array[i]])+B;
	}
}

void printSolution(vector<solution> ParetoSet, int totalVM)
{
	for(register int i=0;i<ParetoSet.size();i++)
	{
		cout<<"\nSolution "<<i+1<<":"<<endl;
		for(register int j=0;j<totalVM;j++)
		{
			cout<<j<<" - "<<ParetoSet[i].array[j]<<endl;
		}
		cout<<"Power Wastage: "<<ParetoSet[i].P<<"\t"<<"Memory Wastage: "<<ParetoSet[i].W;
	}
}
