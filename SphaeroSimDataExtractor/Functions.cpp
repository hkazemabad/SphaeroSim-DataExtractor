// SphaeroSimDataExtractor.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "tinyxml2.h"
#include <string>
#include <vector>
#include <stdio.h>
#include <sstream>
#include <iostream>
#include <fstream>
#include <stdlib.h>
#include <windows.h>
#include <tchar.h>
#include <math.h>
#include <direct.h>
#include <sys\stat.h>
#include <sys\types.h>
#include <time.h>
#include "Functions.h"


using namespace std;
using namespace tinyxml2;


string GetVtkList(string vtk_path)
{
	vector<string> vtk_list;
	string vtk_name_str;
	vtk_path = vtk_path + "/*.*";
	WIN32_FIND_DATA fd;
	wstring vtk_path_temp = wstring(vtk_path.begin(), vtk_path.end());
	LPCWSTR sw = vtk_path_temp.c_str();
	HANDLE hFind = ::FindFirstFile(sw, &fd);
	if (hFind != INVALID_HANDLE_VALUE)
	{
		do {
			// read all (real) files in current folder
			// , delete '!' read other 2 default folder '.' and '..'
			if (!(fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
			{
				vtk_name_str = Win32FindDataToString(fd);
				vtk_list.push_back(vtk_name_str);
			}
		} while (::FindNextFile(hFind, &fd));
		::FindClose(hFind);
	}
	vtk_name_str = vtk_list[vtk_list.size() - 1];
	vtk_list.clear();
	vtk_list.shrink_to_fit();
	return vtk_name_str;
}

//Get all VTK, XML and CSV files from all Simulations in the Databank
vector<string> GetXMLandCSVFiles(string databank_dir,
	double** temp_in_time)
{
	WIN32_FIND_DATA data;
	string file_folder_name, new_databank_dir, sim_dir, inner_folder_dir,
		xml_path, csv_path, re_sph_csv_path, re_sta_csv_path, vtk_path, vtk_file, sim_incomplete = "";
	size_t size_file_folder_name, chck = 0, internal_dir;//chck: check whether all three xml_path, re_sph_csv_path, re_sta_csv_path, vtk_path exist or not
	bool is_csv, is_vtk, is_xml, sim_miss = true;
	is_csv = is_vtk = is_xml = false;

	new_databank_dir = databank_dir + "*.*";
	wstring wsr(new_databank_dir.begin(), new_databank_dir.end());
	HANDLE h = FindFirstFile(LPCWSTR(wsr.c_str()), &data);
	try
	{
		if (h != INVALID_HANDLE_VALUE)
		{
			do
			{
				//get name of each file or folder and put it in file_folder_name_char
				file_folder_name = Win32FindDataToString(data);
				size_file_folder_name = file_folder_name.size();

				//find Results.Statistics.csv file's dir
				if (file_folder_name.find("Results.Statistics.csv") == 0)
				{
					is_csv = true;
					chck++;
					re_sta_csv_path = databank_dir + file_folder_name;
					sim_incomplete = databank_dir;
				}
				else
					//find vtk files' dir
					if (file_folder_name == "Results")
					{
						is_vtk = true;
						vtk_path = databank_dir + file_folder_name;
						vtk_file = GetVtkList(vtk_path);
						chck++;
						sim_incomplete = databank_dir;
					}
					else //find xml file's dir
						if (file_folder_name.substr(file_folder_name.find_last_of(".") + 1) == "xml")
						{
							is_xml = true;
							xml_path = databank_dir + file_folder_name;
							chck++;
							sim_incomplete = databank_dir;
						}
						else // find csv file's dir
							if (file_folder_name.find("Results.Spherulites.csv") == 0)
							{
								is_csv = true;
								chck++;
								csv_path = databank_dir + file_folder_name;
								sim_incomplete = databank_dir;
							}
							else
							{
								if (chck == 4)
								{
									try {
										internal_dir = databank_dir.find_last_of("\\");
										for (size_t i = 0; i < internal_dir; i++)
											inner_folder_dir += databank_dir[i];
										internal_dir = inner_folder_dir.find_last_of("\\");
										for (size_t i = 0; i < internal_dir; i++)
											sim_dir += databank_dir[i];
										inner_folder_dir = sim_dir;
										sim_dir = "";
										internal_dir = inner_folder_dir.find_last_of("\\");
										for (size_t i = internal_dir + 1; i < inner_folder_dir.size(); i++)
											sim_dir += inner_folder_dir[i];

										vector <string> diameter_from_csv;
										//double **result_form_xml;
										int n_sph, num_cell_first, n_nuclei_sph;
										double time_cryst;

										printf("\n%s\n", sim_dir.c_str());

										//temp_in_time = ReadXml(xml_path, sim_dir, temp_in_time, n_time_point, number_of_elements_on_edge, num_cell_first);
										ReadXml(xml_path, sim_dir, temp_in_time, num_cell_first);
										//cout << "Reading XML Is Done!" << "\n";

										if (n_time_point > 0 && crystallisation_temperature)
											time_cryst = CalCrystTime(temp_in_time);
										else time_cryst = -1;
										if (averaged_spherulites_diameter)
										{
											diameter_from_csv = ReadResultsSpherulites(csv_path);
											//cout << "Reading Spherulites Diameter from CSV is Done!\n";
										}
										else
											diameter_from_csv.push_back("-1");

										if (number_of_spherulites)
										{
											n_sph = ReadVtk(vtk_path + "\\" + vtk_file);
											n_nuclei_sph = ReadResultsStatistics(re_sta_csv_path);
											//cout << "Reading Number of Spherulites from VTK Is Done!\n";
										}
										else
										{
											n_sph = -1;
											n_nuclei_sph = -1;
										}

										string sim_str = GetResultFromXmlAndCsv(sim_dir, temp_in_time, time_cryst,
											num_cell_first, diameter_from_csv, n_sph, n_nuclei_sph);


										sim_list.push_back(sim_str);
										//cout << "Add " << sim_dir << " to All Initial Simulations!\n";
										chck++;
									}
									catch (exception& e)
									{
										cout << "Error Bad Allocation: " << e.what() << "\n";
									}
									chck = 0;
									sim_incomplete = "";
									sim_miss = false;
								}
								else
								{
									string extension = file_folder_name.substr(file_folder_name.find_last_of(".") + 1);
									if (extension == "csv" || extension == "txt" || extension == "sph"
										|| extension == "xlsx" || extension == "vtk" || extension == "docx"
										|| file_folder_name == "." || file_folder_name == "..")
									{
										file_folder_name = "";

									}
									else
									{
										string next_databank_dir;
										next_databank_dir = databank_dir + file_folder_name + "\\";

										sim_list = GetXMLandCSVFiles(next_databank_dir, temp_in_time);
										chck = 0;
										sim_incomplete = "";
									}
								}
							}


			} while (FindNextFile(h, &data));
			if (sim_miss)
			{
				ofstream incomplete_sim;
				incomplete_sim.open(sim_incomplete_name.c_str(), ios::app | ios::out);

				if (sim_incomplete.length() > 0)
				{
					printf("misisng file: %s\n", sim_incomplete.c_str());
					//cout << "misisng file: " << sim_incomplete << "\n";
					incomplete_sim << sim_incomplete << "\n";
				}
				incomplete_sim.close();
			}
		}
		else
		{
			if (new_databank_dir.find("InputInfo") == 0)
			{
				printf("This is not Databank folder! \n This is InputInfo.xml file! %s \n", new_databank_dir.c_str());
				/*cout << "This is not Databank folder! \n";
				cout << "This is InputInfo.xml file! " << new_databank_dir << "\n";*/
			}
			else

				if (new_databank_dir.find("Results_Extract") == 0)
				{
					printf("This is not Databank folder! \n This is Results_Extract file!  %s \n", new_databank_dir.c_str());
					/*cout << "This is not Databank folder! \n";
					cout << "This is Results_Extract file! " << new_databank_dir << "\n";*/
				}
				else

					if (new_databank_dir.find("SphaeroSimDataExtractor") == 0)
					{
						printf("This is not Databank folder! \n This is .exe file!  %s \n", new_databank_dir.c_str());
						/*cout << "This is not Databank folder! \n";
						cout << "This is .exe file! " << new_databank_dir << "\n";*/
					}

					else
						if (new_databank_dir.find("C:") == 0 || new_databank_dir.find("E:") == 0
							|| new_databank_dir.find("F:") == 0 || new_databank_dir.find("D:") == 0
							|| new_databank_dir.find("G:") == 0)
							printf("");
						else
							printf("Error: No such folder.\n");
		}
	}
	catch (exception& e)
	{
		cout << "Error: " << e.what();
	}

	FindClose(h);
	return sim_list;
}

void ReadXml(string xml_path,
	string& sim_dir,
	double** temp_in_time_user,
	int& num_cell_first)

{
	double temp_coe;
	string temp_profile_str, sim_name = "", cell_size, num_cell;
	vector <string> temp_profile_array_str;



	tinyxml2::XMLDocument file_handle(true, tinyxml2::PRESERVE_WHITESPACE);
	if (file_handle.LoadFile(xml_path.c_str()) != tinyxml2::XML_NO_ERROR) {
		throw exception("XML file, is not found");
	}

	// load the root entry
	tinyxml2::XMLElement* root_element =
		file_handle.FirstChildElement("SphaeroSim");
	if (root_element == NULL)
		throw exception("Value missing in XML file, The value <SphaeroSim> is missing.");

	// Simulation
	tinyxml2::XMLElement* sim_element =
		root_element->FirstChildElement("Simulation");
	if (sim_element == NULL)
		throw exception("Value missing in XML file, At least one <Simulation> must be defined.");
	//find simulation with name equal to sim_dir
	while (sim_element != NULL)
	{
		//Name
		tinyxml2::XMLElement* name_element =
			sim_element->FirstChildElement("name");
		if (name_element == NULL)
			throw exception("Value missing in XML file, At least one <name> must be defined.");
		sim_name = name_element->GetText();

		if (sim_name == sim_dir)
			break;
		sim_element = sim_element->NextSiblingElement("Simulation");
	}

	if (sim_element->Attribute("cell_size_m"))
		cell_size = sim_element->Attribute("cell_size_m");
	if (number_of_elements_on_edge)
	{
		tinyxml2::XMLElement* num_cell_element =
			sim_element->FirstChildElement("num_cells");
		if (num_cell_element == NULL)
			throw exception("Value missing in XML file, At least one <num_cells> must be defined.");
		num_cell = num_cell_element->GetText();

		//extract fisrt member of nume_cells
		size_t id_start = 1, id_end;
		id_end = num_cell.find(',', id_start);
		num_cell_first = StringToInt(num_cell.substr(id_start, id_end - 1));
	}
	else
		num_cell_first = -1;
	if (n_time_point > 0)
	{
		//tempratur
		tinyxml2::XMLElement* temp_coe_element =
			sim_element->FirstChildElement("temperature");
		temp_coe = StringToDouble(temp_coe_element->GetText());

		//temprature profile
		tinyxml2::XMLElement* temp_pro_element =
			sim_element->FirstChildElement("temperature_profile");

		//split vecotr temprature_profile to temp_profile_array
		temp_profile_str = temp_pro_element->GetText();
		temp_profile_array_str = SplitTempProfile(temp_profile_str, ";");

		//convert temp_profile_array_str to temp_profile_array_double
		size_t row = temp_profile_array_str.size();
		double** temp_in_time = new double*[row];
		for (size_t l = 0; l < row; l++)
		{
			temp_in_time[l] = new double[2];
		}
		for (int f = 0; f < row; f++)
		{
			SplitTempPair(temp_in_time, temp_profile_array_str.at(f), ",", f);
		}

		int i;
		size_t j;
		double** result_from_xml = new double*[n_time_point];
		for (int i = 0; i < n_time_point; i++)
			result_from_xml[i] = new double[2];

		for (i = 0; i < n_time_point; i++)
		{
			for (j = 0; j < row;)
			{
				if (temp_in_time_user[i][0] == temp_in_time[j][0])
				{
					temp_in_time_user[i][1] = temp_in_time[j][1];
					break;
				}
				else
				{
					if (temp_in_time_user[i][0] > temp_in_time[j][0])
					{
						if (j == row - 1)
						{
							temp_in_time_user[i][1] = temp_in_time[j][1];
						}
						j++;
					}
					else if (temp_in_time_user[i][0] < temp_in_time[j][0])
					{
						temp_in_time_user[i][1] = InterlpolationTemp(temp_in_time[j - 1][1], temp_in_time[j][1],
							temp_in_time[j - 1][0], temp_in_time[j][0], temp_in_time_user[i][0]);
						break;
					}
				}
			}
		}


		// power every temp in temp_in_time in temp_coe
		for (int idx_time = 0; idx_time < n_time_point; idx_time++)
		{
			temp_in_time_user[idx_time][1] *= temp_coe;
			result_from_xml[idx_time][0] = temp_in_time_user[idx_time][0];
			result_from_xml[idx_time][1] = temp_in_time_user[idx_time][1];
			//cout << "time in " << result_from_xml[idx_time][0] << ": " << result_from_xml[idx_time][1] << "\n";
		}
		temp_profile_array_str.clear();
		temp_profile_array_str.shrink_to_fit();
		//return result_from_xml;
	}
	/*else
	{
	return temp_in_time_user;
	}*/

}

//extract function
vector <string> SplitTempProfile(const string  & temp_profile,
	const string  & delimiter)
{
	//theString: tempnow, TheDelimiter: ";", theStringVector: vector array of temprature in pair (x, y)
	size_t  start = 0, end = 0;
	vector <string> splited_result;
	while (end != string::npos)
	{
		end = temp_profile.find(delimiter, start);
		splited_result.push_back(temp_profile.substr(start, (end == string::npos)
			? string::npos : end - start));

		start = ((end > (string::npos - delimiter.size()))
			? string::npos : end + delimiter.size());
	}
	return splited_result;
}

//split each temp_pair in double array of temp_in_time
void SplitTempPair(double** Temp_in_time,
	const string& temp_pair,
	const string& delimiter,
	int& idx)
{
	size_t start = 1, end;
	end = temp_pair.find(delimiter, start);
	double index_temp = StringToDouble(temp_pair.substr(start, end - 1));
	start = end + 1;
	end = temp_pair.find(')', start);
	double temp;
	string String_outtemp;
	for (size_t i = start; i < end; i++)
		String_outtemp += temp_pair[i];
	temp = StringToDouble(String_outtemp);
	Temp_in_time[idx][0] = index_temp;
	Temp_in_time[idx][1] = temp;
}

//interpolation
double InterlpolationTemp(double& temp_fisrt,
	double& temp_last,
	double& index_fisrt,
	double& index_last,
	double& index_current)
{
	double temp_current;
	temp_current = (temp_fisrt * (index_last - index_current) + temp_last * (index_current - index_fisrt)) /
		(index_last - index_fisrt);
	return temp_current;
}


//read from csv file
vector <string> ReadResultsSpherulites(string csv_path)
{
	ifstream csv_file;
	string diameter = " ", other_parameter;
	vector <string> out_diameter;

	csv_file.open(csv_path);
	if (!csv_file.is_open())
	{
		printf("NO FILE HAS BEEN OPENED\n");
		//cout << "NO FILE HAS BEEN OPENED\n";
		return out_diameter;
	}
	if (csv_file.is_open())
	{
		while (csv_file.good())
		{
			getline(csv_file, other_parameter, ';');
			getline(csv_file, other_parameter, ';');
			getline(csv_file, diameter, ';');
			getline(csv_file, other_parameter, '\n');
			out_diameter.push_back(diameter);
		}
	}
	csv_file.close();
	return out_diameter;
}

//vector<string> ReadResultsStatistics(string csv_path)
int ReadResultsStatistics(string csv_path)
{
	ifstream csv_file;
	string spherulites = " ", other_parameter;
	vector <string> n_nuclei_sph;

	csv_file.open(csv_path);
	if (!csv_file.is_open())
	{
		printf("NO FILE HAS BEEN OPENED\n");
		//cout << "NO FILE HAS BEEN OPENED\n";
		return -1;
	}
	if (csv_file.is_open())
	{
		while (csv_file.good())
		{
			for (int i = 0; i < 5; i++)
			{
				getline(csv_file, other_parameter, ';');
			}

			getline(csv_file, spherulites, '\n');
			n_nuclei_sph.push_back(spherulites);
		}
	}
	csv_file.close();
	//return n_nuclei_sph;
	return StringToInt(spherulites);
}

int ReadVtk(string vtk_path)
{
	ifstream vtk_file;
	string vtk_line = "";
	size_t n_sph = 0, start_n_sph = -1, end_n_sph, find_n_sph = 0;
	vector<string> n_sph_list;

	vtk_file.open(vtk_path);
	if (!vtk_file.is_open())
	{
		printf("No VTK File Has Been Opened!\n");
		//cout << "No VTK File Has Been Opened!\n";
		return -1;
	}
	else if (vtk_file.is_open())
	{
		while (vtk_file.good())
		{
			if (vtk_line != "VECTORS Location_[mm] float")
			{
				getline(vtk_file, vtk_line, '\n');
				if (vtk_line == "SCALARS spheru_id float 1")
					find_n_sph = 1;
				if (find_n_sph == 1)
				{
					n_sph_list.push_back(vtk_line);
				}
			}
			else
				break;
		}
	}

	size_t size_list = n_sph_list.size();

	if (size_list > 0)
	{
		for (size_t i = 0; i < size_list; i++)
		{
			if (n_sph_list[i] == "SCALARS spheru_id float 1")
				start_n_sph = i;
			end_n_sph = size_list;
		}

		size_t len_ids, id, i, k, j, chck_similarity;
		len_ids = end_n_sph - start_n_sph;
		id = 0;
		chck_similarity = 0;
		string *ids = new string[len_ids];

		for (j = start_n_sph + 2, i = 0; j < end_n_sph - 1; j++, i++)
			ids[i] = n_sph_list[j];

		for (k = 0; k < i; k++)
		{
			if (k == 0)
			{
				//cout << "ids[" << n_sph << "]: " << ids[k] << "\n";
				n_sph++;
			}
			else
			{
				for (int h = k - 1; h > -1; h--)
				{
					if (ids[k] == ids[h])
					{
						chck_similarity = 0;
						break;
					}
					else
						chck_similarity = 1;
				}
				if (chck_similarity == 1)
				{
					//cout << "ids[" << n_sph << "]: " << ids[k] << "\n";
					n_sph++;
				}
			}
		}


		vtk_file.close();
		delete[] ids;
		n_sph_list.clear();
		n_sph_list.shrink_to_fit();
	}
	return n_sph;
}

string GetResultFromXmlAndCsv(string& sim_dir,
	double **temp_in_time,
	double& time_cryst,
	int& num_cell_first,
	vector<string>& diameter_from_csv,
	int& n_sph,
	int& n_nuclei_sph)
{

	string sim_str = "";
	double sum_diameter = 0.0, sd, avg_sph_diameter, difference, sum = 0.0;
	int n_spherulites_diameter = diameter_from_csv.size() - 1, idx_diamter;

	if (n_time_point > 0 || time_cryst >= 0 || num_cell_first >= 0 || n_sph > 0 || n_spherulites_diameter > 1)
		sim_str += sim_dir + ";";
	else
		sim_str += sim_dir;
	if (n_time_point > 0)
	{
		for (int id = 0; id < n_time_point - 1; id++)
			sim_str += to_string(temp_in_time[id][1]) + ";";
		if (time_cryst >= 0 || num_cell_first >= 0 || n_sph > 0 || n_spherulites_diameter > 1)
			sim_str += to_string(temp_in_time[n_time_point - 1][1]) + ";";
		else
			sim_str += to_string(temp_in_time[n_time_point - 1][1]);
	}
	if (time_cryst >= 0)
	{
		if (num_cell_first >= 0 || n_sph > 0 || n_spherulites_diameter > 1)
			sim_str += to_string(time_cryst) + ";";
		else
			sim_str += to_string(time_cryst);
	}
	if (num_cell_first >= 0)
	{
		if (n_sph > 0 || n_spherulites_diameter > 1)
			sim_str += to_string(num_cell_first) + ";";
		else
			sim_str += to_string(num_cell_first);
	}
	if (n_sph > 0)
	{
		if (n_spherulites_diameter > 1)
		{
			sim_str += to_string(n_sph) + ";";
			sim_str += to_string(n_nuclei_sph) + ";";
		}
		else
		{
			sim_str += to_string(n_sph) + ";";
			sim_str += to_string(n_nuclei_sph);
		}
	}

	if (n_spherulites_diameter > 1)
	{
		for (idx_diamter = 1; idx_diamter <= n_spherulites_diameter; idx_diamter++)
			sum_diameter += StringToDouble(diameter_from_csv.at(idx_diamter));
		avg_sph_diameter = (sum_diameter / n_spherulites_diameter);

		for (idx_diamter = 1; idx_diamter <= n_spherulites_diameter; idx_diamter++)
		{
			double m_mean = StringToDouble(diameter_from_csv.at(idx_diamter)) - avg_sph_diameter;
			if (m_mean < 0)
				m_mean = avg_sph_diameter - StringToDouble(diameter_from_csv.at(idx_diamter));
			difference = m_mean * m_mean;
			sum += difference;
		}

		sd = sqrt(sum / (n_spherulites_diameter - 1));

		sim_str += to_string(avg_sph_diameter) + ";" + to_string(sd);
	}

	else
	{
		avg_sph_diameter = sd = -1;
	}


	diameter_from_csv.clear();
	diameter_from_csv.shrink_to_fit();

	return sim_str;
}

void WriteToCsv(vector<string>& sim_list,
	string& file_path,
	double** temp_in_time)
{
	//Remove the previous output file if exist
	remove(file_path.c_str());

	ofstream sim_file;
	//Open new output file and save the results
	sim_file.open(file_path, ios::app);
	if (n_time_point > 0 || crystallisation_temperature || number_of_elements_on_edge || number_of_spherulites ||
		averaged_spherulites_diameter)
		sim_file << "Simulation" << ";";
	else
		sim_file << "Simulation\n";

	if (n_time_point > 0)
	{
		for (int i = 0; i < n_time_point - 1; i++)
			sim_file << "Time" << i << " in " << temp_in_time[i][0] << ";";
		if (crystallisation_temperature || number_of_elements_on_edge || number_of_spherulites ||
			averaged_spherulites_diameter)
			sim_file << "Time" << n_time_point - 1 << " in " << temp_in_time[n_time_point - 1][0] << ";";
		else
			sim_file << "Time" << n_time_point - 1 << " in " << temp_in_time[n_time_point - 1][0] << "\n";
	}

	if (crystallisation_temperature)
	{
		if (number_of_elements_on_edge || number_of_spherulites || averaged_spherulites_diameter)
			sim_file << "Time_Cryst;";
		else
			sim_file << "Time_Cryst\n";
	}
	if (number_of_elements_on_edge)
	{
		if (number_of_spherulites || averaged_spherulites_diameter)
			sim_file << "Num_Cell;";
		else
			sim_file << "Num_Cell\n";
	}
	if (number_of_spherulites)
	{
		if (averaged_spherulites_diameter)
		{
			sim_file << "N_Sph;";
			sim_file << "N_Nuclei_Sph;";
		}
		else
		{
			sim_file << "N_Sph;";
			sim_file << "N_Nuclei_Sph\n";
		}
	}
	if (averaged_spherulites_diameter)
		//sim_file << "Dia; SD" << "\n";
		sim_file << "Dia; SD" << "\n";
	for (size_t i = 0; i < sim_list.size(); i++)
		sim_file << sim_list[i] << "\n";
	sim_list.clear();
	sim_list.shrink_to_fit();
}

vector<string> MakeAvgSim(string& sim_healthy_path,
	double** temp_in_time_user)
{
	ifstream sim_healthy_file;
	string sim_str_avg, sim_id, cryst_time, num_cell, n_sph, n_nuclei_sph, dia, sd;
	vector <string> sim_vector_avg;
	int step = 0, n_line = 0, making_avg = 0;
	double n_sph_avg = 0, n_nuclei_sph_avg = 0, dia_avg = 0, sd_avg = 0;
	string* time = new string[n_time_point];


	sim_healthy_file.open(sim_healthy_path);
	if (!sim_healthy_file.is_open())
	{
		printf("NO FILE HAS BEEN OPENED\n");
		//cout << "NO FILE HAS BEEN OPENED\n";
		making_avg = 0;
	}
	if (sim_healthy_file.is_open())
	{
		while (sim_healthy_file.good())
		{
			if (step == 3)
			{
				sim_str_avg = sim_id.substr(0, sim_id.find("_")) + ";";
				if (n_time_point > 0)
				{
					for (int i = 0; i < n_time_point - 1; i++)
						sim_str_avg += time[i] + ";";

					if (crystallisation_temperature || number_of_elements_on_edge || number_of_spherulites ||
						averaged_spherulites_diameter)
						sim_str_avg += time[n_time_point - 1] + ";";
					//sim_str_avg += time[i] + ";";
					else
						sim_str_avg += time[n_time_point - 1] + '\n';
				}
				if (crystallisation_temperature)
				{
					sim_str_avg += cryst_time;
					if (number_of_elements_on_edge || number_of_spherulites || averaged_spherulites_diameter)
					{
						sim_str_avg += ";";
					}
				}
				if (number_of_elements_on_edge)
				{
					sim_str_avg += num_cell;
					if (number_of_spherulites || averaged_spherulites_diameter)
					{
						sim_str_avg += ";";
					}
				}

				if (number_of_spherulites)
				{
					n_sph_avg /= step;
					n_nuclei_sph_avg /= step;
					sim_str_avg += to_string(n_sph_avg) + ";" + to_string(n_nuclei_sph_avg);
					if (averaged_spherulites_diameter)
					{
						sim_str_avg += ";";
					}
				}
				if (averaged_spherulites_diameter)
				{
					dia_avg /= step;
					sd_avg /= step;
					sim_str_avg += to_string(dia_avg) + ";" + to_string(sd_avg);
				}

				sim_vector_avg.push_back(sim_str_avg);
				step = 0;
				dia_avg = sd_avg = n_sph_avg = 0.0;
				sim_str_avg = "";
			}

			if (n_time_point > 0 || crystallisation_temperature || number_of_elements_on_edge || number_of_spherulites ||
				averaged_spherulites_diameter)
				getline(sim_healthy_file, sim_id, ';');
			else
				getline(sim_healthy_file, sim_id, '\n');
			//sim_str_avg = sim_id.substr(0, sim_id.find("_")) + ";";
			if (n_time_point > 0)
			{
				for (int i = 0; i < n_time_point - 1; i++)
					getline(sim_healthy_file, time[i], ';');

				if (crystallisation_temperature || number_of_elements_on_edge || number_of_spherulites ||
					averaged_spherulites_diameter)
					getline(sim_healthy_file, time[n_time_point - 1], ';');
				//sim_str_avg += time[i] + ";";
				else
					getline(sim_healthy_file, time[n_time_point - 1], '\n');
			}

			if (crystallisation_temperature)
			{
				if (number_of_elements_on_edge || number_of_spherulites || averaged_spherulites_diameter)
					getline(sim_healthy_file, cryst_time, ';');
				else
					getline(sim_healthy_file, cryst_time, '\n');
			}

			if (number_of_elements_on_edge)
			{
				if (number_of_spherulites || averaged_spherulites_diameter)
					getline(sim_healthy_file, num_cell, ';');
				else
					getline(sim_healthy_file, num_cell, '\n');
			}

			if (number_of_spherulites)
			{
				if (averaged_spherulites_diameter)
				{
					getline(sim_healthy_file, n_sph, ';');
					getline(sim_healthy_file, n_nuclei_sph, ';');
				}
				else
				{
					getline(sim_healthy_file, n_sph, ';');
					getline(sim_healthy_file, n_nuclei_sph, '\n');
				}
			}

			if (averaged_spherulites_diameter)
			{
				getline(sim_healthy_file, dia, ';');
				getline(sim_healthy_file, sd, '\n');
			}

			if (n_line > 0)
			{
				step++;
				if (number_of_spherulites)
				{
					n_sph_avg += StringToInt(n_sph);
					n_nuclei_sph_avg += StringToInt(n_nuclei_sph);
				}
				if (averaged_spherulites_diameter)
				{
					dia_avg += StringToDouble(dia);
					sd_avg += StringToDouble(sd);
				}
			}
			n_line++;
		}
		making_avg = 1;
	}

	if (making_avg == 1)
	{
		sim_healthy_file.close();
		return sim_vector_avg;
		/*sim_vector_avg.clear();
		sim_vector_avg.shrink_to_fit();*/
	}
	else
		return sim_vector_avg = { "" };
}

vector <vector<string>> SeperateSimilarandNotSimilarSims(string& sim_avg_path)
{
	ifstream csv_file;
	string sim_id, n_sph, line;
	string* times = new string[n_time_point];
	vector <string> sim_similar, sim_not_similar, sim_list_avg;
	vector <vector<string>> sims;
	int n_sim = 0;

	csv_file.open(sim_avg_path);

	if (!csv_file.is_open())
	{
		printf("NO FILE HAS BEEN OPENED\n");
		//cout << "NO FILE HAS BEEN OPENED\n";

	}
	//read rows from input file
	if (csv_file.is_open())
		while (csv_file.good())
		{
			getline(csv_file, line, '\n');

			if (n_sim > 0)
			{
				if (n_sim == 1)
				{
					sim_not_similar.push_back(line);
				}
				else
					sim_list_avg.push_back(line);
			}
			n_sim++;
		}
	if (n_time_point> 0)
	{
		for (int i = 0; i < n_sim - 3; i++)
		{
			int add_sim_similar = 0;
			int find_input;
			find_input = sim_list_avg[i].find(";");
			sim_id = sim_list_avg[i].substr(0, find_input);
			string sim_res = sim_list_avg[i].substr(find_input + 1);

			bool similar = true;
			size_t size_sim_not_similar_ = sim_not_similar.size();

			for (size_t j = 0; j < size_sim_not_similar_; j++)
			{
				similar = true;
				string sim_id2, sim_res2;
				string* times2 = new string[n_time_point];
				//times = times2 = { "" };
				int find_input2;
				find_input2 = sim_not_similar[j].find(";");
				sim_id2 = sim_not_similar[j].substr(0, find_input2);
				sim_res2 = sim_not_similar[j].substr(find_input2 + 1);

				int l = 0;
				for (l = 0; l < n_time_point;)
				{
					//get times from sim_not_similar
					find_input2 = sim_res2.find(";");
					times2[l] = sim_res2.substr(0, find_input2);
					sim_res2 = sim_res2.substr(find_input2 + 1);

					//get times from sim_list
					find_input = sim_res.find(";");
					times[l] = sim_res.substr(0, find_input);
					sim_res = sim_res.substr(find_input + 1);


					if (StringToDouble(times[l]) == StringToDouble(times2[l]))
					{
						similar = true;
						l++;
					}

					else if (StringToDouble(times[l]) != StringToDouble(times2[l]))
					{
						similar = false;
						l = n_time_point;
						find_input = sim_list_avg[i].find(";");
						sim_id = sim_list_avg[i].substr(0, find_input);
						sim_res = sim_list_avg[i].substr(find_input + 1);
						break;
					}
				}
				if (crystallisation_temperature && similar)
				{

					find_input2 = sim_res2.find(";");
					double time_cryst2 = StringToDouble(sim_res2.substr(0, find_input2));
					sim_res2 = sim_res2.substr(find_input2 + 1);

					//get times from sim_list
					find_input = sim_res.find(";");
					double time_cryst = StringToDouble(sim_res.substr(0, find_input));
					sim_res = sim_res.substr(find_input + 1);
					if (time_cryst2 == time_cryst)
						similar = true;
					else
						similar = false;
					//printf("time_cryst2: %f , time_cryst: %f \n\n\n", time_cryst2, time_cryst);
					//cout << "time_cryst2: " << time_cryst2 << ", time_cryst: " << time_cryst << "\n\n\n";
					//system("pause");
				}

				if (number_of_elements_on_edge && similar)
				{
					find_input2 = sim_res2.find(";");
					int num_cell2 = StringToInt(sim_res2.substr(0, find_input2));
					sim_res2 = sim_res2.substr(find_input2 + 1);

					//get times from sim_list
					find_input = sim_res.find(";");
					int num_cell = StringToInt(sim_res.substr(0, find_input));
					sim_res = sim_res.substr(find_input + 1);
					if (num_cell2 == num_cell)
						similar = true;
					else
						similar = false;
					//printf("num_cell2: %i , num_cell: %i \n\n\n", num_cell2, num_cell);
					//cout << "num_cell2: " << num_cell2 << ", num_cell: " << num_cell << "\n\n\n";
					//system("pause");
				}

				//find_id2 = sim_res2.find(";");
				//times2[l] = sim_res2.substr(0, find_id2);
				//sim_res2 = sim_res2.substr(find_id2 + 1);

				////get times from sim_list
				//find_id = sim_res.find(";");
				//times[l] = sim_res.substr(0, find_id);
				//sim_res = sim_res.substr(find_id + 1);

				if (similar == true)
				{
					sim_similar.push_back(sim_list_avg[i]);
					add_sim_similar = 1;
					j = size_sim_not_similar_;
				}
			}
			if (similar == false && add_sim_similar == 0)
			{
				sim_not_similar.push_back(sim_list_avg[i]);
			}
		}
	}
	else
	{
		if (number_of_elements_on_edge)
		{
			for (int i = 0; i < n_sim - 3; i++)
			{
				int add_sim_similar = 0;
				int find_input;
				find_input = sim_list_avg[i].find(";");
				sim_id = sim_list_avg[i].substr(0, find_input);
				string sim_res = sim_list_avg[i].substr(find_input + 1);

				bool similar = true;
				size_t size_sim_not_similar_ = sim_not_similar.size();

				for (size_t j = 0; j < size_sim_not_similar_; j++)
				{
					similar = true;
					string sim_id2, sim_res2;
					int find_input2;
					find_input2 = sim_not_similar[j].find(";");
					sim_id2 = sim_not_similar[j].substr(0, find_input2);
					sim_res2 = sim_not_similar[j].substr(find_input2 + 1);

					find_input2 = sim_res2.find(";");
					int num_cell2 = StringToInt(sim_res2.substr(0, find_input2));
					sim_res2 = sim_res2.substr(find_input2 + 1);

					//get times from sim_list
					find_input = sim_res.find(";");
					int num_cell = StringToInt(sim_res.substr(0, find_input));
					sim_res = sim_res.substr(find_input + 1);
					if (num_cell2 == num_cell)
					{
						similar = true;
						find_input = sim_list_avg[i].find(";");
						sim_res = sim_list_avg[i].substr(find_input + 1);
					}
					else
					{
						similar = false;
						find_input = sim_list_avg[i].find(";");
						sim_res = sim_list_avg[i].substr(find_input + 1);
					}
					//printf("num_cell2: %i , num_cell: %i \n\n\n", num_cell2, num_cell);
					//cout << "num_cell2: " << num_cell2 << ", num_cell: " << num_cell << "\n";
					//system("pause");

					if (similar == true)
					{
						sim_similar.push_back(sim_list_avg[i]);
						add_sim_similar = 1;
						j = size_sim_not_similar_;
					}
				}
				if (similar == false && add_sim_similar == 0)
				{
					sim_not_similar.push_back(sim_list_avg[i]);
				}
			}
		}
		else
			for (int i = 0; i < n_sim - 3; i++)
				sim_not_similar.push_back(sim_list_avg[i]);
	}

	/*for (size_t i = 0; i < sim_not_similar.size(); i++)
	cout << "not_similar:"  << sim_not_similar[i] << "\n";
	for (size_t i = 0; i < sim_similar.size(); i++)
	cout << "similar" << sim_similar[i] << "\n";*/
	//system("pause");
	sims.push_back(sim_similar);
	sims.push_back(sim_not_similar);

	csv_file.close();
	sim_similar.clear();
	sim_not_similar.clear();
	sim_list_avg.clear();
	sim_similar.shrink_to_fit();
	sim_not_similar.shrink_to_fit();
	sim_list_avg.shrink_to_fit();

	return sims;
}

vector <vector<string>> SeperateHealthAndMissingSim(string& initial_sim_path)
{
	ifstream csv_file;
	string sim_id, n_sph, other_param;
	vector <string> sim_missing, sim_healthy, sim_list;
	vector <vector<string>> sims;
	int n_sim = 0, n_sim_similar;

	csv_file.open(initial_sim_path);

	if (!csv_file.is_open())
	{
		printf("NO FILE HAS BEEN OPENED\n");
		//cout << "NO FILE HAS BEEN OPENED\n";
	}
	//read rows from output file
	if (csv_file.is_open())
		while (csv_file.good())
		{
			getline(csv_file, other_param, '\n');
			if (n_sim > 0)
				sim_list.push_back(other_param);
			n_sim++;
		}

	/*for (int i = 0; i < n_sim - 2; i++)
	cout << "sim: " << sim_list[i] << "\n";
	system("pause");*/
	for (int i = 0; i < n_sim - 2;)
	{
		n_sim_similar = 1;
		int sim_id_part2;
		sim_id = sim_list[i].substr(0, sim_list[i].find(";"));
		sim_id_part2 = StringToInt(sim_id.substr(sim_id.find("_") + 1));
		sim_id = sim_id.substr(0, sim_id.find("_"));

		/*cout << "sim_id_new: " << sim_id << ", part2: " << sim_id_part2 << ", n_sim_similar: " << n_sim_similar << "\n";
		system("pause");*/

		for (int j = i + 1; j < n_sim - 2; j++)
		{
			string sim_id2 = sim_list[j].substr(0, sim_list[j].find(";"));
			int sim_id2_part2 = StringToInt(sim_id2.substr(sim_id2.find("_") + 1));
			sim_id2 = sim_id2.substr(0, sim_id2.find("_"));
			/*cout << "sim_id2: " << sim_id2 << ", sim_id2_part2: " << sim_id2_part2 << "\n";
			system("pause");*/

			if (sim_id2 == sim_id && sim_id2_part2 == sim_id_part2)
			{
				//cout << "sim_id: " << sim_id  << ", part2 = part2: " << sim_id_part2 << ", n_sim_similar: " << n_sim_similar << "\n";
				j = n_sim;
				//system("pause");
				continue;
			}
			else

				if (sim_id2 == sim_id && n_sim_similar < 3 && sim_id2_part2 > sim_id_part2)
				{
					sim_id_part2 = sim_id2_part2;
					n_sim_similar++;
					if (n_sim_similar == 3)
					{
						j = n_sim;
					}

					//cout << "sim_id: " << sim_id << ", sim_id_part2: " << sim_id_part2 << ", n_sim_similar: " << n_sim_similar << "\n";
					//system("pause");
				}
		}

		if (n_sim_similar == 3)
			for (int k = i; k < i + n_sim_similar; k++)
				sim_healthy.push_back(sim_list[k]);
		else
			for (int k = i; k < i + n_sim_similar; k++)
				sim_missing.push_back(sim_list[k]);
		i += n_sim_similar;
	}

	sims.push_back(sim_healthy);
	sims.push_back(sim_missing);
	csv_file.close();
	return sims;
}

//convert string to double
double StringToDouble(const string& value_str) {
	double result_value;
	std::stringstream stream(value_str);
	stream >> result_value;
	if (stream.fail())
	{
		printf("%f\n", result_value);
		//cout << result_value << "\n";
		throw exception("String to Double Conversion Failed!");
	}
	return result_value;
}

//convert string to int
int StringToInt(const string& value_str)
{
	int result_value;
	std::stringstream stream(value_str);
	stream >> result_value;
	if (stream.fail())
	{
		printf("%i\n", result_value);
		cout << result_value << "\n";
		throw exception("String to Int Conversion Failed!");
	}	return result_value;
}

string Win32FindDataToString(WIN32_FIND_DATA data)
{
	string str_file_name;
	char*  chr_file_name = new char[lstrlen(data.cFileName) + 1];
	for (int i = 0; i < lstrlen(data.cFileName); i++)
	{
		chr_file_name[i] = char(data.cFileName[i]);
	}
	chr_file_name[lstrlen(data.cFileName)] = '\0';
	str_file_name = chr_file_name;
	return str_file_name;
}

void SortSim(vector<string>& sim_list)
{
	int size_list = sim_list.size();
	for (int i = 1; i < size_list; i++)
	{
		for (int j = i - 1; j > -1; j--)
		{
			string sim_id = sim_list[j].substr(0, sim_list[j].find(";"));
			sim_id = sim_id.substr(0, sim_id.find("_"));

			string sim_id2 = sim_list[j + 1].substr(0, sim_list[j + 1].find(";"));
			sim_id2 = sim_id2.substr(0, sim_id2.find("_"));

			if (StringToInt(sim_id.substr(3)) > StringToInt(sim_id2.substr(3)))
			{
				/*cout << "sim_list[" << j<<"]: " << sim_list[j] << "\n";
				cout << "sim_list["<<j+1<<"]: " << sim_list[j + 1] << "\n";*/
				string temporary = sim_list[j];
				sim_list[j] = sim_list[j + 1];
				sim_list[j + 1] = temporary;
				/*cout << "sim_list[" << j << "]: " << sim_list[j] << "\n";
				cout << "sim_list[" << j + 1 << "]: " << sim_list[j + 1] << "\n";
				system("pause");*/

			}
			else
				break;
		}
	}
}


void ReadInputXml()
{
	n_time_point = 0;

	tinyxml2::XMLDocument file_handle(true, tinyxml2::PRESERVE_WHITESPACE);
	if (file_handle.LoadFile(input_xml.c_str()) != tinyxml2::XML_NO_ERROR)
		throw exception("Input XML file, is not found");

	// load the root entry
	tinyxml2::XMLElement* root_element =
		file_handle.FirstChildElement("DataEx");
	if (root_element == NULL)
		throw exception("Value missing in XML file, the value <DataEx> is missing.");

	// Databanks
	tinyxml2::XMLElement* databank_path_element =
		root_element->FirstChildElement("databank_path");
	if (databank_path_element == NULL)
		throw exception("Value missing in XML file, At least one <databank_path> must be defined.");
	// Databanks name array
	tinyxml2::XMLElement* databank_name_element =
		databank_path_element->FirstChildElement("databank");
	if (databank_name_element == NULL)
		throw exception("Value missing in XML file, At least one <databank_name> must be defined.");
	while (databank_name_element != NULL)
	{
		string databank_name = databank_name_element->GetText();
		databank_name += "\\";
		//databanks_name.push_back(databank_name_element->GetText());
		databanks_name.push_back(databank_name);
		databank_name_element = databank_name_element->NextSiblingElement("databank");
	}


	// Result files
	tinyxml2::XMLElement* result_files_element =
		root_element->FirstChildElement("result_files");
	if (result_files_element == NULL)
		throw exception("Value missing in XML file, At least one <result_files> must be defined.");

	// Sim_incomplete
	tinyxml2::XMLElement* sim_incomplete_element =
		result_files_element->FirstChildElement("sim_incomplete");
	if (sim_incomplete_element == NULL)
		throw exception("Value missing in XML file, At least one <sim_incomplete> must be defined.");
	sim_incomplete = StringToBool(sim_incomplete_element->GetText());

	// All sim
	tinyxml2::XMLElement* all_sim_element =
		result_files_element->FirstChildElement("all_sim");
	if (all_sim_element == NULL)
		throw exception("Value missing in XML file, At least one <all_sim> must be defined.");
	complete_sim = StringToBool(all_sim_element->GetText());
	// Sim healthy
	tinyxml2::XMLElement* sim_healthy_element =
		result_files_element->FirstChildElement("sim_healthy");
	if (sim_healthy_element == NULL)
		throw exception("Value missing in XML file, At least one <sim_healthy> must be defined.");
	sim_healthy = StringToBool(sim_healthy_element->GetText());
	// Sim not healthy
	tinyxml2::XMLElement* sim_not_healthy_element =
		result_files_element->FirstChildElement("sim_not_healthy");
	if (sim_not_healthy_element == NULL)
		throw exception("Value missing in XML file, At least one <sim_not_healthy> must be defined.");
	sim_not_healthy = StringToBool(sim_not_healthy_element->GetText());
	// Sim averaged
	tinyxml2::XMLElement* sim_averaged_element =
		result_files_element->FirstChildElement("sim_averaged");
	if (sim_averaged_element == NULL)
		throw exception("Value missing in XML file, At least one <sim_averaged> must be defined.");
	sim_avg = StringToBool(sim_averaged_element->GetText());
	// Sim_similar
	tinyxml2::XMLElement* sim_similar_element =
		result_files_element->FirstChildElement("sim_similar");
	if (sim_similar_element == NULL)
		throw exception("Value missing in XML file, At least one <sim_similar> must be defined.");
	sim_similar_bool = StringToBool(sim_similar_element->GetText());
	// Sim_not_similar
	tinyxml2::XMLElement* sim_not_similar_element =
		result_files_element->FirstChildElement("sim_not_similar");
	if (sim_not_similar_element == NULL)
		throw exception("Value missing in XML file, At least one <sim_not_similar> must be defined.");
	sim_not_similar_bool = StringToBool(sim_not_similar_element->GetText());


	///////////////////////////////////////////////////
	// Requested inputs
	tinyxml2::XMLElement* requested_inputs_element =
		root_element->FirstChildElement("requested_inputs");
	if (requested_inputs_element == NULL)
		throw exception("Value missing in XML file, At least one <requested_inputs> must be defined.");
	// Temperature history
	tinyxml2::XMLElement* temperature_history_element =
		requested_inputs_element->FirstChildElement("temperature_history");
	if (temperature_history_element == NULL)
		throw exception("Value missing in XML file, At least one <temperature_history> must be defined.");
	temperature_history = StringToBool(temperature_history_element->GetText());
	// Crystallisation temperature
	tinyxml2::XMLElement* crystallisation_temperature_element =
		requested_inputs_element->FirstChildElement("crystallisation_temperature");
	if (crystallisation_temperature_element == NULL)
		throw exception("Value missing in XML file, At least one <crystallisation_temperature> must be defined.");
	crystallisation_temperature = StringToBool(crystallisation_temperature_element->GetText());
	// Number of elements on edge
	tinyxml2::XMLElement* number_of_elements_on_edge_element =
		requested_inputs_element->FirstChildElement("number_of_elements_on_edge");
	if (number_of_elements_on_edge_element == NULL)
		throw exception("Value missing in XML file, At least one <number_of_elements_on_edge> must be defined.");
	number_of_elements_on_edge = StringToBool(number_of_elements_on_edge_element->GetText());

	///////////////////////////////////////////////////
	// Requested outputs
	tinyxml2::XMLElement* requested_outputs_element =
		root_element->FirstChildElement("requested_outputs");
	if (requested_outputs_element == NULL)
		throw exception("Value missing in XML file, At least one <requested_outputs> must be defined.");
	// Number of spherulites
	tinyxml2::XMLElement* number_of_spherulites_element =
		requested_outputs_element->FirstChildElement("number_of_spherulites");
	if (number_of_spherulites_element == NULL)
		throw exception("Value missing in XML file, At least one <number_of_spherulites> must be defined.");
	number_of_spherulites = StringToBool(number_of_spherulites_element->GetText());
	// Averaged spherulites diameter
	tinyxml2::XMLElement* averaged_spherulites_diameter_element =
		requested_outputs_element->FirstChildElement("averaged_spherulites_diameter");
	if (averaged_spherulites_diameter_element == NULL)
		throw exception("Value missing in XML file, At least one <averaged_spherulites_diameter> must be defined.");
	averaged_spherulites_diameter = StringToBool(averaged_spherulites_diameter_element->GetText());

	if (temperature_history)
	{
		// Time Points
		tinyxml2::XMLElement* time_points_element =
			root_element->FirstChildElement("time_points");
		if (time_points_element == NULL)
			throw exception("Value missing in XML file, At least one <time_points> must be defined.");
		string time_points_str = time_points_element->GetText();
		size_t size_str = time_points_str.size();
		time_points_str = time_points_str.substr(1, time_points_str.size() - 2);

		int end = 0;
		while (!end)
		{
			string time_point = time_points_str.substr(0, time_points_str.find(","));
			time_points.push_back(StringToDouble(time_point));
			n_time_point++;

			if (time_points_str.find(",") < size_str)
			{
				end = 0;
				time_points_str = time_points_str.substr(time_points_str.find(",") + 1);
			}
			else
			{
				end = 1;
			}
		}
	}
	else
		crystallisation_temperature = false;
}


double CalCrystTime(double **temp_in_time_user)
{
	/*for (int i = 0; i < n_time_point; i++)
	{
	cout << "temp_in_time_user " << i << ": " << temp_in_time_user[i][1] << "\n";
	}*/
	for (int i = 0; i < n_time_point; i++)
	{
		if (temp_in_time_user[i][1] < temp_cryst)
		{
			/*cout << "temp_in_time_user[i][1]: " << temp_in_time_user[i][1] << "\n";
			system("pause");
			cout << "temp_cryst: " << temp_cryst << "\n";
			system("pause");
			cout << "temp_in_time_user[i - 1][1]: " << temp_in_time_user[i - 1][1] << "\n";
			system("pause");*/
			if (temp_in_time_user[i - 1][1] > temp_cryst)
			{

				double time_point_crys = (((temp_in_time_user[i][0] - temp_in_time_user[i - 1][0]) /
					(temp_in_time_user[i][1] - temp_in_time_user[i - 1][1]))
					* (403.15 - temp_in_time_user[i - 1][1])) + temp_in_time_user[i - 1][0];
				return time_point_crys;
			}
		}
	}
	return 0;
}


bool StringToBool(string str)
{
	if (StringToInt(str) == 1)
		return true;
	else
		return false;
}

