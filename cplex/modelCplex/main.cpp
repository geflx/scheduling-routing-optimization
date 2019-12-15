#include <ilcplex/ilocplex.h>
#include <iostream>
#include <fstream>
#include <algorithm>
#include <vector>
#include <sstream>
#include <string>
//#define DEBUG_MODE
//#include <bits/stdc++.h>
using namespace std;

#define INF 987654321


typedef IloArray<IloIntArray> IntMatriz;
typedef IloArray<IloNumArray> FloatMatriz;
typedef IloArray<IloArray<IloNumVarArray> > Matriz3D;
typedef IloArray<IloNumVarArray> NumVarMatrix;
typedef IloArray<NumVarMatrix> NumVar3Matrix;


int main(){
    float VarValue;
    string fileName;
    char name[10];
    //cin>>fileName;
    //ifstream in(fileName);
    ifstream in ("8jobs.txt"); //abre arquivo
    ofstream myfile;

    myfile.open("Resultados_8jobs.txt");

    float varMi,delta;
    int numInstancia; //,constanteVeiculo;

    myfile<<setw(15)<<"Instance";
    myfile<<setw(12)<<"Obj_Value";
    myfile<<setw(10)<<"Obj_1";
    myfile<<setw(10)<<"Obj_2";
    myfile<<setw(11)<<"OBj_3";
    myfile<<setw(14)<<"RelativeGap%";
    myfile<<setw(12)<<"Status";
    myfile<<setw(12)<<"CPUtime"<<"\t";
    myfile<<"Processing_Order"<<"    ";
    myfile<<"Vehicle_Routes"<<endl;

    while(in>>numInstancia){

        IloEnv ambiente;
        IloModel modeloP(ambiente);
        IloExpr funcaoO(ambiente);
        IloRangeArray Restricoes(ambiente);

        IloInt numTrabalhos;
        IloInt numVeiculos;
        //1- Tratamento de entrada de dados + Inicializacao das matrizes

        //1.1- Leitura dos dados (trabalhando com arquivos)


        //ifstream in ("teste0.txt"); //abre arquivo
        in>>varMi>>delta;
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
        for(int i=1;i<=numTrabalhos;i++){
            in>>P[i];
        }
        for(int i=1;i<=numTrabalhos;i++){
            in>>d[i];
        }

        for(int i=1;i<=numTrabalhos;i++){
            in>>s[i];
        }


        for(int i=1;i<=numTrabalhos;i++){
            in>>w[i];
        }
        for(int i=0;i<numVeiculos;i++){
            in>>Q[i];
        }
        for(int i=0;i<numVeiculos;i++){
            in>>F[i];
        }


        for(int i=0;i<=numTrabalhos;i++)
            t[i]=IloIntArray(ambiente,numTrabalhos+1);
        for(int i=0;i<=numTrabalhos;i++){
            for(int j=0;j<=numTrabalhos;j++){
                in>>t[i][j];
            }
        }


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
            cout<<Q[i]<<" ";
        cout<<endl;

        cout<<"Vehicle fix cost:\n";
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

            for(int j=0;j<numTrabalhos+1;j++) {
                sprintf(name, "A[%d][%d]", i,j);
                A[i][j].setName(name);
            }

        }


        IloNumVarArray Y(ambiente,numVeiculos,0,1,ILOINT);
        for(int i=0;i<numVeiculos;i++) {
            sprintf(name, "Y[%d]", i);
            Y[i].setName(name);
        }


        IloNumVarArray S(ambiente,numVeiculos,0,IloInfinity,ILOINT);
        for(int i=0;i<numVeiculos;i++) {
            sprintf(name, "S[%d]", i);
            S[i].setName(name);
        }


        IloNumVarArray T(ambiente,numTrabalhos+1,0,IloInfinity,ILOINT);
        for(int i=0;i<numTrabalhos+1;i++) {
            sprintf(name, "T[%d]", i);
            T[i].setName(name);
        }

        IloNumVarArray D(ambiente,numTrabalhos+1,0,IloInfinity,ILOINT);
        for(int i=0;i<numTrabalhos+1;i++) {
            sprintf(name, "D[%d]", i);
            D[i].setName(name);
        }


        IloNumVarArray Ct(ambiente,numTrabalhos+1,0,IloInfinity,ILOINT); //completion time
        for(int i=0;i<numTrabalhos+1;i++) {
            sprintf(name, "Ct[%d]", i);
            Ct[i].setName(name);
        }


        // // // Matriz 3D de decisao do trajeto
        NumVar3Matrix X(ambiente, numTrabalhos+1); //se a tarefa j é processada no batch b na maquina k (Bbk)

        for(int i=0;i<=numTrabalhos;i++){
            X[i]=NumVarMatrix(ambiente,numTrabalhos+1);
            for(int j=0;j<=numTrabalhos;j++){
                X[i][j]=IloNumVarArray(ambiente,numVeiculos,0,1,ILOINT);
                for(int k=0;k<numVeiculos;k++){
                    sprintf(name, "x[%d][%d][%d]", i, j, k);
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

            Restricoes.add(sum==1);

            //IloConstraint c(sum==1);
            //c.setName("ClienteVisitado_UmaVez");
            //modeloP.add(c);
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

            Restricoes.add((Q[k]*Y[k])-sum2>=0);

            //IloConstraint c((Q[k]*Y[k])-sum2>=0);
            //c.setName("Veiculo_caminhos");
            //modeloP.add(c);
            sum2.end();
        }

        // //(4) Garantir que se um veiculo esta em uso, ele sempre deixa a origem e vice-versa -- Revisado
        for(IloInt k=0;k<numVeiculos;k++){
            IloExpr sum3(ambiente);
            for(IloInt j=1;j<=numTrabalhos;j++){
                sum3+=X[0][j][k];
            }

            Restricoes.add(Y[k]-sum3 == 0);

            //IloConstraint c2(Y[k]-sum3 == 0);
            //c2.setName("Veiculo_deixa_a_origem_e_volta");
            //modeloP.add(c2);
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


                Restricoes.add(sum5-sum4==0);

                //IloConstraint c3(sum5-sum4==0);
                //c3.setName("Veiculo_chegaESai_em_cliente");
                //modeloP.add(c3);
                sum4.end();
                sum5.end();
            }
        }

        // //(6) e (7) restricoes para evitar sequencias impossiveis de trabalho.
        // which is required to compute the completion time of each job. The completion time of a job is the processing
        // time of that job, plus sum of the processing times of the jobs produced
        // before that job
        //(6) -- Revisado
        for(IloInt i=0;i<=numTrabalhos;i++){
            for(IloInt j=0;j<=numTrabalhos;j++){
                if(i==j) continue;
                //IloExpr igual(ambiente);
                //igual=A[i][j]+A[j][i];

                Restricoes.add(A[i][j]+A[j][i]==1);

                //IloConstraint c4(A[i][j]+A[j][i]==1);
                //c4.setName("Sequencia_de_tarefas");
                //modeloP.add(c4);
                //igual.end();
            }
        }
        // //(7) --Revisado
        for(IloInt i=0;i<=numTrabalhos;i++){
            for(IloInt j=0;j<=numTrabalhos;j++){
                for(IloInt r=0;r<=numTrabalhos;r++){
                    if((i==j)||(j==r)||(i==r)) continue; //CONFIRMAR!

                    Restricoes.add(A[i][j]+A[j][r]+A[r][i]>=1);

                    //IloExpr maiorIgual(ambiente);
                    //maiorIgual=A[i][j]+A[j][r]+A[r][i];
                    //IloConstraint c5(maiorIgual>=1);
                    //c5.setName("Sequencia_de_tarefas2");
                    //modeloP.add(c5);
                    //maiorIgual.end();

                }
            }
        }

        //    //(7.5)Calculating Big M
        int M=0;
        int maiordist = -1;
        for(int i=0;i<=numTrabalhos;i++){
            M+=P[i];
            maiordist = maiordist > t[0][i] ? maiordist : int(t[0][i]);
        }
        M = M + int(numTrabalhos*maiordist);

        //#ifdef DEBUG_MODE
        //cout<<"A maior distancia entre a origem e um ponto é "<<maiordist<<endl;
        //cout<<"Big M é "<<M<<endl;
        //#endif

        M += numTrabalhos * maiordist;

        //(8)Se um trabalho j e carregado por um veiculo k, entao o veiculo k iniciou apos trabalho j finalizar --Revisado
        for(IloInt k=0;k<numVeiculos;k++){
            for(IloInt j=1;j<=numTrabalhos;j++){
                IloExpr sum7(ambiente);
                for(IloInt i=0;i<=numTrabalhos;i++){
                    if(i==j) continue;
                    sum7+=X[i][j][k];
                }

                Restricoes.add(S[k] - (Ct[j]-M*(1-sum7))>=0);

                //IloExpr agrupado(ambiente);
                //agrupado= Ct[j]-M*(1-sum7);
                //IloConstraint c5(S[k]-agrupado>=0);
                //c5.setName("SkMaior_maiorQue_completionTime");
                //modeloP.add(c5);
                //agrupado.end();
                sum7.end();
            }

        }

        //(9) e (10) sao restricoes em relacao ao tempo de entrega dos trabalhos aos clientes.

        //(9)O tempo de um veiculo k chegar no primeiro cliente e' maior que (tempo inicio veiculo + trajeto) -- Revisado
        for(IloInt k=0;k<numVeiculos;k++){
            for(IloInt j=1;j<=numTrabalhos;j++){

                Restricoes.add((D[j]-S[k])-(t[0][j]- M*(1-X[0][j][k])) >=0 );

                //IloExpr maiorIgual2(ambiente);
                //maiorIgual2= t[0][j]- M*(1-X[0][j][k]);
                //IloConstraint c5((D[j]-S[k])-maiorIgual2>=0);
                //c5.setName("tempoSk_maiorQue_SkMaisTrajeto");
                //modeloP.add(c5);
                //maiorIgual2.end();
            }
        }

        //(10)Se um trabalho nao e' o primeiro a ser entregue num tour, seu tempo de entrega e'
        //maior que (tempo entrega trabalho anterior + trajeto) -- Revisado
        for(IloInt i=1;i<=numTrabalhos;i++){
            for(IloInt j=1;j<=numTrabalhos;j++){
                IloExpr sum8(ambiente);
                for(IloInt k=0;k<numVeiculos;k++)
                    sum8+=X[i][j][k];
                IloExpr maiorIgual3=t[i][j]-M*(1-sum8);

                Restricoes.add((D[j]-D[i])-maiorIgual3>=0);

                //IloConstraint c5((D[j]-D[i])-maiorIgual3>=0);
                //c5.setName("Tempo_entrega_trabalhos");
                //modeloP.add(c5);
                maiorIgual3.end();
                sum8.end();
            }

        }
        //(11) Se uma entrega excede o prazo, o atraso equivale a: (Tempo de entega - vencimento) --Revisado
        for(IloInt i=1;i<=numTrabalhos;i++){
            Restricoes.add(T[i]-D[i]+d[i]>=0);
            //IloExpr maiorIgual4=D[i]-d[i];
            //IloConstraint c5(T[i]-maiorIgual4>=0);
            //c5.setName("Atraso_e_Di_menor_di");
            //modeloP.add(c5);
            //maiorIgual4.end();
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


            Restricoes.add(Ct[j]-igual==0);
            //IloConstraint c5(Ct[j]-igual==0);
            //c5.setName("Completion_time");
            //modeloP.add(c5);
            igual.end();
        }

        // (13) Completion time da tarefa 0:
        //IloConstraint c5(Ct[0]==0);
        //c5.setName("Completion_time_tarefa_fantasma");
        //modeloP.add(c5);
        Restricoes.add(Ct[0]==0);



        //Tarefa fantasma == 0
        Restricoes.add(D[0]==0);
        Restricoes.add(T[0]==0);




        modeloP.add(Restricoes);


        //Execução do Modelo
        IloCplex cplex1(modeloP);

        cplex1.setParam(IloCplex::ClockType, 1);//cputime 1 to specify CPU time or 2 to specify wall-clock time
        cplex1.getMax(IloCplex::Threads);//Pega o numero maximo de threads disponiveis.
        //cplex.setParam(IloCplex::Threads, 8);
        //cplex.setParam(IloCplex::IntSolLim, 2); //O Cplex finaliza sua execucao quando encontrar a 2™ melhor solucao inteira.
        //cplex.setParam(IloCplex::RINSHeur, 1);// Habilita a heurÌstica RINS para melhora da performance do CPLEX.

        //cplex1.exportModel("modelo.lp");
        cplex1.setParam(IloCplex::TiLim, 3600);
        double tempo = 0.0;
        clock_t start, finish;//para calcular tempo de CPU
        start = clock();//tempo inicial


        char instance[50]="";
        sprintf(instance,"%d_%d_%d_%.1lf_%.1lf",numInstancia,numTrabalhos,numVeiculos,varMi,delta);

        cout<<"\n\nArquivo: "<<instance<<endl;

        if(!cplex1.solve()){
            //codigo aqui
            finish = clock();//
            tempo = (double)(finish - start) / CLOCKS_PER_SEC;
            char instance[50]="";
            sprintf(instance,"%d_%d_%d_%.1lf_%.1lf",numInstancia,numTrabalhos,numVeiculos,varMi,delta);
            myfile<<setw(15)<<instance;
            myfile<<setw(12)<<0.0;
            myfile<<setw(15)<<0.0;
            myfile<<setw(12)<<"Infeasible";
            myfile<<setw(12)<<tempo<<"\t";
        }else{

            finish = clock();//
            tempo = (double)(finish - start) / CLOCKS_PER_SEC;
            cout<<"F. objetivo: "<<cplex1.getObjValue()<<endl;

            //cout<<"1o Termo: ";
            long long int firstTerm=0;
            for(int i=0;i<=numTrabalhos;i++){
                for(int j=0;j<=numTrabalhos;j++){
                    if(i==j) continue;
                    for(int k=0;k<numVeiculos;k++){
                        firstTerm+=(cplex1.getValue(X[i][j][k])*t[i][j]);
                    }
                }
            }
            //cout<<firstTerm<<endl;

            //cout<<"2o Termo: ";
            long long int secondTerm=0;
            for(int k=0;k<numVeiculos;k++){
                secondTerm += F[k] * cplex1.getValue(Y[k]);
            }

            //cout<<secondTerm<<endl;

            //cout<<"3o Termo: ";
            double thirdTerm = 0.0;
            for(int i=1;i<=numTrabalhos;i++){
                thirdTerm += w[i] * cplex1.getValue(T[i]);
            }
            //cout<<thirdTerm<<"\n\n\n";
            //            for(IloInt i = 1 ; i <= numTrabalhos; i++){
            //                cout<<"C["<<i<<"] = "<<cplex1.getValue(Ct[i])<<endl;
            //            }
            //
            /*
             for(IloInt k=0;k<numVeiculos;k++){
             cout<<"S["<<k<<"] = "<<cplex1.getValue(S[k])<<endl;
             }
             */
            //
            //            for(IloInt i = 1 ; i <= numTrabalhos; i++){
            //                cout<<"D["<<i<<"] = "<<cplex1.getValue(D[i])<<endl;
            //            }
            //
            //            for(IloInt i = 1 ; i <= numTrabalhos; i++){
            //                cout<<"T["<<i<<"] = "<<cplex1.getValue(T[i])<<endl;
            //            }


            /*
            for(IloInt k=0;k<numVeiculos;k++){
                cout<<"Veículo "<<k<<": "<<endl;
                for(IloInt i=0;i<=numTrabalhos;i++)
                    for(IloInt j=0;j<=numTrabalhos;j++)
                        if(i!=j)
                            if(cplex1.getValue(X[i][j][k]) == 1)
                                cout<<"X["<<i<<"]["<<j<<"] = "<<cplex1.getValue(X[i][j][k])<<endl;
            }


            for(IloInt k=0;k<numVeiculos;k++){
                cout<<"Veículo "<<k<<": "<<endl;
                for(IloInt i=0;i<=numTrabalhos;i++)
                    for(IloInt j=0;j<=numTrabalhos;j++)
                        if(i!=j)
                            if(cplex1.getValue(X[i][j][k]) == 1)
                                cout<<"t["<<i<<"]["<<j<<"] = "<<t[i][j]<<endl;
            }
            */



            myfile<<setw(15)<<instance;
            myfile<<setw(12)<<cplex1.getObjValue();
            myfile<<setw(10)<<firstTerm;
            myfile<<setw(10)<<secondTerm;
            myfile<<setw(11)<<thirdTerm;
            myfile<<setw(14)<<cplex1.getMIPRelativeGap()*100;
            myfile<<setw(12)<<cplex1.getStatus();
            myfile<<setw(12)<<tempo<<"\t";

            // cout<<"Job sequence:\n";
            vector<pair<int,int> > sequencia(numTrabalhos);
            for(int i=0;i<numTrabalhos;i++)
                sequencia[i]=make_pair(0,i+1);

            for(int i=1;i<=numTrabalhos;i++){
                for(int j=1;j<=numTrabalhos;j++){
                    if(i==j)
                        continue;

                    VarValue = cplex1.getValue(A[i][j]);
                    if(VarValue >= 0.9 && VarValue <= 1.1){
                        //if(cplex1.getValue(A[i][j])==1){
                        ++sequencia[j-1].first;
                    }
                }
            }

            sort(sequencia.begin(),sequencia.end()); //Ordenando de forma que as tarefas que mais aparecem ants de outras vao para o inicio do vetor
            for(int i=0;i<numTrabalhos;i++){
                myfile<<sequencia[i].second;
                if(i!=numTrabalhos-1){
                    myfile<<"-";
                }
            }

            myfile<<"    ";
            //Imprimindo as entregas por ordem de Sk
            vector<pair<int,int> > delivery;
            for(int i=0;i<numVeiculos;i++){
                VarValue = cplex1.getValue(Y[i]);
                if(VarValue >= 0.9 && VarValue <= 1.1){
                    //if(cplex1.getValue(Y[i])==1){
                    delivery.push_back( make_pair(cplex1.getValue(S[i]), i) );
                }
            }
            sort(delivery.begin(),delivery.end());

            /*
            //Imprimindo a ordem de entrega
            string entrega="";
            for(int i=0;i<delivery.size();i++){
                int whichCar=delivery[i].second;
                int next=0;
                for(int j=1; j<=numTrabalhos; j++){
                    VarValue = cplex1.getValue(X[0][j][whichCar]);
                    if(VarValue >= 0.9 && VarValue <= 1.1){
                        //if(cplex1.getValue(X[0][j][whichCar])==1){
                        sprintf(entrega, "|-%d-",j);

                        //entrega+= "|-"+to_string(j)+"-";
                        next=j;
                        break;
                    }
                }
                bool imprimiRota=false;

                while(!imprimiRota){
                    VarValue = cplex1.getValue(X[next][0][whichCar]);
                    if(VarValue >= 0.9 && VarValue <= 1.1){
                        //if(cplex1.getValue(X[next][0][whichCar])==1){
                        entrega+="| ";
                        imprimiRota=true;
                        break;
                    }
                    for(int j=0;j<=numTrabalhos; j++){
                        if(j==next){
                            continue;
                        }
                        VarValue = cplex1.getValue(X[next][j][whichCar]);
                        if(VarValue >= 0.9 && VarValue <= 1.1){
                            //if(cplex1.getValue(X[next][j][whichCar])==1){
                            //entrega+= to_string(j)+"-";
                            sprintf(entrega, "%d-",j);
                            next=j;
                            break;
                        }

                    }
                }
            }

            myfile<<entrega<<endl;
            */

             for(int i=0;i<delivery.size();i++){
                 int whichCar=delivery[i].second;
                 if(i==0){
                    myfile<<"| ";
                 }
                 int whichIndex=0;
                 for(int b=0;b<=numTrabalhos;b++){
                     if (b==whichIndex) continue;
                     if(cplex1.getValue(X[whichIndex][b][whichCar])>=0.9 && cplex1.getValue(X[whichIndex][b][whichCar])<=1.1){
                         myfile<<b<<"-";
                         whichIndex=b;
                         break;
                     }
                 }
                 while((cplex1.getValue(X[whichIndex][0][whichCar])<=0.1) ){
                     for(int b=0;b<=numTrabalhos;b++){
                         if(cplex1.getValue(X[whichIndex][b][whichCar])>=0.9 && cplex1.getValue(X[whichIndex][b][whichCar])<=1.1){
                             myfile<<b;
                             whichIndex=b;
                             if((cplex1.getValue(X[whichIndex][0][whichCar])<=0.1)){
                             myfile<<"-";
                             }
                             break;
                         }
                     }
                 }
                 if(i!=delivery.size()-1){

                    myfile<<" | ";
                 }else{

                    myfile<<" |"<<endl;
                 }
             }


            ambiente.end();
        }
    }
    in.close();
    myfile.close();
    return 0;
}