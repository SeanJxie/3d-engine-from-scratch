#include "obj_loader.h"


std::vector<std::string> _split(const std::string s) {
    std::vector<std::string> strings;
    size_t start;
    size_t end = 0;
    while ((start = s.find_first_not_of(' ', end)) != std::string::npos) {
        end = s.find(' ', start);
        strings.push_back(s.substr(start, end - start));
    }
    return strings;
}


std::vector<triangle> load_obj_mtl_fname(std::string obj, bool useMtl, std::string mtl)
{
    std::string line;
    std::vector<std::string> data;
    std::string dataType; // for readability

    std::unordered_map<std::string, SDL_Color> materialMap;
    std::string mtlName;
    
    // Load mtl
    if (useMtl)
    {
        std::cout << "Loading material [" << mtl << "] ..." << std::endl;
        const char* TARGET_PROPERTY = "Ks"; // Ambient/specular/diffuse
        std::cout << "Target texture property = " << TARGET_PROPERTY << "\n\n";
        // https://en.wikipedia.org/wiki/Shading#Ambient_lighting
        std::ifstream mtlfs(mtl);
        while (std::getline(mtlfs, line))
        {
            data = _split(line);

            if (!data.empty())
            {
                dataType = data[0];
                if (dataType == "newmtl")
                {
                    mtlName = data[1];
                }

                if (dataType == TARGET_PROPERTY)
                {
                    SDL_Color c = {
                        std::stof(data[1]) * 255,
                        std::stof(data[2]) * 255,
                        std::stof(data[3]) * 255
                    };

                    materialMap[mtlName] = c;
                }
            }
        }

        mtlfs.close();
    }
    else std::cout << "No material loaded\n\n";


    // Load obj
    std::cout << "Loading object [" << obj << "] ...\n\n";
    std::vector<v3d> vVec;
    std::vector<triangle> fVec;

    std::ifstream objfs(obj);

    SDL_Color currCol;
    

    int nTris = 0, nQuads = 0, nVerts = 0;

    while (std::getline(objfs, line))
    {
        data = _split(line);

        if (!data.empty())
        {
            dataType = data[0];

            if (dataType == "v")
            {
                nVerts++;
                vVec.push_back(
                    {
                        std::stof(data[1]),
                        std::stof(data[2]),
                        std::stof(data[3])
                    }
                );
            }

            else if (dataType == "f")
            {
                
                nTris++;
                if (useMtl) currCol = materialMap[mtlName];
                else  currCol = { (Uint8)(rand() % 255), (Uint8)(rand() % 255) , (Uint8)(rand() % 255) };
  
                fVec.push_back({
                        vVec[std::stoi(data[1]) - 1],
                        vVec[std::stoi(data[2]) - 1],
                        vVec[std::stoi(data[3]) - 1],
                        currCol
                    }
                );

                // Check if quad
                if (data.size() == 5)
                {
                    nQuads++;
                    nTris++;
                    fVec.push_back(
                        {
                            vVec[std::stoi(data[1]) - 1],
                            vVec[std::stoi(data[3]) - 1],
                            vVec[std::stoi(data[4]) - 1],
                            currCol
                        }
                    );
                }
            }

            else if (dataType == "usemtl")
            {
                mtlName = data[1];
            }
        }
    }

    objfs.close();

    std::cout << "Load complete\n\n";
    std::cout << "---OBJECT DATA---\n";
    std::cout << "nTris(with quad splitting) : " << nTris << std::endl << "nQuads : " << nQuads << std::endl << "nVerts : " << nVerts << std::endl << std::endl;

    return fVec;
}