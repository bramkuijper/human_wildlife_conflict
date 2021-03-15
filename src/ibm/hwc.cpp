#include <iostream>
#include <fstream>
#include <string>
#include <random>
#include <vector>
#include <array>
#include <cstdlib>
#include <algorithm>

// set up the random number generator 
std::random_device rd;
unsigned seed = rd();
std::mt19937 rng_r(seed);
std::uniform_real_distribution<> uniform(0.0,1.0);

// the filename
std::string filename = "";
int n_generations = 10; // number of generations
const int n_sites = 500;
const int n_deftypes = 3;

std::array<double,n_deftypes> p;

// resources obtained in animal's
// natural environment
double natural_resources = 0.0;

double init_animal_resources = 0.0;
double init_site_resources = 0.0;

const int n_animals = 500;

enum PatchType {
    natural = 0,
    defense1 = 1,
    defense2 = 2
};

struct animal
{
    // evolvable locus, the attack rate of a single site
    double attack_rate[n_deftypes];

    // the amount of resources gained 
    double resources;

};

// individual patches with resources
struct site
{
    double resource;
    PatchType type;
    double investment[n_deftypes];

    std::vector<int> attacked_by;
};

// initialize the meta
site meta_population[n_sites];
animal wildlife[n_animals];


// initialize arguments
void init_arguments(int argc, char **argv)
{
    for (int p_i = 0; p_i < n_deftypes; ++p_i)
    {
        p[p_i] = atof(argv[1 + p_i]);
    }

    int argv_ctr = 1 + n_deftypes;

    init_animal_resources = atof(argv[argv_ctr++]);
    init_site_resources = atof(argv[argv_ctr++]);
    filename = argv[argv_ctr++];

    std::cout << "filename: " << filename << std::endl;
} // end init_arguments

/**
 * write the parameters to the file
 * @param output_file a stream object to which the parameters should be written
 **/
void write_parameters(std::ofstream &output_file)
{
    output_file << std::endl
        << std::endl
        << "seed;" << seed << std::endl
        << "n_sites;" << n_sites << std::endl
        << "n_deftypes;" << n_deftypes << std::endl
        << "n_animals;" << n_animals << std::endl
        << "init_animal_resources;" << init_animal_resources << std::endl
        << "init_site_resources;" << init_site_resources << std::endl;

    for (int p_i = 0; p_i < n_deftypes; ++p_i)
    {
        output_file << "p" << p_i << ";" << p[p_i] << std::endl;
    }
} // void write_parameters()

// initialize the population
void init_population()
{
    // initialize patches according to frequency distribution
    std::discrete_distribution<PatchType> patch_distr(p.begin(),p.end());
    

    for (int site_i = 0; site_i < n_sites; ++site_i)
    {
        meta_population[site_i].resource = init_site_resources;

        meta_population[site_i].type = patch_distr(rng_r);
        std::cout<<meta_population[site_i].attacked_by.size() <<std::endl;
        meta_population[site_i].attacked_by.clear(); 
    } // end for site_i

    for (int animal_i = 0; animal_i < n_animals; ++animal_i)
    {
        for (int deftype_i = 0; deftype_i < n_deftypes; ++deftype_i)
        {
            wildlife[animal_i].attack_rate[deftype_i] = 0.0;
        }

        wildlife[animal_i].resources = init_animal_resources;
    } // end for animal_i
} // end init_population()


// probability wildlife attacks a site
double pr_attack(double const attack_rate
        ,double const defense_rate)
{
    return(1.0/(1.0 - std::exp(-(attack_rate - defense_rate))));
} // end pr_attack

void forage()
{
    int site_chosen;

    PatchType site_type;
    
    // make distribution of all the sites
    std::uniform_int_distribution<int> random_site(0, n_sites - 1);

    //  update attack counters of sites
    for (int site_i = 0; n_sites; ++site_i)
    {
        meta_population[site_i].attacked_by.clear();
    }

    // let animal choose
    for (int animal_i = 0; animal_i < n_animals; ++animal_i)
    {
        site_chosen = random_site(rng_r);
        site_type = meta_population[site_chosen].type;

        if (natural)
        {
            wildlife[animal_i].resources += natural_resources;
        }
        else if (uniform(rng_r) < pr_attack(
                            wildlife[animal_i].attack_rate[site_type]
                            ,meta_population[site_chosen].investment[site_type]))
        {
            meta_population[site_chosen].attacked_by.push_back(animal_i);
        }
    } // end animal_i

    // all attacks done, now do accounting
    // divvy up resources among attackers
    for (int site_i = 0; site_i < n_sites; ++site_i)
    {
        double resources_per_individual = meta_population[site_i].resource / 
            meta_population[site_i].attacked_by.size();

        for (std::vector<int>::iterator it = meta_population[site_i].attacked_by.begin();
                it != meta_population[site_i].attacked_by.end();
                ++it)
        {
            std::cout << *it << std::endl;
            wildlife[*it].resources += resources_per_individual;
        }
    } // end for site i
} // end forage

// write the headers to the data file
void write_data_headers(std::ofstream &output_file)
{
    output_file << "generation;animal_resource;var_animal_resource;";

    // initialize resource counter to 0
    for (int site_type_i = 0; site_type_i < n_deftypes; ++site_type_i)
    {
        output_file << "n_site" << site_type_i << ";"
                    << "resource_site_type" << site_type_i << ";"
                    << "attack_rate" << site_type_i << ";"
                    << "var_attack_rate" << site_type_i << ";";
    }

    output_file << std::endl;
} // end write_data_headers()

// write the data of the simulation to a file
void write_data(std::ofstream &output_file, int const generation)
{
    double resources_site[n_deftypes];
    double mean_attack_rate[n_deftypes];
    double ss_attack_rate[n_deftypes];
    int site_counts[n_deftypes];

    double mean_resources_animal = 0.0;
    double ss_resources_animal = 0.0;

    // initialize resource counter to 0
    for (int site_type_i = 0; site_type_i < n_deftypes; ++site_type_i)
    {
        resources_site[site_type_i] = 0.0;
        site_counts[site_type_i] = 0;
        mean_attack_rate[site_type_i] = 0;
        ss_attack_rate[site_type_i] = 0;
    }

    // summarize resources and type of all sites
    for (int site_i = 0; site_i < n_sites; ++site_i)
    {
        resources_site[meta_population[site_i].type] += 
            meta_population[site_i].resource;

        ++site_counts[meta_population[site_i].type];
    }

    double tmp_trait;

    // summarize animal resources and attack rates
    for (int animal_i = 0; animal_i < n_animals; ++animal_i)
    {
        tmp_trait = wildlife[animal_i].resources;
        mean_resources_animal += tmp_trait;
        ss_resources_animal += tmp_trait * tmp_trait;

        // now calculate attack rates for each of the sites
        for (int site_type_i = 0; site_type_i < n_deftypes; ++site_type_i)
        {
            tmp_trait = wildlife[animal_i].attack_rate[site_type_i];
            mean_attack_rate[site_type_i] += tmp_trait;
            ss_attack_rate[site_type_i] += tmp_trait * tmp_trait;
        }
    }

    mean_resources_animal /= n_animals;

    double var_resources_animal = ss_resources_animal / n_animals - 
        mean_resources_animal * mean_resources_animal;

    // actual output
    output_file << generation << ";" 
                << mean_resources_animal << ";" 
                << var_resources_animal << ";";

    double var_trait;

    // now write the data
    for (int site_type_i = 0; site_type_i < n_deftypes; ++site_type_i)
    {
        mean_attack_rate[site_type_i] /= n_animals;

        var_trait = ss_attack_rate[site_type_i] - 
            mean_attack_rate[site_type_i] * mean_attack_rate[site_type_i];

        output_file << site_counts[site_type_i] << ";"
                        << (site_counts[site_type_i] == 0.0 ? 
                                0.0 : resources_site[site_type_i] / 
                                site_counts[site_type_i]) << ";"
                        << mean_attack_rate[site_type_i] << ";"
                        << var_trait << ";";
    }

    output_file << std::endl;
} // end write_data()

void defense_investment()
{

}

void survive_reproduce()
{
}

int main(int argc, char **argv)
{
    init_arguments(argc, argv);

    std::ofstream DataFile(filename);

    write_data_headers(DataFile);    

    init_population();

    for (int generation_i = 0; generation_i < n_generations; ++generation_i)
    {
        // life cycle
        // farmers invest in defenses
//        defense_investment();

        forage();

 //       survive_reproduce();

        write_data(DataFile, generation_i);
    }

    write_parameters(DataFile);
} //e 
