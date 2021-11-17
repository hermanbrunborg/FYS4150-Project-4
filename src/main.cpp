#include "project4/ising_model.hpp"
#include "project4/stat_utils.hpp"

#include <iostream>
#include <vector>
#include <map>
#include <algorithm>
#include <fstream>
#include <omp.h>

using namespace std;

void find_burn_in_time(int N, int L=20, double T=1){
    IsingModel model(L, T, true);
    
    vector<int> sampled_E;
    vector<int> sampled_M;
    for (int i=0; i<N; i++){
        sampled_E[i] = model.get_energy();
        sampled_M[i] = model.get_magnetization();
        model.metropolis();
    }
    vector<int> burn_in_time;
    vector<double> expected_E_burn_in;
    vector<double> expected_M_burn_in;
    for (int burn_in=0; burn_in<N; burn_in+=max(1, N/100)){
        auto add_burn_in = [burn_in](int x){return x + burn_in;};
        map<double, double> buckets_E = stat_utils::distribution(sampled_E, N - burn_in, add_burn_in);
        map<double, double> buckets_M = stat_utils::distribution(sampled_M, N - burn_in, add_burn_in);
        expected_E_burn_in.push_back(stat_utils::expected_value(buckets_E));
        expected_M_burn_in.push_back(stat_utils::expected_value(buckets_M));
        burn_in_time.push_back(burn_in);
    }

    ofstream burn_in_csv;
    burn_in_csv.open("output/burn_in.csv");
    burn_in_csv << "burn_in,expected_E,expected_M\n";
    for (int i=0; i<burn_in_time.size(); i++){
        burn_in_csv << burn_in_time[i] << "," << expected_E_burn_in[i] 
                    << "," << expected_M_burn_in[i] << "\n";
    }
    burn_in_csv.close();
}


void sample(vector<int> &sampled_energy, vector<int> &sampled_magnetization_abs, const int iters, int L, double T, int burn_in_time = 1000){
    IsingModel model(L, T, true);
    for (int i = -burn_in_time; i < iters; i++){
        model.metropolis();
        if (i < 0) continue;
        sampled_energy.push_back(model.get_energy());
        sampled_magnetization_abs.push_back(abs(model.get_magnetization()));
    }
}



/**
 * @brief Produces sampled distribution of &epsilon; and |m|
 * 
 * @param N Numbers of MCMC-iterations
 * @param L Size of the Lattice (will have L * L elements)
 * @param T temperature
 * @param burn_in_time Number of iterations to discard when producing estimates 
 */
void produce_distributions(const int iters, const int L, double T, int burn_in_time = 1000){ // note we don't have any indications on what the burn_in_time is yet
    IsingModel model(L, T, true);
    const int N = L * L;
    vector<int> sampled_energy;
    vector<int> sampled_magnetization_abs;
    sample(sampled_energy, sampled_magnetization_abs, iters, L, T);
    auto scale = [N](int x){return (double)x / N;};
    map<double, double> buckets_epsilon = stat_utils::distribution(sampled_energy, iters, scale);
    map<double, double> buckets_m_abs = stat_utils::distribution(sampled_magnetization_abs, iters, scale);
    }


void values(vector<int> sampled_energy, vector<int> sampled_magnetization_abs, int sample_size, int L, double T, 
double &expected_epsilon, double &expected_m_abs, double &c_v, double &chi){
    int N = L * L;
    auto scale = [N](int x){return (double)x / N;};
    auto square = [](int x){return x * x;};
    expected_epsilon = stat_utils::expected_value(sampled_energy, sample_size, scale);
    expected_m_abs = stat_utils::expected_value(sampled_magnetization_abs, sample_size, scale);
    double expected_energy = stat_utils::expected_value(sampled_energy, sample_size);
    double expected_energy_sq = stat_utils::expected_value(sampled_energy, sample_size, square);
    double expected_magnetization_abs = stat_utils::expected_value(sampled_magnetization_abs, sample_size);
    double expected_magnetization_sq = stat_utils::expected_value(sampled_magnetization_abs, sample_size, square);
    c_v = (1. / N) * (1. / (T * T)) * (expected_energy_sq - expected_energy * expected_energy);
    chi = (1. / N) * (1. / T) * (expected_magnetization_sq - expected_magnetization_abs * expected_magnetization_abs);
}



/**
 * @brief estimates <&epsilon;>, <|m|>, C_v and &chiand writes them to the outfile in that order, all after the temperature
 * @param L problem size
 * @param T temperature
 * @param outfile csv-file to write results
 */
void write_values_to_file(int L, double T, ofstream &outfile){
    int sample_size = 10000;
    vector<int> sampled_energy;
    vector<int> sampled_magnetization_abs;
    sample(sampled_energy, sampled_magnetization_abs, sample_size, L, T, 1000);
    double expected_epsilon, expected_m_abs, c_v, chi;
    values(sampled_energy, sampled_magnetization_abs, sample_size, L, T, expected_epsilon, expected_m_abs, c_v, chi);
    outfile << T << "," << expected_epsilon << "," << expected_m_abs << "," << c_v << "," << chi << endl;
}

int test2x2(){
    const double tol = 1e-2;
    vector<int> sampled_energy;
    vector<int> sampled_magnetization_abs;
    double T = 1.;
    int max_sample_size = 10000;
    sample(sampled_energy, sampled_magnetization_abs, max_sample_size, 2, T, 0);
    double expected_epsilon, expected_m_abs, c_v, chi;
    double analytical_expected_epsilon = -1.99598208593669, analytical_expected_m_abs = 0.9986607327485997, analytical_c_v = 0.032082331864287994, analytical_chi = 0.004010739516227435;
    int using_sample_size = 1;
    do {
        values(sampled_energy, sampled_magnetization_abs, using_sample_size, 2, T, expected_epsilon, expected_m_abs, c_v, chi);
        using_sample_size++;
    } while((abs(expected_epsilon - analytical_expected_epsilon) > tol 
            or abs(expected_m_abs - analytical_expected_m_abs) > tol 
            or abs(c_v - analytical_c_v) > tol 
            or abs(chi - analytical_chi) > tol)
            and using_sample_size < max_sample_size);
    return using_sample_size;
}

int main(){
    cout << test2x2() << endl;
    double T_min = 2.1;
    int steps = 10;
    double dT = (2.4 - T_min) / steps;
    ofstream outfile("output/values_L=40.csv");
    // #pragma omp parallel for
    for (int i = 0; i < steps; i++){
        double T = T_min + i * dT;
        write_values_to_file(20, T, outfile);
    }
    outfile.close();
    return 0;
}
