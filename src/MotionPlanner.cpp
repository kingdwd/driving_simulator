
#include "MotionPlanner.h"
#include <algorithm>

#include <sstream>
#include <stdio.h>

const double PI = 3.1415926;
const double obs_veh_num = 4;
const double radius_disks = sqrt(4*4/pow(4*2,2)+2*2*0.25);
const double Length = 4;
const double Width = 2;
const double initi_ego_x = 27;
const double initi_ego_y = -12;
const double initi_ego_v = 3;
const double initi_obs_x = 40;
const double initi_obs_y = 8;
const double initi_obs_v = 5;
const int n_points_spline = 80;
const int N_SPLINE_POINTS = 30;



// Call despot in this file. subscribe the state of vehicles and publish the action as twist in publisher.


vector<tk::spline> Road::Ref_path(vector<double> x, vector<double> y, vector<double> theta, double& dist_spline_pts) {

	double k, dk, L;
	vector<double> X(10), Y(10);
	vector<double> X_all, Y_all, S_all;
	double total_length=0;
	int n_clothoid = 20;
	S_all.push_back(0);



	for (int i = 0; i < x.size()-1; i++){
		Clothoid::buildClothoid(x[i], y[i], theta[i], x[i+1], y[i+1], theta[i+1], k, dk, L);

		Clothoid::pointsOnClothoid(x[i], y[i], theta[i], k, dk, L, n_clothoid, X, Y);
		if (i==0){
			X_all.insert(X_all.end(), X.begin(), X.end());
			Y_all.insert(Y_all.end(), Y.begin(), Y.end());
		}
		else{
			X.erase(X.begin()+0);
			Y.erase(Y.begin()+0);
			X_all.insert(X_all.end(), X.begin(), X.end());
			Y_all.insert(Y_all.end(), Y.begin(), Y.end());
		}
		total_length = total_length + L;
		for (int j=1; j< n_clothoid; j++){
				S_all.push_back(S_all[j-1+i*(n_clothoid-1)]+L/(n_clothoid-1));
		}

	}

	tk::spline ref_path_x, ref_path_y;
	ref_path_x.set_points(S_all, X_all);
	ref_path_y.set_points(S_all, Y_all);


	dist_spline_pts = total_length / n_points_spline;
	vector<double> ss(80),xx(80),yy(80);

	for (int i=0; i<n_points_spline; i++){
		ss[i] = dist_spline_pts *i;
		xx[i] = ref_path_x(ss[i]);
		yy[i] = ref_path_y(ss[i]);
	}
	ref_path_x.set_points(ss,xx);
	ref_path_y.set_points(ss,yy);

	vector<tk::spline> ref_path_(2);
	ref_path_[0] = ref_path_x;
	ref_path_[1] = ref_path_y;

	return ref_path_;
}



MPMPC::MPMPC(){

    /*=====================================
	Build spline path for the ego-vehicle
	=======================================*/
	vector<double> x_R = {27, 27, 17, -200};
	vector<double> y_R = {-12, -2, 8, 8};
	vector<double> theta_R = {PI/2, PI/2, PI, PI};
	ref_path_R = Ref_path(x_R, y_R, theta_R, dist_spline_pts);


	/*============================================
	Initialization of parameters
	=============================================*/
	double model_ub[9] = {+5.0,  +30.0, 1000, +500,   200,    +2.0*PI,     +1.0,    +16.0,  800};
	double model_lb[9] = {-5.0,  -30.0,   0, -500,   -200,    -2.0*PI,     -1.0,    +0.3,   0 };
	for (int i = 0; i<9; i++){
		mpcparams.x0[i] = (model_ub[i]+model_lb[i])/2;
	}
	mpcparams.x0[2] = 0; //initial value of slack
	mpcparams.x0[3] = 27; // initial x position of ego-vehicle
	mpcparams.x0[4] = -12; //initial y position of ego-vehicel
	mpcparams.x0[5] = PI/2; // initial orientation of ego-vehicle
	mpcparams.x0[6] = 0;
	mpcparams.x0[7] = 0;
	mpcparams.x0[8] = 0;

	for (int i=9; i<442; i=i+9){
		mpcparams.x0[i] = mpcparams.x0[0];
		mpcparams.x0[i+1] = mpcparams.x0[1];
		mpcparams.x0[i+2] = mpcparams.x0[2];
		mpcparams.x0[i+3] = mpcparams.x0[3];
		mpcparams.x0[i+4] = mpcparams.x0[4];
		mpcparams.x0[i+5] = mpcparams.x0[5];
		mpcparams.x0[i+6] = mpcparams.x0[6];
		mpcparams.x0[i+7] = mpcparams.x0[7];
		mpcparams.x0[i+8] = mpcparams.x0[8];
	}


	mpcparams.xinit[0] = initi_ego_x;
	mpcparams.xinit[1] = initi_ego_y;
	mpcparams.xinit[2] = PI/2;
	mpcparams.xinit[3] = 0;
	mpcparams.xinit[4] = initi_ego_v;
	mpcparams.xinit[5] = 0;


	for (int i=0; i<75000;i++)
		mpcparams.all_parameters[i] = 0;


	for (int i=0; i<50; i++){
		if (i<40)
			dt[i] = 0.1;
		else
			dt[i] = 0.2;
	}


	time[0] = 0;
	for (int i=1; i<50; i++){
		time[i] = time[i-1] + dt[i-1];
	}
	for (int i=0; i<50; i++){

		int k = i*1500;
		double end_fac;
		if (i > 40)
            end_fac = 1;
        else
            end_fac = 0;

        double h_factor;
        if (i <= 5)
            h_factor = 1 - i/5;
        else
            h_factor = 0;

		mpcparams.all_parameters[k] = dt[i];
		mpcparams.all_parameters[k+1] = 80;
		mpcparams.all_parameters[k+2] = dist_spline_pts;
		mpcparams.all_parameters[k+3] = 0;
		mpcparams.all_parameters[k+19] = 0.6;
		mpcparams.all_parameters[k+20] = 5.0;
		mpcparams.all_parameters[k+21] = 4.0;
		mpcparams.all_parameters[k+22] = 50.0;
		mpcparams.all_parameters[k+23] = 1.5*PI;
		mpcparams.all_parameters[k+24] = 0.4;
		mpcparams.all_parameters[k+25] = 13.0;
		mpcparams.all_parameters[k+26] = 0.3;

		mpcparams.all_parameters[k+41] = h_factor;
		mpcparams.all_parameters[k+42] = 1;
		mpcparams.all_parameters[k+43] = end_fac;

		mpcparams.all_parameters[k+59] = Length;
		mpcparams.all_parameters[k+60] = Width;
		mpcparams.all_parameters[k+61] = radius_disks;

	}



}

vector<double> MPMPC::Uncertainty(double time, double weight){
	double sigma_s0 = 0.01;
	double sigma_s_delta = 0.05 ;
	double sigma_s_max = 3;
	double sigma_d0 = 0.001;
	double sigma_d_delta = 0.01;
	double sigma_d_max = 0.4;
	double p_thresh = 0.01;
	vector<double> uncertainty(2);
	double a_uncertainty, b_uncertainty;

	double sigma_s = min(sigma_s0 + sigma_s_delta*time, sigma_s_max);
	double sigma_d = min(sigma_d0 + sigma_d_delta*time, sigma_d_max);



	if (1/(2*PI*sigma_s*sigma_d) >  p_thresh / weight && sigma_s*sigma_d > sqrt(0.002)){
		a_uncertainty = sigma_s* sqrt((log(p_thresh/weight) + log(2*PI*sigma_s*sigma_d))*-2);
        b_uncertainty = sigma_d* sqrt((log(p_thresh/weight) + log(2*PI*sigma_s*sigma_d))*-2);
    }
    else{
        a_uncertainty = 0;
        b_uncertainty = 0;
    }
    uncertainty[0] = a_uncertainty;
    uncertainty[1] = b_uncertainty;
    return uncertainty;
}


void MPMPC::MPCUpdateParams(vector<double> state_R_, vector<despot::Traj> est_traj_A_, vector<double> beliefs){
    // update initial guess for the solver
    if (Flag){
    std::copy(mpcoutput.x02, mpcoutput.x02+9, mpcparams.x0);
    std::copy(mpcoutput.x03, mpcoutput.x03+9, mpcparams.x0+9);
    std::copy(mpcoutput.x04, mpcoutput.x04+9, mpcparams.x0+18);
    std::copy(mpcoutput.x05, mpcoutput.x05+9, mpcparams.x0+27);
    std::copy(mpcoutput.x06, mpcoutput.x06+9, mpcparams.x0+36);
    std::copy(mpcoutput.x07, mpcoutput.x07+9, mpcparams.x0+45);
    std::copy(mpcoutput.x08, mpcoutput.x08+9, mpcparams.x0+54);
    std::copy(mpcoutput.x09, mpcoutput.x09+9, mpcparams.x0+63);
    std::copy(mpcoutput.x10, mpcoutput.x10+9, mpcparams.x0+72);
    std::copy(mpcoutput.x11, mpcoutput.x11+9, mpcparams.x0+81);
    std::copy(mpcoutput.x12, mpcoutput.x12+9, mpcparams.x0+90);
    std::copy(mpcoutput.x13, mpcoutput.x13+9, mpcparams.x0+99);
    std::copy(mpcoutput.x14, mpcoutput.x14+9, mpcparams.x0+108);
    std::copy(mpcoutput.x15, mpcoutput.x15+9, mpcparams.x0+9*13);
    std::copy(mpcoutput.x16, mpcoutput.x16+9, mpcparams.x0+9*14);
    std::copy(mpcoutput.x17, mpcoutput.x17+9, mpcparams.x0+9*15);
    std::copy(mpcoutput.x18, mpcoutput.x18+9, mpcparams.x0+9*16);
    std::copy(mpcoutput.x19, mpcoutput.x19+9, mpcparams.x0+9*17);
    std::copy(mpcoutput.x20, mpcoutput.x20+9, mpcparams.x0+9*18);
    std::copy(mpcoutput.x21, mpcoutput.x21+9, mpcparams.x0+9*19);
    std::copy(mpcoutput.x22, mpcoutput.x22+9, mpcparams.x0+9*20);
    std::copy(mpcoutput.x23, mpcoutput.x23+9, mpcparams.x0+9*21);
    std::copy(mpcoutput.x24, mpcoutput.x24+9, mpcparams.x0+9*22);
    std::copy(mpcoutput.x25, mpcoutput.x25+9, mpcparams.x0+9*23);
    std::copy(mpcoutput.x26, mpcoutput.x26+9, mpcparams.x0+9*24);
    std::copy(mpcoutput.x27, mpcoutput.x27+9, mpcparams.x0+9*25);
    std::copy(mpcoutput.x28, mpcoutput.x28+9, mpcparams.x0+9*26);
    std::copy(mpcoutput.x29, mpcoutput.x29+9, mpcparams.x0+9*27);
    std::copy(mpcoutput.x30, mpcoutput.x30+9, mpcparams.x0+9*28);
    std::copy(mpcoutput.x31, mpcoutput.x31+9, mpcparams.x0+9*29);
    std::copy(mpcoutput.x32, mpcoutput.x32+9, mpcparams.x0+9*30);
    std::copy(mpcoutput.x33, mpcoutput.x33+9, mpcparams.x0+9*31);
    std::copy(mpcoutput.x34, mpcoutput.x34+9, mpcparams.x0+9*32);
    std::copy(mpcoutput.x35, mpcoutput.x35+9, mpcparams.x0+9*33);
    std::copy(mpcoutput.x36, mpcoutput.x36+9, mpcparams.x0+9*34);
    std::copy(mpcoutput.x37, mpcoutput.x37+9, mpcparams.x0+9*35);
    std::copy(mpcoutput.x38, mpcoutput.x38+9, mpcparams.x0+9*36);
    std::copy(mpcoutput.x39, mpcoutput.x39+9, mpcparams.x0+9*37);
    std::copy(mpcoutput.x40, mpcoutput.x40+9, mpcparams.x0+9*38);
    std::copy(mpcoutput.x41, mpcoutput.x41+9, mpcparams.x0+9*39);
    std::copy(mpcoutput.x42, mpcoutput.x42+9, mpcparams.x0+9*40);
    std::copy(mpcoutput.x43, mpcoutput.x43+9, mpcparams.x0+9*41);
    std::copy(mpcoutput.x44, mpcoutput.x44+9, mpcparams.x0+9*42);
    std::copy(mpcoutput.x45, mpcoutput.x45+9, mpcparams.x0+9*43);
    std::copy(mpcoutput.x46, mpcoutput.x46+9, mpcparams.x0+9*44);
    std::copy(mpcoutput.x47, mpcoutput.x47+9, mpcparams.x0+9*45);
    std::copy(mpcoutput.x48, mpcoutput.x48+9, mpcparams.x0+9*46);
    std::copy(mpcoutput.x49, mpcoutput.x49+9, mpcparams.x0+9*47);
    std::copy(mpcoutput.x50, mpcoutput.x50+9, mpcparams.x0+9*48);
    std::copy(mpcoutput.x50, mpcoutput.x50+9, mpcparams.x0+9*49);




    }

    double break_index = floor(mpcoutput.x02[8] / dist_spline_pts);

    int spline_index = 500;
    int k;
    for (int i=0; i<50; i++){
        k = i*1500;
        // estimated obstacle vehicles' future trajectories
        for (int q=1; q<=obs_veh_num; q++){
        for(int j=1; j<=2; j++){
            vector<double> uncertainty = Uncertainty(time[i], 0.5);
            //cout << "Pass unc" << endl;
            mpcparams.all_parameters[k+98+20*q-19 + (j-1)*8] = sqrt(2)/2*Length + radius_disks + uncertainty[0];
            mpcparams.all_parameters[k+98+20*q-18 + (j-1)*8] = sqrt(2)/2*Width + radius_disks + uncertainty[1];
            //cout << "Pass 2"<< endl;
            mpcparams.all_parameters[k+98+20*q-17 + (j-1)*8] = est_traj_A_[j-1].x[i];
            //cout << "Pass 3" << endl;
            mpcparams.all_parameters[k+98+20*q-16 + (j-1)*8] = est_traj_A_[j-1].y[i];
            vector<double> orientation = Rotation(est_traj_A_[j-1].theta[i]);
            mpcparams.all_parameters[k+98+20*q-15 + (j-1)*8] = orientation[0];
            mpcparams.all_parameters[k+98+20*q-14 + (j-1)*8] = orientation[1];
            mpcparams.all_parameters[k+98+20*q-13 + (j-1)*8] = orientation[2];
            mpcparams.all_parameters[k+98+20*q-12 + (j-1)*8] = orientation[3];
        }
    }
        // Reference path for the ego-vehicle
        for (int j=1; j<=N_SPLINE_POINTS-1;j++){
            mpcparams.all_parameters[k + spline_index + (j-1)*8] = ref_path_R[0].m_a[break_index+j-1];
            mpcparams.all_parameters[k + spline_index + (j-1)*8 + 1] = ref_path_R[0].m_b[break_index+j-1];
            mpcparams.all_parameters[k + spline_index + (j-1)*8 + 2] = ref_path_R[0].m_c[break_index+j-1];
            mpcparams.all_parameters[k + spline_index + (j-1)*8 + 3] = ref_path_R[0].m_d[break_index+j-1];
            mpcparams.all_parameters[k + spline_index + (j-1)*8 + 4] = ref_path_R[1].m_a[break_index+j-1];
            mpcparams.all_parameters[k + spline_index + (j-1)*8 + 5] = ref_path_R[1].m_b[break_index+j-1];
            mpcparams.all_parameters[k + spline_index + (j-1)*8 + 6] = ref_path_R[1].m_c[break_index+j-1];
            mpcparams.all_parameters[k + spline_index + (j-1)*8 + 7] = ref_path_R[1].m_d[break_index+j-1];
        }
    }

    // Update initial state of ego-vehicle
    mpcparams.xinit[0] = state_R_[0];
	mpcparams.xinit[1] = state_R_[1];
	mpcparams.xinit[2] = state_R_[2];
	mpcparams.xinit[3] = state_R_[3];
	mpcparams.xinit[4] = state_R_[4];
	mpcparams.xinit[5] = state_R_[5];

	Flag = true;

	//for (int i=0; i<500; i++)
      //  cout << mpcparams.all_parameters[i] << endl;

}

void MPMPC::MPCSolver(vector<double>& state, despot::Traj& traj_R){
    // Calling the FORCESPro solver
    int exitflag = FORCESNLPsolver_solve(&mpcparams, &mpcoutput, &mpcinfo, stdout, pt2Function);
    if (exitflag == 1){
		cout << "Successful Planning :)"<< endl;
        // Next state
        state.clear();
        state.push_back(mpcoutput.x02[3]);
        state.push_back(mpcoutput.x02[4]);
        state.push_back(mpcoutput.x02[5]);
        state.push_back(mpcoutput.x02[6]);
        state.push_back(mpcoutput.x02[7]);
        state.push_back(mpcoutput.x02[8]);

        //Policy of ego vehicle
        traj_R.x.clear();
        traj_R.y.clear();
        traj_R.theta.clear();

        traj_R.x.push_back(mpcoutput.x01[3]);
        traj_R.x.push_back(mpcoutput.x02[3]);
        traj_R.x.push_back(mpcoutput.x03[3]);
        traj_R.x.push_back(mpcoutput.x04[3]);
        traj_R.x.push_back(mpcoutput.x05[3]);
        traj_R.x.push_back(mpcoutput.x06[3]);
        traj_R.x.push_back(mpcoutput.x07[3]);
        traj_R.x.push_back(mpcoutput.x08[3]);
        traj_R.x.push_back(mpcoutput.x09[3]);
        traj_R.x.push_back(mpcoutput.x10[3]);
        traj_R.x.push_back(mpcoutput.x11[3]);
        traj_R.x.push_back(mpcoutput.x12[3]);
        traj_R.x.push_back(mpcoutput.x13[3]);
        traj_R.x.push_back(mpcoutput.x14[3]);
        traj_R.x.push_back(mpcoutput.x15[3]);
        traj_R.x.push_back(mpcoutput.x16[3]);
        traj_R.x.push_back(mpcoutput.x17[3]);
        traj_R.x.push_back(mpcoutput.x18[3]);
        traj_R.x.push_back(mpcoutput.x19[3]);
        traj_R.x.push_back(mpcoutput.x20[3]);
        traj_R.x.push_back(mpcoutput.x21[3]);
        traj_R.x.push_back(mpcoutput.x22[3]);
        traj_R.x.push_back(mpcoutput.x23[3]);
        traj_R.x.push_back(mpcoutput.x24[3]);
        traj_R.x.push_back(mpcoutput.x25[3]);
        traj_R.x.push_back(mpcoutput.x26[3]);
        traj_R.x.push_back(mpcoutput.x27[3]);
        traj_R.x.push_back(mpcoutput.x28[3]);
        traj_R.x.push_back(mpcoutput.x29[3]);
        traj_R.x.push_back(mpcoutput.x30[3]);
        traj_R.x.push_back(mpcoutput.x31[3]);
        traj_R.x.push_back(mpcoutput.x32[3]);
        traj_R.x.push_back(mpcoutput.x33[3]);
        traj_R.x.push_back(mpcoutput.x34[3]);
        traj_R.x.push_back(mpcoutput.x35[3]);
        traj_R.x.push_back(mpcoutput.x36[3]);
        traj_R.x.push_back(mpcoutput.x37[3]);
        traj_R.x.push_back(mpcoutput.x38[3]);
        traj_R.x.push_back(mpcoutput.x39[3]);
        traj_R.x.push_back(mpcoutput.x40[3]);
        traj_R.x.push_back(mpcoutput.x41[3]);
        traj_R.x.push_back(mpcoutput.x42[3]);
        traj_R.x.push_back(mpcoutput.x43[3]);
        traj_R.x.push_back(mpcoutput.x44[3]);
        traj_R.x.push_back(mpcoutput.x45[3]);
        traj_R.x.push_back(mpcoutput.x46[3]);
        traj_R.x.push_back(mpcoutput.x47[3]);
        traj_R.x.push_back(mpcoutput.x48[3]);
        traj_R.x.push_back(mpcoutput.x49[3]);
        traj_R.x.push_back(mpcoutput.x50[3]);

        traj_R.y.push_back(mpcoutput.x01[4]);
        traj_R.y.push_back(mpcoutput.x02[4]);
        traj_R.y.push_back(mpcoutput.x03[4]);
        traj_R.y.push_back(mpcoutput.x04[4]);
        traj_R.y.push_back(mpcoutput.x05[4]);
        traj_R.y.push_back(mpcoutput.x06[4]);
        traj_R.y.push_back(mpcoutput.x07[4]);
        traj_R.y.push_back(mpcoutput.x08[4]);
        traj_R.y.push_back(mpcoutput.x09[4]);
        traj_R.y.push_back(mpcoutput.x10[4]);
        traj_R.y.push_back(mpcoutput.x11[4]);
        traj_R.y.push_back(mpcoutput.x12[4]);
        traj_R.y.push_back(mpcoutput.x13[4]);
        traj_R.y.push_back(mpcoutput.x14[4]);
        traj_R.y.push_back(mpcoutput.x15[4]);
        traj_R.y.push_back(mpcoutput.x16[4]);
        traj_R.y.push_back(mpcoutput.x17[4]);
        traj_R.y.push_back(mpcoutput.x18[4]);
        traj_R.y.push_back(mpcoutput.x19[4]);
        traj_R.y.push_back(mpcoutput.x20[4]);
        traj_R.y.push_back(mpcoutput.x21[4]);
        traj_R.y.push_back(mpcoutput.x22[4]);
        traj_R.y.push_back(mpcoutput.x23[4]);
        traj_R.y.push_back(mpcoutput.x24[4]);
        traj_R.y.push_back(mpcoutput.x25[4]);
        traj_R.y.push_back(mpcoutput.x26[4]);
        traj_R.y.push_back(mpcoutput.x27[4]);
        traj_R.y.push_back(mpcoutput.x28[4]);
        traj_R.y.push_back(mpcoutput.x29[4]);
        traj_R.y.push_back(mpcoutput.x30[4]);
        traj_R.y.push_back(mpcoutput.x31[4]);
        traj_R.y.push_back(mpcoutput.x32[4]);
        traj_R.y.push_back(mpcoutput.x33[4]);
        traj_R.y.push_back(mpcoutput.x34[4]);
        traj_R.y.push_back(mpcoutput.x35[4]);
        traj_R.y.push_back(mpcoutput.x36[4]);
        traj_R.y.push_back(mpcoutput.x37[4]);
        traj_R.y.push_back(mpcoutput.x38[4]);
        traj_R.y.push_back(mpcoutput.x39[4]);
        traj_R.y.push_back(mpcoutput.x40[4]);
        traj_R.y.push_back(mpcoutput.x41[4]);
        traj_R.y.push_back(mpcoutput.x42[4]);
        traj_R.y.push_back(mpcoutput.x43[4]);
        traj_R.y.push_back(mpcoutput.x44[4]);
        traj_R.y.push_back(mpcoutput.x45[4]);
        traj_R.y.push_back(mpcoutput.x46[4]);
        traj_R.y.push_back(mpcoutput.x47[4]);
        traj_R.y.push_back(mpcoutput.x48[4]);
        traj_R.y.push_back(mpcoutput.x49[4]);
        traj_R.y.push_back(mpcoutput.x50[4]);

        traj_R.theta.push_back(mpcoutput.x01[5]);
        traj_R.theta.push_back(mpcoutput.x02[5]);
        traj_R.theta.push_back(mpcoutput.x03[5]);
        traj_R.theta.push_back(mpcoutput.x04[5]);
        traj_R.theta.push_back(mpcoutput.x05[5]);
        traj_R.theta.push_back(mpcoutput.x06[5]);
        traj_R.theta.push_back(mpcoutput.x07[5]);
        traj_R.theta.push_back(mpcoutput.x08[5]);
        traj_R.theta.push_back(mpcoutput.x09[5]);
        traj_R.theta.push_back(mpcoutput.x10[5]);
        traj_R.theta.push_back(mpcoutput.x11[5]);
        traj_R.theta.push_back(mpcoutput.x12[5]);
        traj_R.theta.push_back(mpcoutput.x13[5]);
        traj_R.theta.push_back(mpcoutput.x14[5]);
        traj_R.theta.push_back(mpcoutput.x15[5]);
        traj_R.theta.push_back(mpcoutput.x16[5]);
        traj_R.theta.push_back(mpcoutput.x17[5]);
        traj_R.theta.push_back(mpcoutput.x18[5]);
        traj_R.theta.push_back(mpcoutput.x19[5]);
        traj_R.theta.push_back(mpcoutput.x20[5]);
        traj_R.theta.push_back(mpcoutput.x21[5]);
        traj_R.theta.push_back(mpcoutput.x22[5]);
        traj_R.theta.push_back(mpcoutput.x23[5]);
        traj_R.theta.push_back(mpcoutput.x24[5]);
        traj_R.theta.push_back(mpcoutput.x25[5]);
        traj_R.theta.push_back(mpcoutput.x26[5]);
        traj_R.theta.push_back(mpcoutput.x27[5]);
        traj_R.theta.push_back(mpcoutput.x28[5]);
        traj_R.theta.push_back(mpcoutput.x29[5]);
        traj_R.theta.push_back(mpcoutput.x30[5]);
        traj_R.theta.push_back(mpcoutput.x31[5]);
        traj_R.theta.push_back(mpcoutput.x32[5]);
        traj_R.theta.push_back(mpcoutput.x33[5]);
        traj_R.theta.push_back(mpcoutput.x34[5]);
        traj_R.theta.push_back(mpcoutput.x35[5]);
        traj_R.theta.push_back(mpcoutput.x36[5]);
        traj_R.theta.push_back(mpcoutput.x37[5]);
        traj_R.theta.push_back(mpcoutput.x38[5]);
        traj_R.theta.push_back(mpcoutput.x39[5]);
        traj_R.theta.push_back(mpcoutput.x40[5]);
        traj_R.theta.push_back(mpcoutput.x41[5]);
        traj_R.theta.push_back(mpcoutput.x42[5]);
        traj_R.theta.push_back(mpcoutput.x43[5]);
        traj_R.theta.push_back(mpcoutput.x44[5]);
        traj_R.theta.push_back(mpcoutput.x45[5]);
        traj_R.theta.push_back(mpcoutput.x46[5]);
        traj_R.theta.push_back(mpcoutput.x47[5]);
        traj_R.theta.push_back(mpcoutput.x48[5]);
        traj_R.theta.push_back(mpcoutput.x49[5]);
        traj_R.theta.push_back(mpcoutput.x50[5]);
	}
	else
		cout << "Failed Planning :(" << endl;
}



namespace despot{
POMDP::POMDP(){}

void POMDP::InitializeDefaultParameters(){
    Globals::config.num_scenarios = 20;
    Globals::config.time_per_move = 1.0/4;
    Globals::config.search_depth = 20;
}

DSPOMDP* POMDP::InitializeModel(option::Option* options){
    DSPOMDP* model  = new POMDP_Plan();
    return model;
}

void POMDP::OptionParse(option::Option* options, int& num_runs,
                   std::string& simulator_type, std::string& belief_type, int& time_limit,
                   std::string& solver_type, bool& search_solver){
  if (options[E_SILENCE])
    Globals::config.silence = true;

  if (options[E_DEPTH])
    Globals::config.search_depth = atoi(options[E_DEPTH].arg);

  if (options[E_DISCOUNT])
    Globals::config.discount = atof(options[E_DISCOUNT].arg);

  if (options[E_SEED])
    Globals::config.root_seed = atoi(options[E_SEED].arg);
  else { // last 9 digits of current time in milli second
    long millis = (long)get_time_second() * 1000;
    long range = (long)pow((double)10, (int)9);
    Globals::config.root_seed =
        (unsigned int)(millis - (millis / range) * range);
  }

  if (options[E_TIMEOUT])
    Globals::config.time_per_move = atof(options[E_TIMEOUT].arg);

  if (options[E_NUMPARTICLES])
    Globals::config.num_scenarios = atoi(options[E_NUMPARTICLES].arg);

  if (options[E_PRUNE])
    Globals::config.pruning_constant = atof(options[E_PRUNE].arg);

  if (options[E_GAP])
    Globals::config.xi = atof(options[E_GAP].arg);

  if (options[E_SIM_LEN])
    Globals::config.sim_len = atoi(options[E_SIM_LEN].arg);

  if (options[E_EVALUATOR])
    simulator_type = options[E_EVALUATOR].arg;

  if (options[E_MAX_POLICY_SIM_LEN])
    Globals::config.max_policy_sim_len =
        atoi(options[E_MAX_POLICY_SIM_LEN].arg);

  if (options[E_DEFAULT_ACTION])
    Globals::config.default_action = options[E_DEFAULT_ACTION].arg;

  if (options[E_RUNS])
    num_runs = atoi(options[E_RUNS].arg);

  if (options[E_BELIEF])
    belief_type = options[E_BELIEF].arg;

  if (options[E_TIME_LIMIT])
    time_limit = atoi(options[E_TIME_LIMIT].arg);

  if (options[E_NOISE])
    Globals::config.noise = atof(options[E_NOISE].arg);

  search_solver = options[E_SEARCH_SOLVER];

  if (options[E_SOLVER])
    solver_type = options[E_SOLVER].arg;

  int verbosity = 0;
  if (options[E_VERBOSITY])
    verbosity = atoi(options[E_VERBOSITY].arg);
  logging::level(verbosity);

}


Solver* POMDP::InitializeSolver(DSPOMDP *model, string solver_type,
                                    option::Option *options) {

  // DESPOT or its default policy
  if (solver_type == "DESPOT" ||
      solver_type == "PLB") // PLB: particle lower bound
  {
    string blbtype = options[E_BLBTYPE] ? options[E_BLBTYPE].arg : "DEFAULT";
    string lbtype = options[E_LBTYPE] ? options[E_LBTYPE].arg : "DEFAULT";
    ScenarioLowerBound *lower_bound =
        model->CreateScenarioLowerBound(lbtype, blbtype);

    logi << "Created lower bound " << typeid(*lower_bound).name() << endl;

    if (solver_type == "DESPOT") {
      string bubtype = options[E_BUBTYPE] ? options[E_BUBTYPE].arg : "DEFAULT";
      string ubtype = options[E_UBTYPE] ? options[E_UBTYPE].arg : "DEFAULT";
      ScenarioUpperBound *upper_bound =
          model->CreateScenarioUpperBound(ubtype, bubtype);

      logi << "Created upper bound " << typeid(*upper_bound).name() << endl;

      solver = new DESPOT(model, lower_bound, upper_bound);
    } else
      solver = lower_bound;
  } // AEMS or its default policy
  else if (solver_type == "AEMS" || solver_type == "BLB") {
    string lbtype = options[E_LBTYPE] ? options[E_LBTYPE].arg : "DEFAULT";
    BeliefLowerBound *lower_bound =
        static_cast<BeliefMDP *>(model)->CreateBeliefLowerBound(lbtype);

    logi << "Created lower bound " << typeid(*lower_bound).name() << endl;

    if (solver_type == "AEMS") {
      string ubtype = options[E_UBTYPE] ? options[E_UBTYPE].arg : "DEFAULT";
      BeliefUpperBound *upper_bound =
          static_cast<BeliefMDP *>(model)->CreateBeliefUpperBound(ubtype);

      logi << "Created upper bound " << typeid(*upper_bound).name() << endl;

      solver = new AEMS(model, lower_bound, upper_bound);
    } else
      solver = lower_bound;
  } // POMCP or DPOMCP
  else if (solver_type == "POMCP" || solver_type == "DPOMCP") {
    string ptype = options[E_PRIOR] ? options[E_PRIOR].arg : "DEFAULT";
    POMCPPrior *prior = model->CreatePOMCPPrior(ptype);

    logi << "Created POMCP prior " << typeid(*prior).name() << endl;

    if (options[E_PRUNE]) {
      prior->exploration_constant(Globals::config.pruning_constant);
    }

    if (solver_type == "POMCP")
      solver = new POMCP(model, prior);
    else
      solver = new DPOMCP(model, prior);
  } else { // Unsupported solver
    cerr << "ERROR: Unsupported solver type: " << solver_type << endl;
    exit(1);
  }
  return solver;
}

void POMDP::DisplayParameters(option::Option *options, DSPOMDP *model) {

  string lbtype = options[E_LBTYPE] ? options[E_LBTYPE].arg : "DEFAULT";
  string ubtype = options[E_UBTYPE] ? options[E_UBTYPE].arg : "DEFAULT";
  default_out << "Model = " << typeid(*model).name() << endl
              << "Random root seed = " << Globals::config.root_seed << endl
              << "Search depth = " << Globals::config.search_depth << endl
              << "Discount = " << Globals::config.discount << endl
              << "driving_simulator steps = " << Globals::config.sim_len << endl
              << "Number of scenarios = " << Globals::config.num_scenarios
              << endl
              << "Search time per step = " << Globals::config.time_per_move
              << endl
              << "Regularization constant = "
              << Globals::config.pruning_constant << endl
              << "Lower bound = " << lbtype << endl
              << "Upper bound = " << ubtype << endl
              << "Policy simulation depth = "
              << Globals::config.max_policy_sim_len << endl
              << "Target gap ratio = " << Globals::config.xi << endl;
  // << "Solver = " << typeid(*solver).name() << endl << endl;
}

void POMDP::Initialization(int argc, char* argv[]){
  clock_t main_clock_start = clock();
  EvalLog::curr_inst_start_time = get_time_second();

  const char *program = (argc > 0) ? argv[0] : "despot";

  argc -= (argc > 0);
  argv += (argc > 0); // skip program name argv[0] if present

  option::Stats stats(usage, argc, argv);
  option::Option *options = new option::Option[stats.options_max];
  option::Option *buffer = new option::Option[stats.buffer_max];
  option::Parser parse(usage, argc, argv, options, buffer);

  string solver_type = "DESPOT";
  bool search_solver;

  /* =========================
   * Parse required parameters
   * =========================*/
  int num_runs = 1;
  string simulator_type = "pomdp";
  string belief_type = "DEFAULT";
  int time_limit = -1;

  /* =========================================
   * Problem specific default parameter values
*=========================================*/
  InitializeDefaultParameters();

  /* =========================
   * Parse optional parameters
   * =========================*/
  if (options[E_HELP]) {
    cout << "Usage: " << program << " [options]" << endl;
    option::printUsage(std::cout, usage);
  }
  OptionParse(options, num_runs, simulator_type, belief_type, time_limit,
              solver_type, search_solver);

  /* =========================
   * Global random generator
   * =========================*/
  Seeds::root_seed(Globals::config.root_seed);
  unsigned world_seed = Seeds::Next();
  unsigned seed = Seeds::Next();
  Random::RANDOM = Random(seed);

  /* =========================
   * initialize model
   * =========================*/
  model = InitializeModel(options);

   /* initialize solver
   * =========================*/
  solver = InitializeSolver(model, solver_type, options);
  assert(solver != NULL);


  /* =========================
   * Display parameters
   * =========================*/
  DisplayParameters(options, model);


    // Initial state
	State* state = model->CreateStartState();

   /* =========================
      Initial belief
      =========================*/

	double start_t = get_time_second();
	delete solver->belief();
	double end_t = get_time_second();

	Belief* belief = model->InitialBelief(state, belief_type);

	solver->belief(belief);
}

void POMDP::ReconstructPolicy(vector<int> policyStar, vector<int> depthOrder, vector<int>& policy0, vector<int>& policy1){
    policy0.push_back(policyStar[0]);
    int i;
    for (i=1; i<depthOrder.size(); i++){
        if (depthOrder[i] > depthOrder[i-1]){
            policy0.push_back(policyStar[i]);
        }
        else
            break;
    }


    policy1 = policy0;
    for (;i < depthOrder.size(); i++){
        if (depthOrder[i] < policy1.size()){
            policy1[depthOrder[i]] = policyStar[i];
        }
        else{
            policy1.push_back(policyStar[i]);
            policy0.push_back(policyStar[i]);
        }
    }

    policy0.insert(policy0.end(), 50-policy0.size(), 2);
    policy1.insert(policy1.end(), 50-policy1.size(), 2);
}

int POMDP::POMDP_Solver(vector<double>& state_A, Traj& traj0, Traj& traj1, Traj traj_R){
    // Update the policy of ego-vehicle
    model->traj_R = traj_R;


    double start_t = despot::get_time_second();
    int action = solver->Search().action;
    policyStar = solver->policyStar;
    depthOrder = solver->depthOrder;
    goal_probs = solver->goal_probs;
    double end_t = despot::get_time_second();

    cout << "- Action = ";
		solver->model_->PrintAction(action);


//	if (state_ != NULL) {
//		if (!Globals::config.silence && out_) {
//			*out_ << "- State:\n";
//			model_->PrintState(*state_, *out_);
//		}
//	}
//
//	if (!Globals::config.silence && out_) {
//		*out_ << "- Observation = ";
//		model_->PrintObs(*state_, obs, *out_);
//	}
//
//	if (state_ != NULL) {
//		if (!Globals::config.silence && out_)
//			*out_ << "- ObsProb = " << model_->ObsProb(obs, *state_, action)
//				<< endl;
//	}
    cout << "DESPOT Search time: " << end_t - start_t << endl;

    // Transition from optimal policy to state space
    vector<int> policy0, policy1;
    ReconstructPolicy(policyStar, depthOrder, policy0, policy1);

    POMDP_Plan simulator;

    vector<double> state(6), next_state, state_sim;

    state[0] = state_A[0];
    state[1] = state_A[1];
    state[2] = state_A[2];
    state[3] = state_A[3];
    state[4] = 0;
    state[5] = state_A[5];

    state_sim = state;
    for (int i=0; i<policy0.size(); i++){
        next_state = simulator.Dynamics_A(state_sim, policy0[i]);
        traj0.x.push_back(next_state[0]);
        traj0.y.push_back(next_state[1]);
        traj0.theta.push_back(next_state[2]);
        state_sim = next_state;
    }
    state[4] = 1;
    state_sim = state;
    for (int i=0; i<policy1.size(); i++){
        next_state = simulator.Dynamics_A(state_sim, policy1[i]);
        traj1.x.push_back(next_state[0]);
        traj1.y.push_back(next_state[1]);
        traj1.theta.push_back(next_state[2]);
        state_sim = next_state;
    }

    state_A = simulator.Dynamics_A(state_A, policy0[0]);

    return action;

}

void POMDP::POMDP_Update(int action, vector<double> pos0, vector<double> pos1){
    POMDP_Plan_State next_state(pos0,pos1);
    POMDP_Plan TranslateObs;
    int observation = TranslateObs.MakeObservation(next_state);

    solver->Update(action, observation);

}
}

int main(int argc, char **argv)
{
    // ROS connection
    ros::init(argc, argv, "MotionPlanner");
    ros::NodeHandle n;
    ros::Rate r(10);
    ros::Publisher veh_pub = n.advertise<visualization_msgs::Marker>("visualization_marker", 10);

    uint32_t shape = visualization_msgs::Marker::CUBE;

    visualization_msgs::Marker Ego_veh, Obs_veh;
    Ego_veh.header.frame_id = "/my_frame";

    Obs_veh.header.frame_id = "/my_frame";


    Ego_veh.ns = "ego_vehicle";
    Obs_veh.ns = "obs_vehicle";
    Ego_veh.id = 0;
    Obs_veh.id = 1;
    Ego_veh.type = shape;
    Obs_veh.type = shape;
    Ego_veh.action = visualization_msgs::Marker::ADD;
    Obs_veh.action = visualization_msgs::Marker::ADD;

    Ego_veh.scale.x = 4.0;
    Ego_veh.scale.y = 2.0;
    Ego_veh.scale.z = 1;
    Obs_veh.scale.x = 4.0;
    Obs_veh.scale.y = 2.0;
    Obs_veh.scale.z = 1.0;

    Ego_veh.color.r = 1.0f;
    Ego_veh.color.g = 0.0f;
    Ego_veh.color.b = 0.0f;
    Ego_veh.color.a = 1.0;

    Obs_veh.color.r = 0.0f;
    Obs_veh.color.g = 0.0f;
    Obs_veh.color.b = 1.0f;
    Obs_veh.color.a = 1.0;

    Ego_veh.lifetime = ros::Duration();
    Obs_veh.lifetime = ros::Duration();

    Ego_veh.pose.position.x = initi_ego_x;
    Ego_veh.pose.position.y = initi_ego_y;
    Ego_veh.pose.position.z = 0;
    Ego_veh.pose.orientation.x = 0.0;
    Ego_veh.pose.orientation.y = 0.0;
    Ego_veh.pose.orientation.z = sin(PI/4);
    Ego_veh.pose.orientation.w = cos(PI/4);

    Obs_veh.pose.position.x = initi_obs_x;
    Obs_veh.pose.position.y = initi_obs_y;
    Obs_veh.pose.position.z = 0;
    Obs_veh.pose.orientation.x = 0.0;
    Obs_veh.pose.orientation.y = 0.0;
    Obs_veh.pose.orientation.z = sin(PI/2);
    Obs_veh.pose.orientation.w = cos(PI/2);


    veh_pub.publish(Ego_veh);
    veh_pub.publish(Obs_veh);


    int count = 0;
    MPMPC MPC_Planner;
    despot::POMDP POMDP_Planner;

    vector<double> state_R = {initi_ego_x, initi_ego_y, PI/2, 0, initi_ego_v, 0};//x,y,theta,steer angle,v,s
    vector<double> state_A = {initi_obs_x, initi_obs_y, PI, initi_obs_v, 0, 0};//x,y,theta,v,g,s

    POMDP_Planner.Initialization(argc, argv);

    //Initial trajectory of ego-vehicle
    despot::Traj traj_R;
    double dx, dy, s;
    for (int i=0; i<50; i++){
        s = initi_ego_v*0.1*i;
        traj_R.x.push_back(MPC_Planner.ref_path_R[0](s));
        traj_R.y.push_back(MPC_Planner.ref_path_R[1](s));
        dx = MPC_Planner.ref_path_R[0].deriv(1,s);
        dy = MPC_Planner.ref_path_R[1].deriv(1,s);
        traj_R.theta.push_back(atan2(dy, dx));
    }

    while (ros::ok())
    {
        cout << "--------------------Round " << count << "----------------" << endl;
    /**
     * Anticipation of obstacle vehicles' motion
     */
        despot::Traj traj_A0, traj_A1;
        int action = POMDP_Planner.POMDP_Solver(state_A, traj_A0, traj_A1, traj_R);

        vector<despot::Traj> traj_A;
        traj_A.push_back(traj_A0);
        traj_A.push_back(traj_A1);


    /**
    * Motion planning for ego-vehicle using MPC
    **/
        MPC_Planner.MPCUpdateParams(state_R, traj_A, POMDP_Planner.goal_probs);

        MPC_Planner.MPCSolver(state_R, traj_R);

    /**
    * Update the DESPOT solver
    **/
        POMDP_Planner.POMDP_Update(action, state_R, state_A);

        /**
        * Update Rviz
        **/
        Ego_veh.header.stamp = ros::Time::now();
        Obs_veh.header.stamp = ros::Time::now();

        Ego_veh.pose.position.x = state_R[0];
        Ego_veh.pose.position.y = state_R[1];
        Ego_veh.pose.position.z = 0;
        Ego_veh.pose.orientation.x = 0.0;
        Ego_veh.pose.orientation.y = 0.0;
        Ego_veh.pose.orientation.z = sin(state_R[2]/2);
        Ego_veh.pose.orientation.w = cos(state_R[2]/2);

        Obs_veh.pose.position.x = state_A[0];
        Obs_veh.pose.position.y = state_A[1];
        Obs_veh.pose.position.z = 0;
        Obs_veh.pose.orientation.x = 0.0;
        Obs_veh.pose.orientation.y = 0.0;
        Obs_veh.pose.orientation.z = sin(state_A[2]/2);
        Obs_veh.pose.orientation.w = cos(state_A[2]/2);

        veh_pub.publish(Ego_veh);
        veh_pub.publish(Obs_veh);
        count++;
        r.sleep();
    }
  return 0;
}
