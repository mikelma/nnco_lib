#include "algorithms.h"
#include "PlackettLuce.h"
#include "BradleyTerry.h"
#include "LopInstance.h"
#include "utility_functions.h"
#include "utils.h"
#include <cmath>
#include <cstdlib>
#include <iostream>
#include <fstream>

using namespace std;
using Eigen::MatrixXd;
using Eigen::Matrix;
using Eigen::VectorXd;
using Eigen::Map;


int updateLambda(PlackettLuce * model, int lowerlambda, int upperlambda, bool restart, int initialLambda){
if (restart==false){
    ldouble entropy=model->entropyParameters();
    int lambda=lowerlambda+(upperlambda-lowerlambda)*(entropy/log(model->n));
    if (lambda<lowerlambda || lambda>upperlambda){
        cout<<"lambda: "<<lambda<<"  error!!!"<<"(entropy: "<<entropy<<" log: "<<log(model->n)<<")"<<endl;
    }
    return (lambda);
}
else{
    return initialLambda;
}
}

ldouble norm2(ldouble* v, int n) {
	ldouble sumSquares = 0.;
	for (int k=0; k<n; k++)
		sumSquares += v[k]*v[k];
	return sqrt(sumSquares);
}


ldouble updateAlpha(ldouble previousAlpha, ldouble* p, ldouble* delta, int n, bool restart, ldouble initialAlpha, ldouble lowerAlpha, ldouble upperAlpha) {
	//let's keep the r parameter constant to this value
	static ldouble r = 1./sqrt(n); //1./n
	//if it is the first iteration from last restart, then p gets zeros
	if (restart) {
        //for (int k=0; k<n; k++)
        //    p[k] = 0.;
        memset(p, 0., sizeof(ldouble)*n);
		return initialAlpha;
	}
	//compute Euclidean norm of delta
	ldouble normDelta = norm2(delta,n);
	//update cumulation vector: p = (1-r)*p + normalizedDelta
	for (int k=0; k<n; k++)
		p[k] = (1.-r)*p[k] + delta[k]/normDelta;
	//new alpha computation
	ldouble alpha = previousAlpha * exp( norm2(p,n) / sqrt(1./(2.*r-r*r)) - 1. ); //questa è corretta!
	if (alpha>upperAlpha) alpha = upperAlpha;
	else if (alpha<lowerAlpha) alpha = lowerAlpha;
	return alpha;
}

ldouble updateAlpha(ldouble previousAlpha, ldouble* p, VectorXd deltaVector, int n, bool restart, ldouble initialAlpha, ldouble lowerAlpha, ldouble upperAlpha) {
    //let's keep the r parameter constant to this value
    static ldouble r = 1./sqrt(n); //1./n
    //if it is the first iteration from last restart, then p gets zeros
    if (restart) {
        //for (int k=0; k<n; k++)
        //    p[k] = 0.;
        memset(p, 0., sizeof(ldouble)*n);
        return initialAlpha;
    }
    //compute Euclidean norm of delta
    //ldouble normDelta = norm2(delta,n);
    ldouble normDelta = deltaVector.norm();
    //update cumulation vector: p = (1-r)*p + normalizedDelta
    for (int k=0; k<n; k++)
        p[k] = (1.-r)*p[k] + deltaVector(k)/normDelta;
    //new alpha computation
    ldouble alpha = previousAlpha * exp( norm2(p,n) / sqrt(1./(2.*r-r*r)) - 1. ); //questa è corretta!
    if (alpha>upperAlpha) alpha = upperAlpha;
    else if (alpha<lowerAlpha) alpha = lowerAlpha;
    return alpha;
}


int* gradientAscent_PL_selfadjusting_learning(string lopInstanceName, LopInstance& lop, int RUNNING_MODE, int lambda, ldouble initialalpha, int maxEvaluations, string utilityFunction, int printInterval, unsigned int seed, ldouble lowerAlpha, ldouble upperAlpha) {

    ldouble alpha=initialalpha;

    int iter=1,i,k,fmax=-1,nfes=0,n=lop.n,nrestarts=0;
    ldouble delta[n],grad[n],p[n]; //p is the cumulation vector required for updating alpha
    bool ok = true;//initialized just to avoid compiler warning
    int opt[n]; //optimum so far

    //setup memory for samples
    int samples1d[n*lambda],fx[lambda];
    int* x[lambda];
    for (i=0; i<lambda; i++)
        x[i] = &samples1d[i*n];

    //setup utility function
    UtilityFunction* uf = UtilityFunction::getUtilityFunction(utilityFunction,x,fx,lambda,&lop);

    //init to uniform distribution
    PlackettLuce pl(n);

	//initial setting for alpha
	bool restart = true;
	alpha = updateAlpha(alpha, p, delta, n, restart, initialalpha, lowerAlpha, upperAlpha);
    int best_evals=0;
    int samples=lambda;
    while(nfes<maxEvaluations){
        if ((lambda+nfes)>maxEvaluations)
            samples=(maxEvaluations-nfes);
        //sample lambda permutations and evaluate them
        for (i=0; i<samples; i++) {
            pl.sample(x[i]);
            fx[i] = lop.eval(x[i]);
            nfes++;
            if (fx[i]>fmax) {
                fmax = fx[i];
                best_evals=nfes;
                copyPerm(opt,x[i],n);
                #ifdef VERBOSE
                cout<<"[PLSL] evals consumed: "<<nfes<<" fitness: "<<fmax<<" alpha: "<<alpha<<endl;
                #endif
            }
        }
        if (nfes<maxEvaluations){
			//calculate delta-vector by means of gradients
			uf->preUtility();
			for (k=0; k<n; k++)
				delta[k] = 0.L;
			for (i=0; i<lambda; i++) {
				ok = pl.gradLogProb(grad,x[i]);
				if (!ok) break; //exit cycle if there was a numerical problem
				for (k=0; k<n; k++)
					delta[k] += uf->utility(i) * grad[k];
			}

			//print somethig
            if (RUNNING_MODE==2){
            if (printInterval && (iter==1 || iter%printInterval==0)) {
				int mode[n];
				pl.mode(mode);
				cout <<lopInstanceName << ";" << "PLSL" << ";" << lambda << ";" << alpha << ";" << iter << ";" << lop.eval(mode) << ";" << pl.entropyParameters() << ";" << pl.maxProb() << ";" << fmax << ";" << seed << "\n";
				if (!ok) cerr << "Numerical problem occurred!!!\n";
            }}
			//update Plackett Luce weights
			if (ok) { //no numerical problem, so update as usual
				for (k=0; k<n; k++){
					pl.w[k] += alpha*delta[k];
				}
				pl.updateInnerParameters();
				restart = false;
			} else { //there was a numerical problem, so restart to a degenerate distribution with mode in the optimum so far
				pl.setDegenerateParameters(opt);
				nrestarts++;
				restart = true;
			}

			//update alpha
			//cout << alpha << endl;
            alpha = updateAlpha(alpha, p, delta, n, restart, initialalpha, lowerAlpha, upperAlpha);

			//update lambda
			//lambda=updateLambda(&x, n, lowerlambda, upperlambda);
			// lambda=updateLambda(x,n, lowerlambda, upperlambda);
        }

        //end-for
        iter++;

		//if (iter==20) break;
    }
    //print something
    //cerr << "fmax      = " << fmax << "\n";
    //cerr << "nrestarts = " << nrestarts << "\n";
    //free memory
    delete uf;
    //return maximum fitness so far
    int * best= new int[2];
    best[0]=fmax;
    best[1]=best_evals;
    return best;
}

int* gradientAscent_PL_selfadjusting_sample(string lopInstanceName, LopInstance& lop,int RUNNING_MODE, int initialLambda, int lowerlambda, int upperlambda, ldouble alpha, int maxEvaluations, string utilityFunction, int printInterval, unsigned int seed) {

    int lambda = 100;
    int iter=1,i,k,fmax=-1,nfes=0,n=lop.n,nrestarts=0;
    ldouble delta[n],grad[n],p[n]; //p is the cumulation vector required for updating alpha
    bool ok = true;//initialized just to avoid compiler warning
    int opt[n]; //optimum so far

    //setup memory for samples
    int samples1d[n*upperlambda],fx[upperlambda];
    int* x[upperlambda];
    for (i=0; i<upperlambda; i++)
        x[i] = &samples1d[i*n];

    //setup utility function
    UtilityFunction* uf = UtilityFunction::getUtilityFunction(utilityFunction,x,fx,lambda,&lop);

    //init to uniform distribution
    PlackettLuce pl(n);

    //initial setting for lambda
    bool restart = true;

    int best_evals=0;
    int samples;
    while(nfes<maxEvaluations){
        samples=lambda;
        if ((lambda+nfes)>maxEvaluations)
            samples=(maxEvaluations-nfes);
        //sample lambda permutations and evaluate them
       // cout<<"sampling..."<<endl;
        for (i=0; i<samples; i++) {
            pl.sample(x[i]);
         //   cout<<"sample i: "<<i<<"   "; printPerm(x[i],n,"");
            fx[i] = lop.eval(x[i]);
            nfes++;
            if (fx[i]>fmax) {
                fmax = fx[i];
                best_evals=nfes;
                copyPerm(opt,x[i],n);
                #ifdef VERBOSE
                cout<<"[PLSS] evals consumed: "<<nfes<<" fitness: "<<fmax<<" lambda: "<<lambda<<endl;
                #endif
            }
        }
        //cout<<"gradient computation"<<endl;
        if (nfes<maxEvaluations){
            //calculate delta-vector by means of gradients
          //  cout<<"pre utility"<<endl;
            uf->preUtility();
            for (k=0; k<n; k++)
                delta[k] = 0.L;
            for (i=0; i<lambda; i++) {
            //    cout<<"i: "<<i<<endl;
                ok = pl.gradLogProb(grad,x[i]);
                if (!ok)
                {
              //      cout<<"Numerical error"<<endl;
                    break; //exit cycle if there was a numerical problem
                }
                for (k=0; k<n; k++)
                    delta[k] += uf->utility(i) * grad[k];
            }

            //print somethig
            if (RUNNING_MODE==2){
            if (printInterval && (iter==1 || iter%printInterval==0)) {
                int mode[n];
                pl.mode(mode);
                cout <<lopInstanceName << ";" << "PLSS" << ";" << lambda << ";" << alpha << ";" << iter << ";" << lop.eval(mode) << ";" << pl.entropyParameters() << ";" << pl.maxProb() << ";" << fmax << ";" << seed << "\n";
                if (!ok) cerr << "Numerical problem occurred!!!\n";
            }}
           // cout<<"we are here"<<endl;
            //update Plackett Luce weights
            if (ok) { //no numerical problem, so update as usual
                for (k=0; k<n; k++){
                    pl.w[k] += alpha*delta[k];
                }
                pl.updateInnerParameters();
                restart = false;
            } else { //there was a numerical problem, so restart to a degenerate distribution with mode in the optimum so far
                pl.setDegenerateParameters(opt);
                nrestarts++;
                restart = true;
            }
            //update lambda
           // cout<<"updating lambda"<<endl;
            lambda=updateLambda(&pl, lowerlambda, upperlambda, restart, initialLambda);
            delete uf;
            uf = UtilityFunction::getUtilityFunction(utilityFunction,x,fx,lambda,&lop);
            //cout<<"here"<<endl;
        }

        //end-for
        iter++;

    }

    //free memory
    delete uf;
    //return maximum fitness so far
    int * best= new int[2];
    best[0]=fmax;
    best[1]=best_evals;
    return best;
}

int* gradientAscent_PL_selfadjusting_total(string lopInstanceName, LopInstance& lop, int RUNNING_MODE, int initialLambda, int lowerlambda, int upperlambda, ldouble initialalpha,  int maxEvaluations, string utilityFunction, int printInterval, unsigned int seed, ldouble lowerAlpha, ldouble upperAlpha) {

    ldouble alpha=initialalpha;
    int lambda = initialLambda;
    int iter=1,i,k,fmax=-1,nfes=0,n=lop.n,nrestarts=0;
    ldouble delta[n],grad[n],p[n]; //p is the cumulation vector required for updating alpha
    bool ok = true;//initialized just to avoid compiler warning
    int opt[n]; //optimum so far

    //setup memory for samples
    int samples1d[n*upperlambda],fx[upperlambda];
    int* x[upperlambda];
    for (i=0; i<upperlambda; i++)
        x[i] = &samples1d[i*n];

    //setup utility function
    UtilityFunction* uf = UtilityFunction::getUtilityFunction(utilityFunction,x,fx,lambda,&lop);

    //init to uniform distribution
    PlackettLuce pl(n);
#ifdef RUNNING_ON_PRISCILLA
    FILE *result_file;
    if (RUNNING_MODE==2)
    {
        result_file= fopen("./scratch_results.csv","a+");
    }
#endif

    //initial setting for alpha
    bool restart = true;

    alpha = updateAlpha(alpha, p, delta, n, restart, initialalpha,lowerAlpha, upperAlpha);

    int best_evals=0;
    int samples;
    while(nfes<maxEvaluations){
        samples=lambda;
        if ((lambda+nfes)>maxEvaluations)
            samples=(maxEvaluations-nfes);
        //sample lambda permutations and evaluate them
       // cout<<"sampling..."<<endl;
        for (i=0; i<samples; i++) {
            pl.sample(x[i]);
         //   cout<<"sample i: "<<i<<"   "; printPerm(x[i],n,"");
            fx[i] = lop.eval(x[i]);
            nfes++;
            if (fx[i]>fmax) {
                fmax = fx[i];
                best_evals=nfes;
                copyPerm(opt,x[i],n);
                #ifdef VERBOSE
                cout<<"[PLSA] evals consumed: "<<nfes<<" fitness: "<<fmax<<" lambda: "<<lambda<<" alpha: "<<alpha<<endl;
                #endif
            }
        }

        //cout<<"gradient computation"<<endl;
        if (nfes<maxEvaluations){
            //calculate delta-vector by means of gradients
           //cout<<"pre utility"<<endl;
            uf->preUtility();
            for (k=0; k<n; k++)
                delta[k] = 0.L;
            for (i=0; i<lambda; i++) {
            //    cout<<"i: "<<i<<endl;
                ok = pl.gradLogProb(grad,x[i]);
                if (!ok)
                {
                    break; //exit cycle if there was a numerical problem
                }
                for (k=0; k<n; k++)
                    delta[k] += uf->utility(i) * grad[k];
            }

            //print somethig
            if (RUNNING_MODE==2){
        if (printInterval && (iter==1 || iter%printInterval==0)) {
            int mode[n];
            pl.mode(mode);
#ifndef RUNNING_ON_PRISCILLA
            cout <<lopInstanceName << ";" << "PLSA" << ";" << lambda << ";" << alpha << ";" << iter << ";" << lop.eval(mode) << ";" << pl.entropyParameters() << ";" << pl.maxProb() << ";" << fmax << ";" << seed << ";" <<nrestarts<<"\n";
            if (!ok) { cerr << "Numerical problem occurred... restart model\n";}
#else
            fprintf(result_file,"\"%s\";\"%s\";%d;%.3Lf;%d;%d;%.3Lf;%.3Lf;%d;%d;%d\n",lopInstanceName.c_str(),"PLSA",lambda,alpha,iter,lop.eval(mode),pl.entropyParameters(),pl.maxProb(),fmax,seed,nrestarts);
#endif
        }
            }
            //update Plackett Luce weights
            if (ok) { //no numerical problem, so update as usual
                for (k=0; k<n; k++){
                    pl.w[k] += alpha*delta[k];
                }
                pl.updateInnerParameters();
                ldouble entropy=pl.entropyParameters();
                if (isInfNan(entropy)){
                    pl.setDegenerateParameters(opt);
                    nrestarts++;
                    restart = true;
                }
                else{
                    restart = false;
                }
            } else { //there was a numerical problem, so restart to a degenerate distribution with mode in the optimum so far
                pl.setDegenerateParameters(opt);
                nrestarts++;
                restart = true;
            }
            //update alpha
            alpha = updateAlpha(alpha, p, delta, n, restart, initialalpha, lowerAlpha, upperAlpha);

            //update lambda
            lambda=updateLambda(&pl, lowerlambda, upperlambda, restart, initialLambda);

            delete uf;
            uf = UtilityFunction::getUtilityFunction(utilityFunction,x,fx,lambda,&lop);
        }

        //end-for
        iter++;

    }
#ifdef RUNNING_ON_PRISCILLA
    if (RUNNING_MODE==2)
        fclose(result_file);
#endif
    //free memory
    delete uf;
    //return maximum fitness so far
    int * best= new int[2];
    best[0]=fmax;
    best[1]=best_evals;
    return best;
}

#define LOG_CONVERGENCY 1

int* gradientAscent_PL(string lopInstanceName, LopInstance& lop,int RUNNING_MODE, ldouble alpha, int lambda, int maxEvaluations, string utilityFunction, int printInterval, unsigned int seed, ldouble lowerAlpha, ldouble upperAlpha) {
	int iter=1,i,k,fmax=-1,nfes=0,n=lop.n,nrestarts=0;
	ldouble delta[n],grad[n];
	bool ok = true;//initialized just to avoid compiler warning
	int opt[n]; //optimum so far
	//setup memory for samples
	int samples1d[n*lambda],fx[lambda];
	int* x[lambda];
	for (i=0; i<lambda; i++)
		x[i] = &samples1d[i*n];

	//setup utility function
	UtilityFunction* uf = UtilityFunction::getUtilityFunction(utilityFunction,x,fx,lambda,&lop);

	//init to uniform distribution
	PlackettLuce pl(n);

#ifdef LOG_CONVERGENCY
        printf("\n[NOTE] Logging convergency curves. This introduces latency, \
you can disble it using the LOG_CONVERGENCY variable in algorithms.cpp, bye.\n");
        ofstream biglog;
        string fname = "convergency_" + std::to_string(rand()) + ".csv";
        biglog.open(fname);
        biglog << "iteration,distribution,best fitness\n";
#endif


#ifdef RUNNING_ON_PRISCILLA
    FILE *result_file;
    if (RUNNING_MODE==2)
    {
        char fname[30];
        snprintf(fname, 30, "convergence/%d.csv", seed);
        result_file= fopen(fname,"a+");
        fprintf(result_file,"instance,algorithm,lambda,alpha,iter,fmode,pl entropy,pl max prob,best fitness,seed,nrestarts\n");
    }
#endif
    int best_evals=0;
    int samples=lambda;
    while(nfes<maxEvaluations){
        if ((lambda+nfes)>maxEvaluations)
            samples=(maxEvaluations-nfes);
		//sample lambda permutations and evaluate them
		for (i=0; i<samples; i++) {
            pl.sample(x[i]);
            //pl.sample_fast(x[i]);
			fx[i] = lop.eval(x[i]);
			nfes++;
			if (fx[i]>fmax) {
				fmax = fx[i];
                copyPerm(opt,x[i],n);
                best_evals=nfes;
                #ifdef VERBOSE
                cout<<"[PL] evals consumed: "<<nfes<<" fitness: "<<fmax<<endl;
                #endif
			}
		}
        if (nfes<maxEvaluations){
		//calculate delta-vector by means of gradients
		uf->preUtility();
		for (k=0; k<n; k++)
			delta[k] = 0.L;
		for (i=0; i<lambda; i++) {
			ok = pl.gradLogProb(grad,x[i]);
			if (!ok) break; //exit cycle if there was a numerical problem
			for (k=0; k<n; k++)
				delta[k] += uf->utility(i) * grad[k];
		}

		//print somethig
        if (RUNNING_MODE==2){
		if (printInterval && (iter==1 || iter%printInterval==0)) {

            int mode[n];
			pl.mode(mode);
#ifndef RUNNING_ON_PRISCILLA
			cout <<lopInstanceName << "," << "GS" << "," << lambda << "," << alpha << "," << iter << "," << lop.eval(mode) << "," << pl.entropyParameters() << "," << pl.maxProb() << "," << fmax << "," << seed << "," <<nrestarts<<"\n";
            if (!ok) { cerr << "Numerical problem occurred... restart model\n";}
#else
            fprintf(result_file,"\"%s\",\"%s\",%d,%.3Lf,%d,%d,%.3Lf,%.3Lf,%d,%d,%d\n",lopInstanceName.c_str(),"GS",lambda,alpha,iter,lop.eval(mode),pl.entropyParameters(),pl.maxProb(),fmax,seed,nrestarts);
#endif
        }
            }
		//update Plackett Luce weights
		if (ok) { //no numerical problem, so update as usual
            for (k=0; k<n; k++){
				pl.w[k] += alpha*delta[k];
            }
            pl.updateInnerParameters();
            ldouble entropy=pl.entropyParameters();
            if (isInfNan(entropy)){
                pl.setDegenerateParameters(opt);
                nrestarts++;
            }
		} else { //there was a numerical problem, so restart to a degenerate distribution with mode in the optimum so far
			pl.setDegenerateParameters(opt);
			nrestarts++;
		}
        }
		//end-for

#ifdef LOG_CONVERGENCY
        if (iter % 10 == 0 || iter == 1) {
            biglog << iter << ",\"[";
            for (int i=0; i<  lop.n -1; i++) {
                biglog << pl.expw[i] << ",";
            }
            biglog << pl.expw[lop.n-1] << "]\",";
            biglog << fmax << "\n";
        }
#endif
        iter++;
	}
#ifdef RUNNING_ON_PRISCILLA
    if (RUNNING_MODE==2)
        fclose(result_file);
#endif

	//free memory
	delete uf;
	//return maximum fitness so far
    int * best= new int[2];
    best[0]=fmax;
    best[1]=best_evals;
	return best;
}

int* naturalgradientAscent_PL(string lopInstanceName, LopInstance& lop, int RUNNING_MODE, ldouble alpha, int lambda, int maxEvaluations, string utilityFunction, int printInterval, unsigned int seed, ldouble lowerAlpha, ldouble upperAlpha) {
    int iter=1,i,k,fmax=-1,nfes=0,n=lop.n,nrestarts=0;
    //ldouble delta[n];
    ldouble grad[n];
    bool ok = true;//initialized just to avoid compiler warning
    int opt[n]; //optimum so far
    //setup memory for samples
    int samples1d[n*lambda],fx[lambda];
    int* x[lambda];
    for (i=0; i<lambda; i++)
        x[i] = &samples1d[i*n];
    MatrixXd Finv;
    MatrixXd F = MatrixXd::Constant(n,n,0); //INITIALIZE FISHER MATRIX
    VectorXd gradLog = VectorXd::Constant(n, 0); //INITIALIZE GRADIENTS VECTOR
    VectorXd FJ= VectorXd::Constant(n, 0);
    VectorXd deltaVector= VectorXd::Constant(n, 0);

    //setup utility function
    UtilityFunction* uf = UtilityFunction::getUtilityFunction(utilityFunction,x,fx,lambda,&lop);

    //init to uniform distribution
    PlackettLuce pl(n);
#ifdef RUNNING_ON_PRISCILLA
    FILE *result_file;
    if (RUNNING_MODE==2)
    {
        result_file= fopen("./scratch_results.csv","a+");
    }
#endif

    int best_evals=0;
    int samples=lambda;
    while(nfes<maxEvaluations){
        if ((lambda+nfes)>maxEvaluations)
            samples=(maxEvaluations-nfes);
        //sample lambda permutations and evaluate them
        for (i=0; i<samples; i++) {
            pl.sample(x[i]);
            fx[i] = lop.eval(x[i]);
            nfes++;
            if (fx[i]>fmax) {
                fmax = fx[i];
                copyPerm(opt,x[i],n);
                best_evals=nfes;
                #ifdef VERBOSE
                cout<<"[PLN] evals consumed: "<<nfes<<" fitness: "<<fmax<<endl;
                #endif
            }
        }

        if (nfes<maxEvaluations){
        //calculate delta-vector by means of gradients
        uf->preUtility();
        //for (k=0; k<n; k++)
         //   delta[k] = 0.L;
        deltaVector.setZero();
        F.setZero();
        for (i=0; i<lambda; i++) {
            ok = pl.gradLogProb(grad,x[i]);
            if (!ok) break; //exit cycle if there was a numerical problem
            // COMPUTE DELTA GRAD
            for (k=0; k<n; k++){
                //delta[k] += uf->utility(i) * grad[k];
                gradLog(k)=grad[k];
                deltaVector(k)= deltaVector(k)+ (uf->utility(i) * grad[k]);
            }
            F=F+(gradLog*gradLog.transpose());
        }
        if (ok){
            F=F/lambda;
            Finv= F.inverse();
            if (isInfNan(Finv(1,1))){
                ok=false;
        }
        else{
            FJ=Finv*(deltaVector/lambda);
        }

        }

        //print somethig
            if (RUNNING_MODE==2){
        if (printInterval && (iter==1 || iter%printInterval==0)) {
            int mode[n];
            pl.mode(mode);
#ifndef RUNNING_ON_PRISCILLA
            cout <<lopInstanceName << ";" << "PLN" << ";" << lambda << ";" << alpha << ";" << iter << ";" << lop.eval(mode) << ";" << pl.entropyParameters() << ";" << pl.maxProb() << ";" << fmax << ";" << seed << ";" <<nrestarts<<"\n";
            if (!ok) { cerr << "Numerical problem occurred... restart model\n";}
#else
            fprintf(result_file,"\"%s\";\"%s\";%d;%.3Lf;%d;%d;%.3Lf;%.3Lf;%d;%d;%d\n",lopInstanceName.c_str(),"PLN",lambda,alpha,iter,lop.eval(mode),pl.entropyParameters(),pl.maxProb(),fmax,seed,nrestarts);
#endif
        }
            }

        //update Plackett Luce weights
        if (ok) { //no numerical problem, so update as usual
            for (k=0; k<n; k++){
                pl.w[k] += alpha*FJ(k); // NEW
            }
            pl.updateInnerParameters();
            ldouble entropy=pl.entropyParameters();
            if (isInfNan(entropy)){
                pl.setDegenerateParameters(opt);
                nrestarts++;
            }

        } else { //there was a numerical problem, so restart to a degenerate distribution with mode in the optimum so far
            pl.setDegenerateParameters(opt);
            nrestarts++;
        }
        }
        //end-for
        iter++;
    }
#ifdef RUNNING_ON_PRISCILLA
    if (RUNNING_MODE==2)
        fclose(result_file);
#endif
    //free memory
    delete uf;
    //return maximum fitness so far
    int * best= new int[2];
    best[0]=fmax;
    best[1]=best_evals;
    return best;
}


int* naturalgradientAscent_PL_selfadjusting_total(string lopInstanceName, LopInstance& lop,int RUNNING_MODE,  int initialLambda, int lowerlambda, int upperlambda, ldouble initialalpha, int maxEvaluations, string utilityFunction, int printInterval, unsigned int seed, ldouble lowerAlpha, ldouble upperAlpha) {

    ldouble alpha=initialalpha;
    int lambda = initialLambda;
    int iter=1,i,k,fmax=-1,nfes=0,n=lop.n,nrestarts=0;
    ldouble delta[n],grad[n],p[n]; //p is the cumulation vector required for updating alpha
    bool ok = true;//initialized just to avoid compiler warning
    int opt[n]; //optimum so far

    //setup memory for samples
    int samples1d[n*upperlambda],fx[upperlambda];
    int* x[upperlambda];
    for (i=0; i<upperlambda; i++)
        x[i] = &samples1d[i*n];
    MatrixXd Finv;
    MatrixXd F = MatrixXd::Constant(n,n,0); //INITIALIZE FISHER MATRIX
    VectorXd gradLog = VectorXd::Constant(n, 0); //INITIALIZE GRADIENTS VECTOR
    VectorXd FJ= VectorXd::Constant(n, 0);
    VectorXd deltaVector= VectorXd::Constant(n, 0);

    //setup utility function
    UtilityFunction* uf = UtilityFunction::getUtilityFunction(utilityFunction,x,fx,lambda,&lop);

    //init to uniform distribution
    PlackettLuce pl(n);
#ifdef RUNNING_ON_PRISCILLA
    FILE *result_file;
    if (RUNNING_MODE==2)
    {
        result_file= fopen("./scratch_results.csv","a+");
    }
#endif

    //initial setting for alpha
    bool restart = true;
    alpha = updateAlpha(alpha, p, delta, n, restart, initialalpha, lowerAlpha, upperAlpha);

    int best_evals=0;
    int samples;
    while(nfes<maxEvaluations){
        samples=lambda;
        if ((lambda+nfes)>maxEvaluations)
            samples=(maxEvaluations-nfes);
        //sample lambda permutations and evaluate them
       // cout<<"sampling..."<<endl;
        for (i=0; i<samples; i++) {
            pl.sample(x[i]);
            fx[i] = lop.eval(x[i]);
            nfes++;
            if (fx[i]>fmax) {
                fmax = fx[i];
                best_evals=nfes;
                copyPerm(opt,x[i],n);
                #ifdef VERBOSE
                cout<<"[PLNSA] evals consumed: "<<nfes<<" fitness: "<<fmax<<" lambda: "<<lambda<<" alpha: "<<alpha<<endl;
                #endif
            }
        }

        if (nfes<maxEvaluations){
        //calculate delta-vector by means of gradients
        uf->preUtility();
        deltaVector.setZero();
        F.setZero();
        for (i=0; i<lambda; i++) {
            ok = pl.gradLogProb(grad,x[i]);
            if (!ok) break; //exit cycle if there was a numerical problem
            // COMPUTE DELTA GRAD
            for (k=0; k<n; k++){
                //delta[k] += uf->utility(i) * grad[k];
                gradLog(k)=grad[k];
                deltaVector(k)= deltaVector(k)+ (uf->utility(i) * grad[k]);
            }
            F=F+(gradLog*gradLog.transpose());
        }
        if (ok){
            F=F/lambda;
            Finv= F.inverse();
            if (isInfNan(Finv(1,1))){
                ok=false;
            }
            else{
                FJ=Finv*(deltaVector/lambda);
            }
        }

            //print somethig
            if (RUNNING_MODE==2){
        if (printInterval && (iter==1 || iter%printInterval==0)) {
            int mode[n];
            pl.mode(mode);
#ifndef RUNNING_ON_PRISCILLA
            cout <<lopInstanceName << ";" << "PLNSA" << ";" << lambda << ";" << alpha << ";" << iter << ";" << lop.eval(mode) << ";" << pl.entropyParameters() << ";" << pl.maxProb() << ";" << fmax << ";" << seed << ";" <<nrestarts<<"\n";
            if (!ok) { cerr << "Numerical problem occurred... restart model\n";}
#else
            fprintf(result_file,"\"%s\";\"%s\";%d;%.3Lf;%d;%d;%.3Lf;%.3Lf;%d;%d;%d\n",lopInstanceName.c_str(),"PLNSA",lambda,alpha,iter,lop.eval(mode),pl.entropyParameters(),pl.maxProb(),fmax,seed,nrestarts);
#endif
        }
            }
            //update Plackett Luce weights
            if (ok) { //no numerical problem, so update as usual
                for (k=0; k<n; k++){
                    pl.w[k] += alpha*FJ(k); // NEW
                }

                pl.updateInnerParameters();
                ldouble entropy=pl.entropyParameters();
                if (isInfNan(entropy)){
                    pl.setDegenerateParameters(opt);
                    nrestarts++;
                    restart = true;
                }else{
                restart = false;
                }
            } else { //there was a numerical problem, so restart to a degenerate distribution with mode in the optimum so far
                pl.setDegenerateParameters(opt);
                nrestarts++;
                restart = true;
            }
            //update alpha
            alpha = updateAlpha(alpha, p, deltaVector, n, restart, initialalpha, lowerAlpha, upperAlpha);

            //update lambda
            lambda=updateLambda(&pl, lowerlambda, upperlambda, restart, initialLambda);
            delete uf;
            uf = UtilityFunction::getUtilityFunction(utilityFunction,x,fx,lambda,&lop);
        }

        //end-for
        iter++;

    }
#ifdef RUNNING_ON_PRISCILLA
    if (RUNNING_MODE==2)
        fclose(result_file);
#endif
    //free memory
    delete uf;
    //return maximum fitness so far
    int * best= new int[2];
    best[0]=fmax;
    best[1]=best_evals;
    return best;
}


int* gradientAscent_BT(string lopInstanceName, LopInstance& lop, int RUNNING_MODE, ldouble alpha, int lambda, int sampling_iterations, int maxEvaluations, string utilityFunction, int printInterval, unsigned int seed, ldouble lowerAlpha, ldouble upperAlpha) {

    int iter=1,i,k,fmax=-1,nfes=0,n=lop.n,nrestarts=0;
    ldouble delta[n],grad[n];
    bool ok = true;//initialized just to avoid compiler warning
    int opt[n]; //optimum so far
    //setup memory for samples
    int samples1d[n*lambda],fx[lambda];
    int* x[lambda];
    for (i=0; i<lambda; i++)
        x[i] = &samples1d[i*n];
    //setup utility function
    UtilityFunction* uf = UtilityFunction::getUtilityFunction(utilityFunction,x,fx,lambda,&lop);
    //init to uniform distribution
    BradleyTerry bt(n);
    //print header string
    //if (printInterval)
    //    cout << "lambda;alpha;iter;fmode;entropy;maxw;fmax;seed\n";
    //main loop

    //for (iter=1; iter<=maxIterations; iter++){
    int samples=lambda;
    int best_evals=0;
   // cout<<"max evaluations: "<<maxEvaluations<<endl;
    while(nfes<maxEvaluations){
        if ((lambda+nfes)>maxEvaluations)
            samples=(maxEvaluations-nfes);
        //sample lambda permutations and evaluate them
        for (i=0; i<samples; i++) {
            bt.sample_heuristic_classical(x[i],sampling_iterations);
            fx[i] = lop.eval(x[i]);
            nfes++;
            if (fx[i]>fmax) {
                fmax = fx[i];
                copyPerm(opt,x[i],n);
                best_evals=nfes;
                #ifdef VERBOSE
                cout<<"iter BT: "<<iter<<" fitness: "<<fmax<<endl;
                #endif
            }
        }

        if (nfes<maxEvaluations){
          //  cout<<"inside"<<endl;
        //calculate delta-vector by means of gradients
        uf->preUtility();
        // this part is to remove it.

        for (k=0; k<n; k++)
            delta[k] = 0.L;


        for (i=0; i<samples; i++) {
            //ok = bt.gradLogProb_exhaustive(grad,x[i]); //just to ensure that the calculus of the grandients without the normalization constant is correct.
            //ok = bt.gradLogProb_exponential(grad,x[i]);
            //printValues(grad, n);
            ok = bt.gradLogProb_exponential_efficient(grad,x[i]);
            //printValues(grad, n);
            //exit(1);
            if (!ok) break; //exit cycle if there was a numerical problem
            for (k=0; k<n; k++)
                delta[k] += uf->utility(i) * grad[k];
        }

        //print somethig
            if (RUNNING_MODE==2){
        if (printInterval && (iter==1 || iter%printInterval==0)) {
            int mode[n];
            bt.mode(mode);
            cout <<lopInstanceName << ";" << "BT" << ";" << lambda << ";" << alpha << ";" << iter << ";" << lop.eval(mode) << ";" << bt.entropyParameters() << ";" << bt.maxProb() << ";" << fmax << ";" << seed << "\n";
            if (!ok) cerr << "Numerical problem occurred!!!\n";
        }
            }
        //update Bradley-Terry weights
        if (ok) { //no numerical problem, so update as usual
            for (k=0; k<n; k++)
                bt.w[k] += alpha*delta[k];
            bt.updateInnerParameters();


        } else { //there was a numerical problem, so restart to a degenerate distribution with mode in the optimum so far
            bt.setDegenerateParameters(opt);
            nrestarts++;
        }
            iter++;
        //end-for
    }
    }
    //print something
   // cerr << "fmax      = " << fmax << "\n";
   // cerr << "nrestarts = " << nrestarts << "\n";
   // cerr << "solution = "; printPerm(opt, n);
    //free memory
    delete uf;
    //return maximum fitness so far
    int * best= new int[2];
    best[0]=fmax;
    best[1]=best_evals;
    return best;
}
