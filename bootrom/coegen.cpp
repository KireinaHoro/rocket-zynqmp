// COE generator by Darren Anderson
// bugfix for width > 8 by KireinaHoro <i@jsteward.moe>
#include <boost/program_options.hpp>
#include <iostream>
#include <fstream>
#include <string>
#include <iomanip>

namespace pOptions = boost::program_options;

void DoConversion(std::string fname, int width, int depth)
{
    std::ifstream inputfile(fname.c_str(), std::ios::in | std::ios::binary | std::ios::ate);
    unsigned char* buffer;
    std::string outputfname;
    size_t filesize;
    unsigned int memlength = depth * (width / 8);

    if(inputfile.is_open())
    {
        filesize = inputfile.tellg();
        buffer = new unsigned char[filesize];
        std::cout << "Input file found. Size = " << filesize << " bytes." << std::endl;

        if(memlength >= filesize)
        {
            outputfname = fname;
            outputfname.append(".coe");
            std::cout << "Output file: " << outputfname << std::endl;

            std::ofstream outputfile(outputfname.c_str(), std::ios::out);

            if(outputfile.is_open())
            {
                //Everything's good. Let's convert the file.
                inputfile.seekg(0, std::ios::beg);
                inputfile.read((char*)buffer, filesize);
                std::cout << "Input file read into buffer." << std::endl;

                std::cout << "Generating COE file. Width = " << width << " Depth = " << depth << std::endl;

                //outputfile << "# Created by COEGen. Width = " << width << " Depth = " << depth << std::endl;
                outputfile << "memory_initialization_radix=16;" << std::endl;
                outputfile << "memory_initialization_vector=" << std::endl;

                int wcnt, dcnt;
                unsigned int bufcnt;

                //The actual conversion process.
                for(dcnt=0;dcnt<depth;dcnt++)
                {
					// use little endian
                    for(wcnt=(width/8)-1;wcnt>=0;wcnt--)
                    {
                        bufcnt = (dcnt * (width/8))+wcnt;

                        if(bufcnt <= filesize)
                        {
                            outputfile << std::hex << std::setw(2) << std::setfill('0') << (int)buffer[bufcnt];
                        }
                        else
                        {
                            outputfile << "00";
                        }
                    }
                    if(dcnt != (depth-1))
                    {
                        outputfile << "," << std::endl;
                    }
                    else
                    {
                       outputfile << ";" << std::endl;
                    }
                }

                std::cout << "Output file created." << std::endl;
                inputfile.close();
                outputfile.close();

            }
            else
            {
                std::cout << "Error opening output file." << std::endl;
                inputfile.close();
                return;
            }

        }
        else
        {
            std::cout << "Error, specified depth of memory is not enough to contain the input file." << std::endl;
            inputfile.close();
            return;
        }
    }
    else
    {
        std::cout << "Error opening file." << std::endl;
        return;
    }

}

int main (int argc, char* argv[])
{
    std::cout << "------------------------------------------------" << std::endl;
    std::cout << "COEGen by Darren Anderson" << std::endl;
    std::cout << "------------------------------------------------" << std::endl;
    std::cout << "A utility for initialising Xilinx FPGA B-RAM" << std::endl;
    std::cout << "------------------------------------------------" << std::endl;
    std::cout << "Contact me at http://wornwinter.wordpress.com/" << std::endl;
    std::cout << "------------------------------------------------" << std::endl;

    try
    {
        pOptions::options_description desc("Program Options");
        desc.add_options()
            ("help", "Show this screen.")
            ("file", pOptions::value<std::string>(), "File to convert.")
            ("depth", pOptions::value<int>(), "Depth of memory to initialise.")
            ("width", pOptions::value<int>(), "Width of memory to initialise in bits. Must be a multiple of 8.");

        pOptions::variables_map vm;
        pOptions::store(pOptions::parse_command_line(argc, argv, desc), vm);
        pOptions::notify(vm);

        if(vm.count("help"))
        {
            std::cout << desc << std::endl;
            std::cout << "Example:" << std::endl;
            std::cout << "$ COEGen --file binary.bin --width 8 --depth 256" << std::endl;
            std::cout << "This will output 'binary.bin.coe'" << std::endl;
            std::cout << "------------------------------------------------" << std::endl;
        }

        if(vm.count("file"))
        {
            if(vm.count("depth") && vm.count("width"))
            {
                int width, depth;
                width = vm["width"].as<int>();
                depth = vm["depth"].as<int>();
                std::string filename = vm["file"].as<std::string>();

                if((width % 8) != 0)
                {
                    std::cout << "Error, width must be a multiple of 8." << std::endl;
                    return 0;
                }
                else
                {
                    DoConversion(filename, width, depth);
                    return 0;
                }
            }
            else
            {
                std::cout << "Error, you must set both width and depth. Start program with '--help' for more information." << std::endl;
                return 0;
            }
        }
        else {
            std::cout << desc << std::endl;
            std::cout << "Example:" << std::endl;
            std::cout << "$ COEGen --file binary.bin --width 8 --depth 256" << std::endl;
            std::cout << "This will output 'binary.bin.coe'" << std::endl;
            std::cout << "------------------------------------------------" << std::endl;
        }


    }
    catch(std::exception& e)
    {
        std::cerr << "Error: " << e.what() << std::endl;
    }

}
