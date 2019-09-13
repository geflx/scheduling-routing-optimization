#include <ilcplex/ilocplex.h>
#include <iostream>
#include <fstream>
#define DEBUG_MODE
#include <bits/stdc++.h>
using namespace std;

#define INF 987654321
/*
	Modelo proposto no artigo: "Mathematical programming and solution approaches for minimizing tardiness and transportation costs in the supply chain scheduling problem." (pags. 644 e 645), versao 2019.
	Codigo por: Gabriel de Paula Felix
	Data: Agosto/2019
	Professor Orientador: Jose Claudio Elias Arroyo
	Universidade Federal de Vicosa, Vicosa-MG, Brasil
*/

typedef IloArray<IloIntArray> IntMatriz;
typedef IloArray<IloNumArray> FloatMatriz;
typedef IloArray<IloArray<IloNumVarArray>> Matriz3D;
typedef IloArray<IloNumVarArray> NumVarMatrix;
typedef IloArray<NumVarMatrix> NumVar3Matrix;


int main(){
	IloEnv ambiente;
	IloModel modeloP(ambiente);
	IloExpr funcaoO(ambiente);
	IloRangeArray Restricoes(ambiente);

	IloInt numTrabalhos;
	IloInt numVeiculos;
	//1- Tratamento de entrada de dados + Inicializacao das matrizes

	//1.1- Leitura dos dados (trabalhando com arquivos)
	string fileName;
	float varMi,sigmaUm,sigmaDois;
	int numInstancia,constanteVeiculo;
	cin>>fileName;
	ifstream in(fileName);
	in>>numInstancia>>varMi>>sigmaUm>>sigmaDois>>constanteVeiculo;
	in>>numTrabalhos;
	in>>numVeiculos;

	IloNumArray F(ambiente,numVeiculos); //custo do veiculo
	IloNumArray Q(ambiente,numVeiculos); //capacidade do veiculo

	//0- Declaracao de variaveis e vetores constantes
	FloatMatriz C(ambiente,numTrabalhos+1); //custo
	IntMatriz t(ambiente,numTrabalhos+1); //tempo da viagem
	IloNumArray P(ambiente,numTrabalhos+1); //tempo de processamento
	IloNumArray w(ambiente,numTrabalhos+1); //peso da penalidade
	IloNumArray d(ambiente,numTrabalhos+1); //vencimento
	IloNumArray s(ambiente,numTrabalhos+1); //tamanho da tarefa j E N

	P[0]=0; //Tarefa fantasma nao possui tempo de processamento.
	d[0]=0;
	w[0]=0;
	s[0]=0;
	for(int i=1;i<=numTrabalhos;i++)
		in>>P[i];
	for(int i=1;i<=numTrabalhos;i++)
		in>>d[i];
	for(int i=1;i<=numTrabalhos;i++)
		in>>s[i];

	for(int i=0;i<numVeiculos;i++)
		in>>Q[i];
	for(int i=0;i<numVeiculos;i++)
		in>>F[i];
	for(int i=1;i<=numTrabalhos;i++)
		in>>w[i];

    for(int i=0;i<=numTrabalhos;i++)
		t[i]=IloIntArray(ambiente,numTrabalhos+1);
	for(int i=0;i<=numTrabalhos;i++){
		for(int j=0;j<=numTrabalhos;j++){
			in>>t[i][j];
        }
    }

	in.close();

	#ifdef DEBUG_MODE
	cout<<"Processing time:\n";
	for(int i=1;i<=numTrabalhos;i++)
		cout<<P[i]<<" ";
    cout<<endl;

	cout<<"Due date:\n";
	for(int i=1;i<=numTrabalhos;i++)
		cout<<d[i]<<" ";
    cout<<endl;

	cout<<"Job sizes:\n";
	for(int i=1;i<=numTrabalhos;i++)
		cout<<s[i]<<" ";
    cout<<endl;


	cout<<"Vehicle capacity:\n";
	for(int i=0;i<numVeiculos;i++)
		cout<<F[i]<<" ";
    cout<<endl;

	cout<<"Penalty weight:\n";
	for(int i=1;i<=numTrabalhos;i++)
		cout<<w[i]<<" ";
    cout<<endl;

	cout<<"Time[i][j]:\n";
	for(int i=0;i<=numTrabalhos;i++){
		for(int j=0;j<=numTrabalhos;j++){
			cout<<t[i][j]<<" ";
        }
        cout<<endl;
    }
    #endif
	//2- Declaracao de matrizes, vetores e variaveis de decisao
	NumVarMatrix A(ambiente,numTrabalhos+1);
	for(int i=0;i<=numTrabalhos;i++){
		A[i]=IloNumVarArray(ambiente,numTrabalhos+1,0,1,ILOINT);
        for(int j=0;j<=numTrabalhos;j++){
            char name[50]="";
            sprintf(name,"A[%d][%d]",i,j);
            A[i][j].setName( (char*)name);
        }
    }

	IloNumVarArray Y(ambiente,numVeiculos,0,1,ILOINT);
	for(int i=0;i<numVeiculos;i++){
        char name[50]="";
        sprintf(name,"Y[%d]",i);
        Y[i].setName(name);
	}
	IloNumVarArray S(ambiente,numVeiculos,0,IloInfinity,ILOINT);
	for(int i=0;i<numVeiculos;i++){
        char name[50]="";
        sprintf(name,"S[%d]",i);
        S[i].setName(name);
	}
	IloNumVarArray T(ambiente,numTrabalhos+1,0,IloInfinity,ILOINT);
	for(int i=0;i<=numTrabalhos;i++){
        char name[50]="";
        sprintf(name,"T[%d]",i);
        T[i].setName(name);
	}
	IloNumVarArray D(ambiente,numTrabalhos+1,0,IloInfinity,ILOINT);
		for(int i=0;i<=numTrabalhos;i++){
        char name[50]="";
        sprintf(name,"D[%d]",i);
        D[i].setName(name);
	}
	IloNumVarArray completionTime(ambiente,numTrabalhos+1,0,IloInfinity,ILOINT); //completion time
		for(int i=0;i<=numTrabalhos;i++){
        char name[50]="";
        sprintf(name,"Cj[%d]",i);
        completionTime[i].setName(name);
	}


	// // // Matriz 3D de decisao do trajeto
	  Matriz3D X= Matriz3D(ambiente,numTrabalhos+1);
	  for(int i=0;i<=numTrabalhos;i++){
	  	X[i]=NumVarMatrix(ambiente,numTrabalhos+1);
	  	for(int j=0;j<=numTrabalhos;j++){
	  		X[i][j]=IloNumVarArray(ambiente,numVeiculos,0,1,ILOINT);
	  		for(int k=0;k<numVeiculos;k++){
                char name[50]="";
                sprintf(name,"X[%d][%d][%d]",i,j,k);
                X[i][j][k].setName(name);
	  		}
	  	}
	  }


	// 3- Declaracao da funcao objetivo --Revisado
	for(IloInt i=0;i<=numTrabalhos;i++)
		for(IloInt j=0;j<=numTrabalhos;j++)
			for(IloInt k=0;k<numVeiculos;k++)
				funcaoO+=t[i][j]*X[i][j][k]; //Cij e' tij no momento. Considerado no artigo.

	for(IloInt k=0;k<numVeiculos;k++)
		funcaoO+=F[k]*Y[k];
	for(IloInt i=1;i<=numTrabalhos;i++)
		funcaoO+=w[i]*T[i];
	modeloP.add(IloMinimize(ambiente,funcaoO));

	// //3-Declaracao de restricoes -- Revisado
	// // (2) Garantir que cada cliente e' visitado somente uma vez.
	for(IloInt i=1;i<=numTrabalhos;i++){
		IloExpr sum(ambiente);
		for(IloInt k=0;k<numVeiculos;k++){
			for(IloInt j=0;j<=numTrabalhos;j++){
				if(j==i) continue; //j!=i
				sum+=X[i][j][k];
			}
		}
		IloConstraint c(sum==1);
        c.setName("ClienteVisitado_UmaVez");
        modeloP.add(c);
		sum.end();
	}

	// //(3) Veiculo em USO com Q trabalhos pode se mover por ate Q+1 caminhos --Revisado
	for(IloInt k=0;k<numVeiculos;k++){
		IloExpr sum2(ambiente);
		for(IloInt i=0;i<=numTrabalhos;i++){
			for(IloInt j=0;j<=numTrabalhos;j++){
				if(i==j) continue; //i!=j
				sum2+=s[i]*X[i][j][k];
			}
		}
		IloConstraint c(((Q[k])*Y[k])-sum2>=0);
        c.setName("Veiculo_caminhos");
        modeloP.add(c);
		sum2.end();
	}

	// //(4) Garantir que se um veiculo esta em uso, ele sempre deixa a origem e vice-versa -- Revisado
	for(IloInt k=0;k<numVeiculos;k++){
		IloExpr sum3(ambiente);
		for(IloInt j=0;j<=numTrabalhos;j++){
			sum3+=X[0][j][k];
		}
		IloConstraint c2(Y[k]-sum3==0);
        c2.setName("Veiculo_deixa_a_origem_e_volta");
        modeloP.add(c2);
		sum3.end();
	}

	// //(5) Se um veiculo chega em um cliente, ele deve sair desse cliente --Revisado

	for(IloInt k=0;k<numVeiculos;k++){
		for(IloInt h=0;h<=numTrabalhos;h++){
			IloExpr sum4(ambiente);
			IloExpr sum5(ambiente);
			for(IloInt i=0;i<=numTrabalhos;i++){
				sum4+=X[i][h][k];
			}
			for(IloInt j=0;j<=numTrabalhos;j++){
				sum5+=X[h][j][k];
			}
			IloConstraint c3(sum5-sum4==0);
            c3.setName("Veiculo_chegaESai_em_cliente");
            modeloP.add(c3);
			sum4.end();
			sum5.end();
		}
	}

	// //(6) e (7) restricoes para evitar sequencias impossiveis de trabalho.
	// /*which is required to compute the completion time of each job. The completion time of a job is the processing
	// time of that job, plus sum of the processing times of the jobs produced
	// before that job*/
	//(6) -- Revisado
	for(IloInt i=0;i<=numTrabalhos;i++){
		for(IloInt j=0;j<=numTrabalhos;j++){
			if(i==j) continue;
			IloExpr igual(ambiente);
			igual=A[i][j]+A[j][i];
			IloConstraint c4(igual==1);
            c4.setName("Sequencia_de_tarefas");
            modeloP.add(c4);
			igual.end();
		}
	}
	// //(7) --Revisado
	for(IloInt i=0;i<=numTrabalhos;i++){
		for(IloInt j=0;j<=numTrabalhos;j++){
			for(IloInt r=0;r<=numTrabalhos;r++){
				if((i==j)||(j==r)||(i==r)) continue; //CONFIRMAR!
				IloExpr maiorIgual(ambiente);
				maiorIgual=A[i][j]+A[j][r]+A[r][i];
				IloConstraint c5(maiorIgual>=1);
                c5.setName("Sequencia_de_tarefas2");
                modeloP.add(c5);
				maiorIgual.end();
			}
		}
	}

 //    //(7.5)Calculating Big M
    int M=0;
    int maiordist=-1;
    for(int i=0;i<=numTrabalhos;i++){
    	M+=P[i];
    	maiordist=maiordist>t[0][i]?maiordist:t[0][i];
    }
    #ifdef DEBUG_MODE
    	cout<<"A maior distancia entre a origem e um ponto é "<<maiordist<<endl;
    #endif
    M+= 2 * maiordist;

    //(8)Se um trabalho j e carregado por um veiculo k, entao o veiculo k iniciou apos trabalho j finalizar --Revisado
	for(IloInt k=0;k<numVeiculos;k++){
		IloExpr sum7(ambiente);
		for(IloInt j=1;j<=numTrabalhos;j++){
			for(IloInt i=1;i<=numTrabalhos;i++){
				if(i==j) continue;
				sum7+=X[i][j][k];
			}
			IloExpr agrupado(ambiente);
			agrupado= completionTime[j]-M*(1-sum7);
            IloConstraint c5(S[k]-agrupado>=0);
            c5.setName("SkMaior_maiorQue_completionTime");
            modeloP.add(c5);
			agrupado.end();
		}
	}

	//(9) e (10) sao restricoes em relacao ao tempo de entrega dos trabalhos aos clientes.

	//(9)O tempo de um veiculo k chegar no primeiro cliente e' maior que (tempo inicio veiculo + trajeto) -- Revisado
	for(IloInt k=0;k<numVeiculos;k++){
		for(IloInt j=1;j<=numTrabalhos;j++){
			IloExpr maiorIgual2(ambiente);
			maiorIgual2= t[0][j]- M*(1-X[0][j][k]);
            IloConstraint c5((D[j]-S[k])-maiorIgual2>=0);
            c5.setName("tempoSk_maiorQue_SkMaisTrajeto");
            modeloP.add(c5);
			maiorIgual2.end();
		}
	}

	/*(10)Se um trabalho nao e' o primeiro a ser entregue num tour, seu tempo de entrega e'
	maior que (tempo entrega trabalho anterior + trajeto) -- Revisado */
	for(IloInt i=1;i<=numTrabalhos;i++){
		for(IloInt j=1;j<=numTrabalhos;j++){
			IloExpr sum8(ambiente);
			for(IloInt k=0;k<numVeiculos;k++)
				sum8+=X[i][j][k];
			IloExpr maiorIgual3=t[i][j]-M*(1-sum8);
            IloConstraint c5((D[j]-D[i])-maiorIgual3>=0);
            c5.setName("Tempo_entrega_trabalhos");
            modeloP.add(c5);
			maiorIgual3.end();
		}
	}
	//(11) Se uma entrega excede o prazo, o atraso equivale a: (Tempo de entega - vencimento) --Revisado
	for(IloInt i=1;i<=numTrabalhos;i++){
			IloExpr maiorIgual4=D[i]-d[i];
			IloConstraint c5(T[i]-maiorIgual4>=0);
            c5.setName("Atraso_e_Di_menos_di");
            modeloP.add(c5);
			maiorIgual4.end();
	}
	//(12) Variavel completion time (Cj):
	//Declaracao da nova variavel Cj, completion time
	for(int j=0;j<=numTrabalhos;j++){
		IloExpr igual(ambiente);
		for(int i=0;i<=numTrabalhos;i++){
			if(i==j) continue;
			igual+=P[i]*A[i][j];
		}
		igual+=P[j];
        IloConstraint c5(completionTime[j]-igual==0);
        c5.setName("Completion_time");
        modeloP.add(c5);
	}

	// (13) Completion time da tarefa 0:
	IloConstraint c5(completionTime[0]==0);
    c5.setName("Completion_time_tarefa_fantasma");
    modeloP.add(c5);

	//(14) (15) (16) >=0
	for(int k=0;k<numVeiculos;k++){
		IloConstraint c6(S[k]>=0);
        c6.setName("Sk_maiorIgual_0");
        modeloP.add(c6);
	}
	for(int i=1;i<=numTrabalhos;i++){
		IloConstraint c6(D[i]>=0);
        c6.setName("Di_maiorIgual_0");
        modeloP.add(c6);
	}
	for(int i=1;i<=numTrabalhos;i++){
		IloConstraint c6(T[i]>=0);
        c6.setName("Ti_maiorIgual_0");
        modeloP.add(c6);
	}
	//Tarefa fantasma == 0
    IloConstraint c6(D[0]==0);
    c6.setName("d0_igual_zero");
    modeloP.add(c6);

    IloConstraint c7(T[0]==0);
    c7.setName("t0_igual_zero");
    modeloP.add(c7);

	modeloP.add(Restricoes);

	//Execução do Modelo
	IloCplex cplex1(modeloP);
	cplex1.exportModel("modelo.lp");
	if(!cplex1.solve()){
		//codigo aqui
		cout<<"Sem solucao encontrada"<<endl;
	}else{
        cout<<endl;
		cout<<"A solucao adquirida foi: "<<cplex1.getObjValue()<<endl;
		cout<<endl<<endl;
        cout<<"Os valores de T sao:";
        for(int i=0;i<=numTrabalhos;i++){
            cout<<cplex1.getValue(T[i])<<" ";
        }cout<<endl;

        cout<<"Os valores de D sao:";
         for(int i=0;i<=numTrabalhos;i++){
            cout<<cplex1.getValue(D[i])<<" ";
        }cout<<endl;

        cout<<"Os valores de S sao:";
        for(int i=0;i<numVeiculos;i++){
            cout<<cplex1.getValue(S[i])<<" ";
        }cout<<endl;
	}
	ambiente.end();

	return 0;
}
