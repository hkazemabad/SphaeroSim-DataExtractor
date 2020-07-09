#pragma once

#include <string>
#include <vector>

using namespace std;

#define temp_cryst 403.15
size_t n_time_point;

string result_folder = "Results_Extract", logfile= result_folder + "\\" + "0.LogFile.txt", 
complete_sim_name = result_folder + "\\" + "1.AllCompleteSim.csv", sim_healthy_name = result_folder + "\\" + 
"2.HealthySim.csv", sim_not_healthy_name = result_folder + "\\" + "3.NotHealthySim.csv", sim_avg_name = 
result_folder + "\\" + "4.AveragedSim.csv", sim_similar_name = result_folder + "\\" + "5.SimilarSim.csv", 
sim_not_similar_name = result_folder + "\\" + "6.NotSimilarSim.csv", databank_name, input_xml = "InputInfo.xml";

bool sim_incomplete, complete_sim, sim_healthy, sim_not_healthy, sim_avg, sim_similar_bool, sim_not_similar_bool,
temperature_history, crystallisation_temperature, number_of_elements_on_edge, number_of_spherulites, 
averaged_spherulites_diameter;


vector<double> time_points;
vector<string> databanks_name, sim_list;
vector <vector<string>> sim_seperated;

void WriteToCsv(vector<string>&, string&, double**);

void ReadInputXml();

void ReadXml(string&, string&, double**, int&);

vector<int> ReadResultsSpherulites(string&);

int ReadResultsStatistics(string&);

int ReadVtk(string&);

string GetVtkList(string);

vector<string> GetXMLandCSVFiles(string&, double**, ofstream& log);

string GetResultFromXmlAndCsv(string&, double**, double&, int&, vector<int>&, int&, int&);

vector<vector<string>> SeperateHealthAndMissingSim(string&);

vector<vector<string>> SeperateSimilarandNotSimilarSims(string&);

vector<string> MakeAvgSim(string&, double**);

void SortSim(vector<string>&);

string Win32FindDataToString(WIN32_FIND_DATA data);

vector<string> SplitTempProfile(const string&, const string& );

void SplitTempPair(double**, const string&, const string&, int&);

double InterlpolationTemp(double&, double&, double&, double&, double&);

double CalCrystTime(double**);

double StringToDouble(const string&);

int StringToInt(const string&);

bool StringToBool(const string&);



