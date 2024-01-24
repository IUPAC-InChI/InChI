# TODO: logging

function test_inchi_regression() {
    local dataset=$1
    local name_output=$2
    local args=$3

    ../../INCHI-1-SRC/INCHI_EXE/bin/Linux/inchi-1 datasets/$dataset results/$name_output NUL -AuxNone -OutErrINCHI -NoLabels -NoWarnings $args

    local inchi_diff=$(diff --strip-trailing-cr -y --suppress-common-lines expected-results/$name_output results/$name_output)
    if [ "$inchi_diff" ]; then
        REGRESSIONS+="\n\nInChI(s) computed on '${dataset}' with arguments '${args}' are not as expected:\n${inchi_diff}"
    fi
}

mkdir -p results
unzip -o test-datasets.zip -d datasets
unzip -o test-results.zip -d expected-results

(cd ../../INCHI-1-SRC/INCHI_EXE/inchi-1/gcc && make -j && make clean)

REGRESSIONS=""

# Standard
test_inchi_regression InChI_TestSet.sdf its-std-01.inc "-Key"
test_inchi_regression InChI_TestSet.sdf its-std-02.inc "-NEWPSOFF"
test_inchi_regression InChI_TestSet.sdf its-std-03.inc "-SNon -Key"
test_inchi_regression InChI_TestSet.sdf its-std-04.inc "-DoNotAddH"
test_inchi_regression InChI_TestSet.sdf its-std-05.inc "-SNon -DoNotAddH"

# Non-standard
test_inchi_regression InChI_TestSet.sdf its-non-std-01.inc "-SUU -SLUUD"
test_inchi_regression InChI_TestSet.sdf its-non-std-02.inc "-SRel -Key"
test_inchi_regression InChI_TestSet.sdf its-non-std-03.inc "-SRac"
test_inchi_regression InChI_TestSet.sdf its-non-std-04.inc "-SUU -SLUUD -SUCF"
test_inchi_regression InChI_TestSet.sdf its-non-std-05.inc "-NEWPSOFF -SRel"
test_inchi_regression InChI_TestSet.sdf its-non-std-06.inc "-NEWPSOFF -SRac"
test_inchi_regression InChI_TestSet.sdf its-non-std-07.inc "-NEWPSOFF -SLUUD -SUCF"
test_inchi_regression InChI_TestSet.sdf its-non-std-08.inc "-FixedH -Key"
test_inchi_regression InChI_TestSet.sdf its-non-std-09.inc "-NEWPSOFF -FixedH"
test_inchi_regression InChI_TestSet.sdf its-non-std-10.inc "-FixedH -SNon"
test_inchi_regression InChI_TestSet.sdf its-non-std-11.inc "-FixedH -SRel"
test_inchi_regression InChI_TestSet.sdf its-non-std-12.inc "-RecMet -Key"
test_inchi_regression InChI_TestSet.sdf its-non-std-13.inc "-NEWPSOFF -RecMe"
test_inchi_regression InChI_TestSet.sdf its-non-std-14.inc "-RecMet -SNon"
test_inchi_regression InChI_TestSet.sdf its-non-std-15.inc "-RecMet -SRel"
test_inchi_regression InChI_TestSet.sdf its-non-std-16.inc "-FixedH -RecMet -Key"
test_inchi_regression InChI_TestSet.sdf its-non-std-17.inc "-NEWPSOFF -FixedH -RecMet"
test_inchi_regression InChI_TestSet.sdf its-non-std-18.inc "-FixedH -RecMet -SNon"
test_inchi_regression InChI_TestSet.sdf its-non-std-19.inc "-FixedH -RecMet -SRel"
test_inchi_regression InChI_TestSet.sdf its-non-std-20.inc "-KET -Key"
test_inchi_regression InChI_TestSet.sdf its-non-std-21.inc "-KET -SNon"
test_inchi_regression InChI_TestSet.sdf its-non-std-22.inc "-KET -SRel"
test_inchi_regression InChI_TestSet.sdf its-non-std-23.inc "-15T -Key"
test_inchi_regression InChI_TestSet.sdf its-non-std-24.inc "-15T -SNon"
test_inchi_regression InChI_TestSet.sdf its-non-std-25.inc "-15T -SRel"
test_inchi_regression InChI_TestSet.sdf its-non-std-26.inc "-KET -15T"
test_inchi_regression InChI_TestSet.sdf its-non-std-27.inc "-NEWPSOFF -KET -15T -Key"
test_inchi_regression InChI_TestSet.sdf its-non-std-28.inc "-NEWPSOFF -KET -15T -SUU -SLUUD"

# Polymers/Zz atoms
test_inchi_regression zzp.sdf zzp.inc "-Key"
test_inchi_regression zzp.sdf zzp-NPZZ.inc "-NPZZ"
test_inchi_regression zzp.sdf zzp-Polymers.inc "-Polymers"
test_inchi_regression zzp.sdf zzp-Polymers-FoldCRU-NPZZ.inc "-Polymers -FoldCRU -NPZZ"
test_inchi_regression zzp.sdf zzp-Polymers105.inc "-Polymers105"
test_inchi_regression zzp.sdf zzp-Polymers105-NPZZ.inc "-Polymers105 -NPZZ"

if [ "$REGRESSIONS" ]; then
    echo -e "$REGRESSIONS"
    exit 1
fi
