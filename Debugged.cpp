
/* ***************************(( Project ACO ))****************************** */

#include <iostream>
#include <vector>
#include<algorithm>
#include<cmath>
#include<map>
#include<climits>

#define ll long long
#define MAX 100
#define NO_OF_ANTS 2
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

/* Done */ /* Verified */ bool getServer(Server a, Server b);
/* Done */ /* Verified */ bool getVM(VM a, VM b);
/* Done */ /* Verified */ void generateGreedySolution(solution& S0, Server server[],ll m, VM vm[],ll n);
/* Done */ /* Verified */ void initializePheromone(double pheromoneTrail[][MAX],solution S0,Server b[],ll m, VM a[], ll n);
/* Done */ /* Verified */ void randomSort(Server server[], ll len);
/* Done */ /* Verified */ void randomSortVM(VM vm[], ll len);
/* Done */ /* Verified */ void setInitials(solution& a);
/* Done */ /* Verified */ bool compatibleVM(solution antSol,int i,VM vm[],ll n,int j,Server s,map<ll,qualifiedVM> omega);
/* Done */ /* Verified */ void prepare(qualifiedVM& t,int nextVM,solution antSol,VM vm[],ll n,int nextServer,Server server[],ll m,double pheromoneTrail[][MAX]);  //will use 1,2 to update t
/* Done */ /* Verified */ double drawQ();
/* Done */ /* Verified */ int exploitation(map<ll,qualifiedVM> omega);   //return maximum of probability
/* Done */ /* Verified */ int exploration(map<ll,qualifiedVM> omega);    //return random number according to probability
/* Done */ /* Verified */ void localPheromoneUpdate(double pheromoneTrail[][MAX], int i, int j);
/* Done */ /* Verified */ void updatePandW(vector<solution>& CurrSol,VM vm[],ll n,Server server[],ll m);
/* Done */ /* Verified */ void updateDomination(vector<solution>& ParetoSet,vector<solution>& CurrSol);
/* Done */ /* Verified */ void updateParetoIteration(vector<solution>& ParetoSet);
/* Done */ /* Verified */ void globalPheromoneUpdate(double pheromoneTrail[][MAX],solution s,ll t);
/* Done */ /* Verified */ void printSolution(vector<solution> ParetoSet,int totalVM);


void printSolution(solution s, ll n, ll m)
{
	for(int i=0;i<n;i++)
	{
		cout<<i<<"-"<<s.array[i]<<endl;
	}
	cout<<"Power: "<<s.P<<endl;
	cout<<"Memory: "<<s.W<<endl;
}

void viewVM(VM v[], ll n)
{
	for(int i=0;i<n;i++)
	{
		cout<<i<<" "<<v[i].CPU<<" "<<v[i].MEM<<endl;
	}
}

void viewServer(Server s[], ll m)
{
	for(int i=0;i<m;i++)
	{
		cout<<i<<" "<<s[i].ThreshCPU<<" "<<s[i].ThreshMEM<<endl;
	}
}

void seeSolution(vector<solution> CurrentSol, ll n)
{
	vector<solution>:: iterator it;
	for(it=CurrentSol.begin();it!=CurrentSol.end();it++)
	{
		cout<<"Solution: ";
		for(int i=0;i<n;i++)
		{
			cout<<i<<"-"<<it->array[i]<<endl;
		}
		cout<<"Power: "<<it->P<<" Memory: "<<it->W<<" Dominated: "<<it->dominated<<endl;
	}
}

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
	//printSolution(S0,n,m);
	randomSortVM(vm,n);
	//viewVM(vm,n);
	initializePheromone(pheromoneTrail,S0,server,m,vm,n);
	
	/* *********************** Iterative Loop ******************************* */
	ll it=1;
	while(it<=NO_OF_ITERATIONS)
	{
		vector<solution> CurrentSol;
		for(register int k=1;k<=NO_OF_ANTS;k++)
		{
			randomSort(server,m);
			//viewServer(server,m);
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
						//cout<<"for loop started\n";
						if(compatibleVM(antSol,i,vm,n,j,server[j],omega))
						{
							qualifiedVM t;      		    			 		 //Create set of these VM's
							prepare(t,i,antSol,vm,n,j,server,m,pheromoneTrail);      //getdesirability();getprobability();
							//cout<<t.desirability<<" "<<t.probability<<endl;
							omega[i]=t;
						}
					}
					
					//cout<<"outside for loop\n";
					
					if(omega.empty())						/*No remaining VM fits into server anymore OR Set is Empty */
					{
						//cout<<"checking Break!!\n";
						break;
					}
					double q=drawQ();						//Draw q random number between 0 to 1
					int i;
					if(q<=Q0)
					{
						//cout<<"Inside q<=Q0\n";
						i=exploitation(omega);					//Exploitaion - Generate i(VM) to place in this host
						antSol.array[i]=j;
					}
					else
					{
						//cout<<"Inside else\n";
						i=exploration(omega);					//Exploration Get any number according to the probability distribution function formula
						antSol.array[i]=j;
					}
					VMcount++;                              //Incrementing, since another VM got placed
					localPheromoneUpdate(pheromoneTrail,i,j);					//Apply Local updating Rule
				}
				if(VMcount==n)		/*If all VM's are placed */       //break of server choosing
				{
					//cout<<"VM count reach n\n";
					//flag=true;
					break;			//a solution is created
				}
			}
			CurrentSol.push_back(antSol);
			//cout<<"Solution is added\n";
		}
		
		//seeSolution(CurrentSol,n);

		/*
		Evaluate the sets and update it accordingly
			1.Calculate values of P(S) and W(S) for each solution S
			2.Update the Pareto set according to dominations
		*/
		
		updatePandW(CurrentSol,vm,n,server,m);
		//seeSolution(CurrentSol,n);
		updateDomination(ParetoSet,CurrentSol);
		//cout<<"Updated solution\n";
		//seeSolution(ParetoSet,n);
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
	return ((((REL_PARAMETER)*(double)a.ThreshCPU)+((1-REL_PARAMETER)*(double)a.ThreshMEM))>(((REL_PARAMETER)*(double)b.ThreshCPU)+((1-REL_PARAMETER)*(double)b.ThreshMEM)));
}

bool getVM(VM a, VM b)							//compare function for randomSortVM function
{
	return ((((REL_PARAMETER)*(double)a.CPU)+((1-REL_PARAMETER)*(double)a.MEM))<(((REL_PARAMETER)*(double)b.CPU)+((1-REL_PARAMETER)*(double)b.MEM)));
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
		//cout<<remCPU<<" "<<remMEM<<endl;
		if(remCPU>=vm[i].CPU && remMEM>=vm[i].MEM)
		{
			S0.array[i]=j;
			remCPU-=(vm[i].CPU);remMEM-=(vm[i].MEM);
		}
		else{
			normalisedCPU=(((server[j].ThreshCPU)-remCPU)/(double)server[j].ThreshCPU);
			//cout<<"normalesdCPU "<<normalisedCPU<<endl;
			normalisedMEM=(((server[j].ThreshMEM)-remMEM)/(double)server[j].ThreshMEM);
			//cout<<"normalesdMEM "<<normalisedMEM<<endl;
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
	//cout<<"normalesdCPU "<<normalisedCPU<<endl;
	normalisedMEM=(((server[j].ThreshMEM)-remMEM)/(double)server[j].ThreshMEM);
	//cout<<"normalesdMEM "<<normalisedMEM<<endl;
	normalisedRemCPU=(remCPU/(double)server[j].ThreshCPU);
	normalisedRemMEM=(remMEM/(double)server[j].ThreshMEM);
	resourceWastage+=((abs(normalisedRemCPU-normalisedRemMEM)+E)/(double)(normalisedCPU+normalisedMEM));
	server[j].peakPower=((Pbusy-Pidle)*(double)normalisedCPU+Pidle);
	normalisedPowerConsumption+=(((Pbusy-Pidle)*(double)normalisedCPU)+Pidle/server[j].peakPower);
	
	S0.P=normalisedPowerConsumption;
	S0.W=resourceWastage;
	//cout<<"Greedy Solution Generated\n";
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
	//cout<<"Pheromone Initialized: "<<t0<<endl;
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
	//cout<<"Random Sorting Done\n";
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
	//cout<<"Random sorting of VM Done\n";
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
	//cout<<"here Compatible function started\n";
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
			//cout<<"Compatible function finished true returned\n";
			return true;
		}
		else{ /* cout<<"Compatible function finished false returned\n"; */ return false; };
	}
	//cout<<"Compatible function finished false returned\n";
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
	//cout<<"Qualified VM Prepared\n";
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
	//cout<<"Exploitation Done- index= "<<index<<endl;;
	return index;
}

int exploration(map<ll,qualifiedVM> omega)                  //This is not working properly
{
	//cout<<"exploration is called"<<endl;
	double probabDistribution[2][MAX];
	for(int i=0;i<2;i++)
	{
		for(int j=0;j<MAX;j++)
		{
			probabDistribution[i][j]=0;
		}
	}
	//cout<<"Loop initialization completed\n";
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
			//cout<<"Exploration Done index= "<<(int)probabDistribution[1][j]<<endl;
			return (int)probabDistribution[1][j];
		}
	}
}

void localPheromoneUpdate(double pheromoneTrail[][MAX], int i, int j)
{
	pheromoneTrail[i][j]=(double)(1-RHO_LOCAL)*pheromoneTrail[i][j] + (RHO_LOCAL*T0);
	//cout<<"Local Pheromone update done\n";
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
	//cout<<"P and W updated\n";
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
	for(register int i=0;i<CurrSol.size();i++)                   //All elements of current set are compared to all elements of currentset
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
	//cout<<"Domination Updated\n";
}

void updateParetoIteration(vector<solution>& ParetoSet)
{
	for(register int i=0;i<ParetoSet.size();i++)
	{
		ParetoSet[i].NO_OF_IT++;
	}
	//cout<<"PartoSet soulution's Iterator updated\n";
}

void globalPheromoneUpdate(double pheromoneTrail[][MAX],solution s,ll t)
{
	double lembda=(double)NO_OF_ANTS/((double)(t-s.NO_OF_IT+1));
	double B=((RHO_GLOBAL*lembda)/((double)(s.P+s.W)));
	for(register int i=0;i<MAX;i++)
	{
		pheromoneTrail[i][s.array[i]]=(double)((1-RHO_GLOBAL)*pheromoneTrail[i][s.array[i]])+B;
	}
	//cout<<"Global Pheromone Updated\n";
}

void printSolution(vector<solution> ParetoSet, int totalVM)
{
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
