#!/bin/bash
# djb-rwth: InChI CMake build tool for Linux/MacOS OSs

clear

function jumpto
{
    label=$1
    cmd=$(sed -n "/$label:/{:a;n;p;ba};" $0 | grep -v ':$')
    eval "$cmd"
    exit
}

ccol='\033[0;96m'
uccol='\033[4;96m'
gcol='\033[0;92m'
dcol='\033[0m'

echo -e "${uccol}InChI CMake build tool${dcol}"
echo
echo -e "Please select an option from the list:"
echo
echo -e "${ccol}1${dcol}: Build CLI project (inchi-1)"
echo -e "${ccol}2${dcol}: Build LIBINCHI library (without binaries)"
echo -e "${ccol}3${dcol}: Build MAIN API project (LIBINCHI w/ corresponding binary INCHI_MAIN)"
echo -e "${ccol}4${dcol}: Build TEST_IXA project (for testing IXA API)"
echo -e "${ccol}5${dcol}: Build MOL2INCHI project (conversion of MOL/SDF files to InChI w/ multithreading support)"
echo -e "${ccol}6${dcol}: Build ALL projects (please refer to options 1-5 above)"
echo

while true; do
    read -p $'\e[1;96mPlease enter the option number [1-6]: \e[0m' cmb_opt
    case $cmb_opt in
        [1] ) jumpto cli; break;;
        [2] ) jumpto lib; break;;
        [3] ) jumpto main; break;;
        [4] ) jumpto ixa; break;;
        [5] ) jumpto mol2inchi; break;;
        [6] ) jumpto allinchi; break;;
        # * ) echo -e "${ccol}Incorrect value -- please enter a number [1-6].${dcol}";;
    esac
done

cli:
echo
echo
echo -e "${ccol}Creating build directory${dcol}"
mkdir ../CMake_build
cd ../CMake_build
echo
echo -e "${ccol}Creating build directory for InChI CLI${dcol}"
if [ -d cli_build ]; then 
    rm cli_build -r -f
fi
mkdir cli_build 
cd cli_build
echo
echo -e "${ccol}Configuring InChI CLI${dcol}"
echo
cmake ../../INCHI-1-SRC/INCHI_EXE/inchi-1/src
echo
echo -e "${ccol}Building InChI CLI${dcol}"
echo
cmake --build .
echo
echo
echo -e "${gcol}========== BUILD SUCCEEDED ==========${dcol}"
echo -e "${ccol}inchi-1 (ELF)${dcol} is located in directory ${ccol}CMake_build/cli_build/bin${dcol}"
echo
exit 0

lib:
echo
echo
echo -e "${ccol}Creating build directory${dcol}"
mkdir ../CMake_build
cd ../CMake_build
echo
echo -e "${ccol}Creating build directory for LIBINCHI${dcol}"
if [ -d libinchi_build ]; then 
    rm libinchi_build -r -f
fi
mkdir libinchi_build
cd libinchi_build
echo
echo -e "${ccol}Configuring LIBINCHI${dcol}"
echo
cmake ../../INCHI-1-SRC/INCHI_API/libinchi/src
echo
echo -e "${ccol}Building LIBINCHI${dcol}"
echo
cmake --build .
echo
echo
echo -e "${gcol}========== BUILD SUCCEEDED ==========${dcol}"
echo -e "${ccol}libinchi.so.1.07${dcol} is located in directory ${ccol}CMake_build/libinchi_build/bin${dcol}"
echo
exit 0

main:
echo
echo
echo -e "${ccol}Creating build directory${dcol}"
mkdir ../CMake_build
cd ../CMake_build
echo
echo -e "${ccol}Creating build directory for INCHI_MAIN${dcol}"
if [ -d inchi_main_build ]; then 
    rm inchi_main_build -r -f
fi
mkdir inchi_main_build
cd inchi_main_build
echo
echo -e "${ccol}Configuring INCHI_MAIN${dcol}"
echo
cmake ../../INCHI-1-SRC/INCHI_API/demos/inchi_main/src
echo
echo -e "${ccol}Building INCHI_MAIN${dcol}"
echo
cmake --build .
echo
echo
echo -e "${gcol}========== BUILD SUCCEEDED ==========${dcol}"
echo -e "${ccol}inchi_main (ELF) & libinchi.so.1.07${dcol} are located in directory ${ccol}CMake_build/inchi_main_build/bin${dcol}"
echo
exit 0

ixa:
echo
echo
echo -e "${ccol}Creating build directory${dcol}"
mkdir ../CMake_build
cd ../CMake_build
echo
echo -e "${ccol}Creating build directory for TEST_IXA${dcol}"
if [ -d test_ixa_build ]; then 
    rm test_ixa_build -r -f
fi
mkdir test_ixa_build
cd test_ixa_build
echo
echo -e "${ccol}Configuring TEST_IXA${dcol}"
echo
cmake ../../INCHI-1-SRC/INCHI_API/demos/test_ixa/src
echo
echo -e "${ccol}Building TEST_IXA${dcol}"
echo
cmake --build .
echo
echo
echo -e "${gcol}========== BUILD SUCCEEDED ==========${dcol}"
echo -e "${ccol}test_ixa (ELF) & libinchi.so.1.07${dcol} are located in directory ${ccol}CMake_build/test_ixa_build/bin${dcol}"
echo
exit 0

mol2inchi:
echo
echo
echo -e "${ccol}Creating build directory${dcol}"
mkdir ../CMake_build
cd ../CMake_build
echo
echo -e "${ccol}Creating build directory for MOL2INCHI${dcol}"
if [ -d mol2inchi_build ]; then 
    rm mol2inchi_build -r -f
fi
mkdir mol2inchi_build
cd mol2inchi_build
echo
echo -e "${ccol}Configuring MOL2INCHI${dcol}"
echo
cmake ../../INCHI-1-SRC/INCHI_API/demos/mol2inchi/src
echo
echo -e "${ccol}Building MOL2INCHI${dcol}"
echo
cmake --build .
echo
echo
echo -e "${gcol}========== BUILD SUCCEEDED ==========${dcol}"
echo -e "${ccol}mol2inchi (ELF) & libinchi.so.1.07${dcol} are located in directory ${ccol}CMake_build/mol2inchi_build/bin${dcol}"
echo
exit 0

allinchi:
echo
echo
echo -e "${ccol}Creating build directory${dcol}"
mkdir ../CMake_build
cd ../CMake_build
echo
echo -e "${ccol}Creating build directory for InChI CLI${dcol}"
if [ -d cli_build ]; then 
    rm cli_build -r -f
fi
mkdir cli_build
cd cli_build
echo
echo -e "${ccol}Configuring InChI CLI${dcol}"
echo
cmake ../../INCHI-1-SRC/INCHI_EXE/inchi-1/src
echo
echo -e "${ccol}Building InChI CLI${dcol}"
echo
cmake --build .
echo
cd ..
echo
echo -e "${ccol}Creating build directory for LIBINCHI${dcol}"
if [ -d libinchi_build ]; then 
    rm libinchi_build -r -f
fi
mkdir libinchi_build
cd libinchi_build
echo
echo -e "${ccol}Configuring LIBINCHI${dcol}"
echo
cmake ../../INCHI-1-SRC/INCHI_API/libinchi/src
echo
echo -e "${ccol}Building LIBINCHI${dcol}"
echo
cmake --build .
echo
cd ..
echo
echo -e "${ccol}Creating build directory for INCHI_MAIN${dcol}"
if [ -d inchi_main_build ]; then 
    rm inchi_main_build -r -f
fi
mkdir inchi_main_build
cd inchi_main_build
echo
echo -e "${ccol}Configuring INCHI_MAIN${dcol}"
echo
cmake ../../INCHI-1-SRC/INCHI_API/demos/inchi_main/src
echo
echo -e "${ccol}Building INCHI_MAIN${dcol}"
echo
cmake --build .
echo
cd ..
echo
echo -e "${ccol}Creating build directory for TEST_IXA${dcol}"
if [ -d test_ixa_build ]; then 
    rm test_ixa_build -r -f
fi
mkdir test_ixa_build
cd test_ixa_build
echo
echo -e "${ccol}Configuring TEST_IXA${dcol}"
echo
cmake ../../INCHI-1-SRC/INCHI_API/demos/test_ixa/src
echo
echo -e "${ccol}Building TEST_IXA${dcol}"
echo
cmake --build .
echo
cd ..
echo
echo -e "${ccol}Creating build directory for MOL2INCHI${dcol}"
if [ -d mol2inchi_build ]; then 
    rm mol2inchi_build -r -f
fi
mkdir mol2inchi_build
cd mol2inchi_build
echo
echo -e "${ccol}Configuring MOL2INCHI${dcol}"
echo
cmake ../../INCHI-1-SRC/INCHI_API/demos/mol2inchi/src
echo
echo -e "${ccol}Building MOL2INCHI${dcol}"
echo
cmake --build .
echo
cd ..
echo
echo
echo -e "${gcol}========== BUILD SUCCEEDED ==========${dcol}"
echo -e "${ccol}inchi-1 (ELF)${dcol} is located in directory ${ccol}CMake_build/cli_build/bin${dcol}"
echo -e "${ccol}libinchi.so.1.07${dcol} is located in directory ${ccol}CMake_build/libinchi_build/bin${dcol}"
echo -e "${ccol}inchi_main (ELF) & libinchi.so.1.07${dcol} are located in directory ${ccol}CMake_build/inchi_main_build/bin${dcol}"
echo -e "${ccol}test_ixa (ELF) & libinchi.so.1.07${dcol} are located in directory ${ccol}CMake_build/test_ixa_build/bin${dcol}"
echo -e "${ccol}mol2inchi (ELF) & libinchi.so.1.07${dcol} are located in directory ${ccol}CMake_build/mol2inchi_build/bin${dcol}"
echo
exit 0