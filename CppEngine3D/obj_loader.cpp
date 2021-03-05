#include "obj_loader.h"



std::string _strip(std::string s)
{
    std::string res = "";
    for (auto c : s)
    {
        if (c != ' ')
        {

            res += c;
        }
    }
    return res;
}



std::vector<std::string> _split(std::string s)
{
    std::vector<std::string> words;
    std::string word;

    for (auto c : s)
    {
        if (c != ' ' && c != DELIM) word += c;
        else
        {            
            words.push_back(word);       
            word = "";
        }

    }
     
    return words;
}


std::vector<triangle> load_obj_from_fname(std::string fname)
{
    std::cout << "Loading " << fname << "..." << std::endl;

    std::vector<v3d> vVec;
    std::vector<triangle> fVec;

    std::vector<std::string> raw;

    std::ifstream file(fname);
    std::string line;

    int nTris = 0, nQuads = 0, nVerts = 0;

    while (std::getline(file, line))
    {
        raw = _split(line + DELIM);

        if (raw[0] == "v")
        {
            nVerts++;
            vVec.push_back(
                { 
                    std::stof(_strip(raw[1])), 
                    std::stof(_strip(raw[2])), 
                    std::stof(_strip(raw[3])) 
                }
            );
        }
        
        else if (raw[0] == "f")
        {
            nTris++;
            fVec.push_back(
                {
                    vVec[std::stoi(raw[1]) - 1],
                    vVec[std::stoi(raw[2]) - 1],
                    vVec[std::stoi(raw[3]) - 1],
                    rand(),
                    rand(),
                    rand()
                }
            );

            if (raw.size() == 5)
            {
                nQuads++;
                nTris++;
                fVec.push_back(
                    {
                        vVec[std::stoi(raw[1]) - 1],
                        vVec[std::stoi(raw[3]) - 1],
                        vVec[std::stoi(raw[4]) - 1],
                        rand(),
                        rand(),
                        rand()
                    }
                );
            }

        }
    }

    std::cout << "Load complete\n\n";
    std::cout << "nTris (with quad splitting): " << nTris << std::endl << "nQuads: " << nQuads << std::endl << "nVerts: " << nVerts << std::endl;

    return fVec;
}
