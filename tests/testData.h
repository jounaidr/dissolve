// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2023 Team Dissolve and contributors

#include "classes/coreData.h"
#include "classes/species.h"
#include "data/elements.h"
#include "io/import/data1D.h"
#include "io/import/data3D.h"
#include "io/import/forces.h"
#include "main/dissolve.h"
#include "math/data3D.h"
#include "math/error.h"
#include "math/sampledData1D.h"
#include "math/sampledDouble.h"
#include "math/sampledVector.h"
#include <gtest/gtest.h>

namespace UnitTest
{

// Flags that can modify how a test is setUp.  This can be useful
// for masking tests that are known to be failing, but cannot be
// resolved at this juncture.  The indices must be unique powers
// of two in order for masks to be composable.
//
// Eventually, once we move to C++23, this can be replaced by a
// std::bitset, but bitset isn't constexpr until then.
enum TestFlags
{
    TomlFailure = 1, // tests where the TOML testing is known to fail
};

void compareToml(std::string location, SerialisedValue toml, SerialisedValue toml2)
{
    if (toml.is_table())
    {
        ASSERT_TRUE(toml2.is_table()) << location;
        for (auto &[k, v] : toml.as_table())
        {
            ASSERT_TRUE(toml2.contains(k)) << location << "." << k << std::endl << "Expected:" << std::endl << toml[k];
            compareToml(fmt::format("{}.{}", location, k), v, toml2.at(k));
        }
    }
    else if (toml.is_array())
    {
        auto arr = toml.as_array();
        auto arr2 = toml2.as_array();
        ASSERT_EQ(arr.size(), arr2.size()) << location << std::endl << "Expected" << std::endl << toml;
        for (int i = 0; i < arr.size(); ++i)
            compareToml(fmt::format("{}[{}]", location, i), arr[i], arr2[i]);
    }
    else
    {
        EXPECT_EQ(toml, toml2) << location;
    }
}

class DissolveSystemTest
{
    public:
    DissolveSystemTest() : dissolve_(coreData_) { dissolve_.setRestartFileFrequency(0); };

    /*
     * Dissolve & CoreData
     */
    private:
    CoreData coreData_;
    Dissolve dissolve_;
    // Whether to perform rewrite checks on setUp
    bool rewriteCheck_{true};
    // Function to execute to perform additional setup prior to prepare()
    std::function<void(Dissolve &D, CoreData &C)> additionalSetUp_;

    public:
    // Return the Dissolve object
    Dissolve &dissolve() { return dissolve_; }
    // Return the CoreData object
    CoreData &coreData() { return coreData_; }

    /*
     * SetUp & Input
     */
    public:
    // Set up simulation ready for running, calling any additional setup function if already set
    template <int flags = 0> void setUp(std::string_view inputFile)
    {

        dissolve_.clear();
        if constexpr (Dissolve::toml_testing_flag || !(flags & TomlFailure))
        {
            SerialisedValue toml;
            {
                CoreData otherCoreData;
                Dissolve otherDissolve{otherCoreData};

                if (!otherDissolve.loadInput(inputFile))
                    throw(std::runtime_error(fmt::format("Input file '{}' failed to load correctly.\n", inputFile)));
                if (rewriteCheck_)
                {
                    auto newInput = fmt::format("{}/TestOutput_{}.{}.rewrite", DissolveSys::beforeLastChar(inputFile, '/'),
                                                DissolveSys::afterLastChar(inputFile, '/'),
                                                ::testing::UnitTest::GetInstance()->current_test_info()->name());
                    if (!otherDissolve.saveInput(newInput))
                        throw(std::runtime_error(fmt::format("Input file '{}' failed to rewrite correctly.\n", inputFile)));

                    otherDissolve.clear();
                    if (!otherDissolve.loadInput(newInput))
                        throw(std::runtime_error(fmt::format("Input file '{}' failed to reload correctly.\n", newInput)));
                }

                // Run any other additional setup functions
                if (additionalSetUp_)
                    additionalSetUp_(otherDissolve, otherCoreData);

                if (!otherDissolve.prepare())
                    throw(std::runtime_error("Failed to prepare simulation.\n"));

                toml = otherDissolve.serialise();
            }

            dissolve_.deserialise(toml);
            dissolve_.setInputFilename(std::string(inputFile));
            auto repeat = dissolve_.serialise();

            // Run any other additional setup functions
            if (additionalSetUp_)
                additionalSetUp_(dissolve_, coreData_);

            if (!dissolve_.prepare())
                throw(std::runtime_error("Failed to prepare simulation.\n"));

            compareToml("", toml, repeat);
        }
        else
        {
            if (!dissolve_.loadInput(inputFile))
                throw(std::runtime_error(fmt::format("Input file '{}' failed to load correctly.\n", inputFile)));
            if (rewriteCheck_)
            {
                auto newInput = fmt::format("{}/TestOutput_{}.{}.rewrite", DissolveSys::beforeLastChar(inputFile, '/'),
                                            DissolveSys::afterLastChar(inputFile, '/'),
                                            ::testing::UnitTest::GetInstance()->current_test_info()->name());
                if (!dissolve_.saveInput(newInput))
                    throw(std::runtime_error(fmt::format("Input file '{}' failed to rewrite correctly.\n", inputFile)));

                dissolve_.clear();
                if (!dissolve_.loadInput(newInput))
                    throw(std::runtime_error(fmt::format("Input file '{}' failed to reload correctly.\n", newInput)));
            }

            // Run any other additional setup functions
            if (additionalSetUp_)
                additionalSetUp_(dissolve_, coreData_);

            if (!dissolve_.prepare())
                throw(std::runtime_error("Failed to prepare simulation.\n"));
        }
    }

    template <int flags = 0>
    void setUp(std::string_view inputFile, const std::function<void(Dissolve &D, CoreData &C)> &additionalSetUp)
    {
        additionalSetUp_ = additionalSetUp;
        setUp<flags>(inputFile);
    }
    // Load restart file
    void loadRestart(std::string_view restartFile)
    {
        if (!dissolve_.loadRestart(restartFile))
            throw(std::runtime_error(fmt::format("Restart file '{}' failed to load correctly.\n", restartFile)));
    }

    /*
     * Restart Iterator
     */
    public:
    // Iterate for set number of steps but chunked into smaller runs to test restart capability
    template <int flags = 0> bool iterateRestart(const int nIterations, const int chunkSize = 20)
    {
        // Set the restart file frequency, and grab the input and restart filenames
        dissolve_.setRestartFileFrequency(chunkSize);
        auto inputFile = std::string(dissolve_.inputFilename());
        auto restartFile = std::string(dissolve_.restartFilename());
        rewriteCheck_ = false;

        auto iterationsDone = 0;
        while (iterationsDone != nIterations)
        {
            // Run another chunk of iterations - the whole chunk if possible, otherwise the remainder
            auto itersToDo = (nIterations - iterationsDone) >= chunkSize ? chunkSize : nIterations - iterationsDone;
            if (!dissolve_.iterate(itersToDo))
                return false;

            iterationsDone += itersToDo;

            // Clear and reload restart file
            if (iterationsDone != nIterations)
            {
                fmt::print("Resetting at iteration {}...\n", iterationsDone);
                setUp<flags>(inputFile);
                loadRestart(restartFile);
            }
        }

        return true;
    }

    /*
     * Module Helpers
     */
    public:
    // Set enabled status for named module
    static void setModuleEnabled(std::string_view name, bool enabled)
    {
        auto *module = Module::find(name);
        if (!module)
            throw(std::runtime_error(fmt::format("Module '{}' does not exist.\n", name)));
        module->setEnabled(enabled);
    }
    // Find and return named module
    template <class M> M *getModule(std::string_view name)
    {
        auto *module = Module::find(name);
        if (!module)
            throw(std::runtime_error(fmt::format("Module '{}' does not exist.\n", name)));
        auto *castModule = dynamic_cast<M *>(module);
        if (!castModule)
            throw(std::runtime_error(
                fmt::format("Module '{}' did not cast to the target type '{}' (it is of Module type '{}').\n", name,
                            typeid(M).name(), ModuleTypes::moduleType(module->type()))));
        return castModule;
    }

    /*
     * Checks
     */
    public:
    // Test simple double
    [[nodiscard]] static bool checkDouble(std::string_view quantity, double A, double B, double threshold)
    {
        auto delta = fabs(A - B);
        auto isOK = delta <= threshold;
        Messenger::print("Reference {} delta with correct value is {:15.9e} and is {} (threshold is {:10.3e})\n", quantity,
                         delta, isOK ? "OK" : "NOT OK", threshold);
        return isOK;
    }
    // Test sampled double
    [[nodiscard]] bool checkSampledDouble(std::string_view quantity, std::string_view tag, double B, double threshold)
    {
        // Locate the target reference data
        const auto &A = dissolve_.processingModuleData().retrieve<SampledDouble>(tag);

        return checkDouble(quantity, A.value(), B, threshold);
    }
    // Test Data1D
    [[nodiscard]] static bool checkData1D(const Data1D &dataA, std::string_view nameA, const Data1D &dataB,
                                          std::string_view nameB, double tolerance = 5.0e-3,
                                          Error::ErrorType errorType = Error::ErrorType::EuclideanError)
    {
        // Generate the error estimate and compare against the threshold value
        auto error = Error::error(errorType, dataA, dataB).error;
        auto notOK = isnan(error) || error > tolerance;
        Messenger::print("Internal data '{}' has error of {:7.3e} with data '{}' and is {} (threshold is {:6.3e}).\n", nameA,
                         error, nameB, notOK ? "NOT OK" : "OK", tolerance);
        return !notOK;
    }
    // Test Data1D (by tag and external file data)
    [[nodiscard]] bool checkData1D(std::string_view tagA, Data1DImportFileFormat externalFileFormat, double tolerance = 5.0e-3,
                                   Error::ErrorType errorType = Error::ErrorType::EuclideanError)
    {
        auto optDataA = dissolve_.processingModuleData().searchBase<Data1DBase, Data1D, SampledData1D>(tagA);
        if (!optDataA)
            throw(std::runtime_error(fmt::format("No data with tag '{}' exists.\n", tagA)));

        Data1D dataB;
        if (!externalFileFormat.fileExists() || !externalFileFormat.importData(dataB))
            throw(std::runtime_error(fmt::format("External data '{}' failed to load.\n", externalFileFormat.filename())));

        return checkData1D(optDataA->get(), tagA, dataB, externalFileFormat.filename(), tolerance, errorType);
    }
    // Test Data1D (by tags)
    [[nodiscard]] bool checkData1D(std::string_view tagA, std::string_view tagB, double tolerance = 5.0e-3,
                                   Error::ErrorType errorType = Error::ErrorType::EuclideanError)
    {
        auto optDataA = dissolve_.processingModuleData().searchBase<Data1DBase, Data1D, SampledData1D>(tagA);
        if (!optDataA)
            throw(std::runtime_error(fmt::format("No data with tag '{}' exists.\n", tagA)));

        auto optDataB = dissolve_.processingModuleData().searchBase<Data1DBase, Data1D, SampledData1D>(tagB);
        if (!optDataB)
            throw(std::runtime_error(fmt::format("No data with tag '{}' exists.\n", tagB)));

        return checkData1D(optDataA->get(), tagA, optDataB->get(), tagB, tolerance, errorType);
    }
    // Test Data3D
    [[nodiscard]] static bool checkData3D(const Data3D &dataA, std::string_view nameA, const Data3D &dataB,
                                          std::string_view nameB, double tolerance = 5.0e-3,
                                          Error::ErrorType errorType = Error::ErrorType::EuclideanError)
    {
        // Generate the error estimate and compare against the threshold value
        auto error = Error::error(errorType, dataA.values().linearArray(), dataB.values().linearArray()).error;
        auto notOK = isnan(error) || error > tolerance;
        Messenger::print("Internal data '{}' has error of {:7.3f} with external data '{}' and is {} (threshold is {:6.3e})\n\n",
                         nameA, error, nameB, notOK ? "NOT OK" : "OK", tolerance);

        return !notOK;
    }
    // Test Data3D (by tag and external file data)
    [[nodiscard]] bool checkData3D(std::string_view tagA, Data3DImportFileFormat externalFileFormat, double tolerance = 5.0e-3,
                                   Error::ErrorType errorType = Error::ErrorType::EuclideanError)
    {
        auto optDataA = dissolve_.processingModuleData().search<const Data3D>(tagA);
        if (!optDataA)
            throw(std::runtime_error(fmt::format("No data with tag '{}' exists.\n", tagA)));

        Data3D dataB;
        if (!externalFileFormat.fileExists() || !externalFileFormat.importData(dataB))
            throw(std::runtime_error(fmt::format("External data '{}' failed to load.\n", externalFileFormat.filename())));

        return checkData3D(*optDataA, tagA, dataB, externalFileFormat.filename(), tolerance, errorType);
    }
    // Test Data3D (by tags)
    [[nodiscard]] bool checkData3D(std::string_view tagA, std::string_view tagB, double tolerance = 5.0e-3,
                                   Error::ErrorType errorType = Error::ErrorType::EuclideanError)
    {
        auto optDataA = dissolve_.processingModuleData().search<const Data3D>(tagA);
        if (!optDataA)
            throw(std::runtime_error(fmt::format("No data with tag '{}' exists.\n", tagA)));

        auto optDataB = dissolve_.processingModuleData().search<const Data3D>(tagB);
        if (!optDataB)
            throw(std::runtime_error(fmt::format("No data with tag '{}' exists.\n", tagB)));

        return checkData3D(optDataA->get(), tagA, optDataB->get(), tagB, tolerance, errorType);
    }
    // Test SampledVector data
    [[nodiscard]] bool checkSampledVector(std::string_view tag, const std::vector<double> &referenceData,
                                          double tolerance = 5.0e-3,
                                          Error::ErrorType errorType = Error::ErrorType::EuclideanError)
    {
        // Locate the target reference data
        auto optData = dissolve_.processingModuleData().search<const SampledVector>(tag);
        if (!optData)
            throw(std::runtime_error(fmt::format("No data with tag '{}' exists.\n", tag)));
        const auto &data = optData->get();

        // Generate the error estimate and compare against the threshold value
        auto error = Error::error(errorType, data.values(), referenceData).error;
        auto notOK = isnan(error) || error > tolerance;
        Messenger::print("Target data '{}' has error of {:7.3e} with reference data and is {} (threshold is {:6.3e})\n\n", tag,
                         error, notOK ? "NOT OK" : "OK", tolerance);
        return !notOK;
    }
    // Test Vec3 data
    static void checkVec3(const Vec3<double> &A, const Vec3<double> &B, double tolerance = 1.0e-6)
    {
        EXPECT_NEAR(A.x, B.x, tolerance);
        EXPECT_NEAR(A.y, B.y, tolerance);
        EXPECT_NEAR(A.z, B.z, tolerance);
    }
    // Test Vec3 vector data
    static void checkVec3Vector(const std::vector<Vec3<double>> &A, const std::vector<Vec3<double>> &B,
                                double tolerance = 1.0e-6)
    {
        ASSERT_EQ(A.size(), B.size());
        for (auto n = 0; n < A.size(); ++n)
            checkVec3(A[n], B[n], tolerance);
    }
    // Test Vec3 vector data (by tag and external data)
    void checkVec3Vector(std::string_view tag, ForceImportFileFormat externalForces, double tolerance)
    {
        auto &vec = dissolve_.processingModuleData().value<std::vector<Vec3<double>>>(tag);
        std::vector<Vec3<double>> B(vec.size());
        ASSERT_TRUE(externalForces.importData(B));
        checkVec3Vector(vec, B, tolerance);
    }
    // Test species atom type
    static void checkSpeciesAtomType(const Species *sp, int atomIndex, std::string_view atomTypeName)
    {
        ASSERT_TRUE(atomIndex >= 0 && atomIndex < sp->nAtoms());
        auto &spAtom = sp->atom(atomIndex);
        auto at = spAtom.atomType();
        ASSERT_TRUE(at);
        EXPECT_EQ(at->name(), atomTypeName);
    }
    // Test interaction parameters
    template <class Intra>
    void checkIntramolecularTerms(const std::string &termInfo, const InteractionPotential<Intra> &expectedParams,
                                  const InteractionPotential<Intra> &actualParams, double tolerance = 1.0e-6)
    {
        Messenger::print("Testing intramolecular interaction: {}...\n", termInfo);
        EXPECT_EQ(Intra::forms().keyword(actualParams.form()), Intra::forms().keyword(expectedParams.form()));
        EXPECT_EQ(actualParams.nParameters(), expectedParams.nParameters());
        for (auto &&[current, expected] : zip(actualParams.parameters(), expectedParams.parameters()))
            EXPECT_NEAR(current, expected, tolerance);
    }
    // Test species bond term
    void checkSpeciesIntramolecular(const Species *sp, std::vector<int> atoms,
                                    const InteractionPotential<BondFunctions> &expectedParams, double tolerance = 1.0e-6)
    {
        ASSERT_TRUE(atoms.size() == 2);
        const auto &b = sp->getBond(atoms[0], atoms[1]);
        if (!b)
            throw(std::runtime_error(fmt::format("No bond {} exists in species '{}'.\n", joinStrings(atoms, "-"), sp->name())));
        checkIntramolecularTerms(fmt::format("bond {}", joinStrings(atoms, "-")), expectedParams,
                                 b->get().interactionPotential(), tolerance);
    }
    // Test species angle term
    void checkSpeciesIntramolecular(const Species *sp, std::vector<int> atoms,
                                    const InteractionPotential<AngleFunctions> &expectedParams, double tolerance = 1.0e-6)
    {
        ASSERT_TRUE(atoms.size() == 3);
        const auto &a = sp->getAngle(atoms[0], atoms[1], atoms[2]);
        if (!a)
            throw(
                std::runtime_error(fmt::format("No angle {} exists in species '{}'.\n", joinStrings(atoms, "-"), sp->name())));
        checkIntramolecularTerms(fmt::format("angle {}", joinStrings(atoms, "-")), expectedParams,
                                 a->get().interactionPotential(), tolerance);
    }
    // Test species torsion / improper term
    void checkSpeciesIntramolecular(const Species *sp, std::vector<int> atoms,
                                    const InteractionPotential<TorsionFunctions> &expectedParams, double tolerance = 1.0e-6)
    {
        ASSERT_TRUE(atoms.size() == 4);
        const auto &t = sp->getTorsion(atoms[0], atoms[1], atoms[2], atoms[3]);
        const auto &i = sp->getImproper(atoms[0], atoms[1], atoms[2], atoms[3]);
        if (!t && !i)
            throw(std::runtime_error(
                fmt::format("No torsion or improper {} exists in species '{}'.\n", joinStrings(atoms, "-"), sp->name())));
        else if (t)
            checkIntramolecularTerms(fmt::format("torsion {}", joinStrings(atoms, "-")), expectedParams,
                                     t->get().interactionPotential(), tolerance);
        else
            checkIntramolecularTerms(fmt::format("improper {}", joinStrings(atoms, "-")), expectedParams,
                                     i->get().interactionPotential(), tolerance);
    }
};

// Return argon test species
const Species &argonSpecies()
{
    static Species argon_;
    if (argon_.nAtoms() == 0)
    {
        argon_.setName("Argon");
        argon_.addAtom(Elements::Ar, {0.000000, 0.000000, 0.000000});
    }

    return argon_;
}

// Return diatomic test species
const Species &diatomicSpecies()
{
    static Species diatomic_;
    if (diatomic_.nAtoms() == 0)
    {
        diatomic_.setName("Diatomic");
        diatomic_.addAtom(Elements::N, {0.000000, 0.000000, 0.000000});
        diatomic_.addAtom(Elements::O, {0.000000, 1.100000, 0.000000});
        diatomic_.addMissingBonds();
    }

    return diatomic_;
}

// Return methane test species
const Species &methaneSpecies()
{
    static Species methane_;
    if (methane_.nAtoms() == 0)
    {
        methane_.setName("Methane");
        methane_.addAtom(Elements::C, {0.000000, 0.000000, 0.000000});
        methane_.addAtom(Elements::H, {0.000000, 1.080000, 0.000000});
        methane_.addAtom(Elements::H, {0.000000, -0.360511, -1.018053});
        methane_.addAtom(Elements::H, {0.881973, -0.359744, 0.509026});
        methane_.addAtom(Elements::H, {-0.881973, -0.359744, 0.509026});
        methane_.addMissingBonds();
    }
    return methane_;
}

// Return ethane test species
const Species &ethaneSpecies()
{
    static Species ethane_;
    if (ethane_.nAtoms() == 0)
    {
        ethane_.setName("Ethane");
        ethane_.addAtom(Elements::C, {0.000000, 0.000000, 0.000000});
        ethane_.addAtom(Elements::C, {0.000000, 1.540000, 0.000000});
        ethane_.addAtom(Elements::H, {0.000000, -0.360511, -1.018053});
        ethane_.addAtom(Elements::H, {0.881973, -0.359744, 0.509026});
        ethane_.addAtom(Elements::H, {-0.881973, -0.359744, 0.509026});
        ethane_.addAtom(Elements::H, {0.000000, 1.900511, 1.018053});
        ethane_.addAtom(Elements::H, {0.881973, 1.899744, -0.509026});
        ethane_.addAtom(Elements::H, {-0.881973, 1.899744, -0.509026});
        ethane_.addMissingBonds();
    }
    return ethane_;
}

// Construct methanol test species
const Species &methanolSpecies()
{
    static Species methanol_;
    if (methanol_.nAtoms() == 0)
    {
        methanol_.setName("Methanol");
        methanol_.addAtom(Elements::C, {0.000078, -0.353546, -0.169274});
        methanol_.addAtom(Elements::H, {0.000078, -0.714057, -1.187327});
        methanol_.addAtom(Elements::H, {0.882051, -0.713290, 0.339752});
        methanol_.addAtom(Elements::H, {-0.881895, -0.713290, 0.339752});
        methanol_.addAtom(Elements::O, {-0.000129, 1.066453, -0.170343});
        methanol_.addAtom(Elements::H, {-0.000177, 1.394279, 0.753199});
        methanol_.addMissingBonds();
    }
    return methanol_;
}

// Return 1,4-difluorobenzene test species
const Species &difluorobenzeneSpecies()
{
    static Species dfb_;
    if (dfb_.nAtoms() == 0)
    {
        dfb_.setName("Difluorobenzene");
        dfb_.addAtom(Elements::C, {-1.399000e+00, 1.600000e-01, 0.000000e+00});
        dfb_.addAtom(Elements::C, {-5.610000e-01, 1.293000e+00, 0.000000e+00});
        dfb_.addAtom(Elements::C, {8.390000e-01, 1.132000e+00, 0.000000e+00});
        dfb_.addAtom(Elements::C, {1.399000e+00, -1.600000e-01, 0.000000e+00});
        dfb_.addAtom(Elements::C, {5.600000e-01, -1.293000e+00, 0.000000e+00});
        dfb_.addAtom(Elements::C, {-8.390000e-01, -1.132000e+00, 0.000000e+00});
        dfb_.addAtom(Elements::F, {1.483000e+00, 2.001000e+00, 0.000000e+00});
        dfb_.addAtom(Elements::H, {2.472000e+00, -2.840000e-01, 0.000000e+00});
        dfb_.addAtom(Elements::H, {9.910000e-01, -2.284000e+00, 0.000000e+00});
        dfb_.addAtom(Elements::F, {-1.483000e+00, -2.000000e+00, 0.000000e+00});
        dfb_.addAtom(Elements::H, {-2.472000e+00, 2.820000e-01, 0.000000e+00});
        dfb_.addAtom(Elements::H, {-9.900000e-01, 2.284000e+00, 0.000000e+00});
        dfb_.addMissingBonds();
    }
    return dfb_;
}

// Return rings test species
const Species &ringsSpecies()
{
    static Species rings_;
    if (rings_.nAtoms() == 0)
    {
        rings_.setName("Rings");
        rings_.addAtom(Elements::C, {-0.208763, -0.975789, -0.095504});
        rings_.addAtom(Elements::C, {0.045228, 0.295623, -0.380595});
        rings_.addAtom(Elements::C, {1.300571, 0.857602, -0.348819});
        rings_.addAtom(Elements::C, {2.360934, -0.005443, -0.027080});
        rings_.addAtom(Elements::C, {2.111776, -1.366987, 0.240107});
        rings_.addAtom(Elements::C, {0.805131, -1.869462, 0.219207});
        rings_.addAtom(Elements::H, {1.468477, 1.902060, -0.572229});
        rings_.addAtom(Elements::H, {3.373539, 0.371667, 0.002685});
        rings_.addAtom(Elements::H, {2.939423, -2.029266, 0.465935});
        rings_.addAtom(Elements::H, {0.596002, -2.910744, 0.426097});
        rings_.addAtom(Elements::N, {-1.289150, 0.716693, -0.702798});
        rings_.addAtom(Elements::C, {-1.646291, -0.686037, -0.306734});
        rings_.addAtom(Elements::C, {-1.785682, 1.724996, 0.228719});
        rings_.addAtom(Elements::H, {-1.619801, 1.449463, 1.291036});
        rings_.addAtom(Elements::H, {-1.288061, 2.694431, 0.011695});
        rings_.addAtom(Elements::H, {-2.873536, 1.867631, 0.046101});
        rings_.addAtom(Elements::H, {-2.075934, -1.268474, -1.147377});
        rings_.addAtom(Elements::H, {-2.213863, -0.767963, 0.649555});
        rings_.addMissingBonds();
    }
    return rings_;
}

// Return geometry test species
const Species &geometricSpecies()
{
    static Species geometric_;
    if (geometric_.nAtoms() == 0)
    {
        geometric_.setName("Geometric");
        geometric_.addAtom(Elements::P, {-5.824000, 3.981000, 0.000000});
        geometric_.addAtom(Elements::P, {-1.824000, 3.981000, 0.000000});
        geometric_.addAtom(Elements::F, {-1.824000, 4.981000, 0.000000});
        geometric_.addAtom(Elements::P, {2.176000, 3.981000, 0.000000});
        geometric_.addAtom(Elements::F, {2.176000, 4.981000, 0.000000});
        geometric_.addAtom(Elements::F, {2.176000, 2.981000, 0.000000});
        geometric_.addAtom(Elements::P, {6.176000, 4.003000, 0.000000});
        geometric_.addAtom(Elements::F, {6.176000, 5.003000, 0.000000});
        geometric_.addAtom(Elements::F, {5.241000, 3.463000, 0.000000});
        geometric_.addAtom(Elements::F, {7.111000, 3.463000, 0.000000});
        geometric_.addAtom(Elements::P, {-5.000000, 0.000000, 0.000000});
        geometric_.addAtom(Elements::F, {-5.000000, 1.080000, 0.000000});
        geometric_.addAtom(Elements::F, {-5.000000, -0.361000, -1.018000});
        geometric_.addAtom(Elements::F, {-4.118000, -0.360000, 0.509000});
        geometric_.addAtom(Elements::F, {-5.882000, -0.360000, 0.509000});
        geometric_.addAtom(Elements::P, {0.000000, 0.000000, 0.000000});
        geometric_.addAtom(Elements::F, {0.000000, 1.200000, 0.000000});
        geometric_.addAtom(Elements::F, {0.000000, -1.200000, 0.000000});
        geometric_.addAtom(Elements::F, {-1.280000, 0.000000, 0.000000});
        geometric_.addAtom(Elements::F, {1.280000, 0.000000, 0.000000});
        geometric_.addAtom(Elements::F, {0.000000, 0.000000, -1.200000});
        geometric_.addAtom(Elements::F, {0.000000, 0.000000, 1.200000});
        geometric_.addAtom(Elements::P, {4.800000, -0.020000, 0.000000});
        geometric_.addAtom(Elements::F, {4.800000, 1.180000, 0.000000});
        geometric_.addAtom(Elements::F, {3.865000, -0.560000, 0.000000});
        geometric_.addAtom(Elements::F, {5.735000, -0.560000, 0.000000});
        geometric_.addAtom(Elements::F, {4.800000, -0.020000, -1.200000});
        geometric_.addAtom(Elements::F, {4.800000, -0.020000, 1.200000});
        geometric_.addAtom(Elements::P, {-3.000000, -4.300000, 0.000000});
        geometric_.addAtom(Elements::F, {-3.000000, -3.100000, 0.000000});
        geometric_.addAtom(Elements::F, {-4.280000, -4.300000, 0.000000});
        geometric_.addAtom(Elements::F, {-1.720000, -4.300000, 0.000000});
        geometric_.addAtom(Elements::P, {3.000000, -4.000000, 0.000000});
        geometric_.addAtom(Elements::F, {3.000000, -2.800000, 0.000000});
        geometric_.addAtom(Elements::F, {3.000000, -5.200000, 0.000000});
        geometric_.addAtom(Elements::F, {1.720000, -4.000000, 0.000000});
        geometric_.addAtom(Elements::F, {4.280000, -4.000000, 0.000000});
        geometric_.addMissingBonds();
    }
    return geometric_;
}

// Return benzene test species
const Species &benzeneSpecies()
{
    static Species benzene_;
    if (benzene_.nAtoms() == 0)
    {
        benzene_.setName("Benzene");
        benzene_.addAtom(Elements::C, {-1.390000, 0.000000, 0.000000});
        benzene_.addAtom(Elements::C, {-0.695000, 1.203775, 0.000000});
        benzene_.addAtom(Elements::C, {0.695000, 1.203775, 0.000000});
        benzene_.addAtom(Elements::C, {1.390000, 0.000000, 0.000000});
        benzene_.addAtom(Elements::C, {0.695000, -1.203775, 0.000000});
        benzene_.addAtom(Elements::C, {-0.695000, -1.203775, 0.000000});
        benzene_.addAtom(Elements::H, {1.195000, 2.069801, 0.000000});
        benzene_.addAtom(Elements::H, {2.390000, 0.000000, 0.000000});
        benzene_.addAtom(Elements::H, {1.195000, -2.069801, 0.000000});
        benzene_.addAtom(Elements::H, {-1.195000, -2.069801, 0.000000});
        benzene_.addAtom(Elements::H, {-2.390000, 0.000000, 0.000000});
        benzene_.addAtom(Elements::H, {-1.195000, 2.069801, 0.000000});
        benzene_.addMissingBonds();
    }
    return benzene_;
}

} // namespace UnitTest
