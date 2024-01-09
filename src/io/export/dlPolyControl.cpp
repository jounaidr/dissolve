// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2023 Team Dissolve and contributors

#include "io/export/dlPolyControl.h"
#include "base/lineParser.h"
#include "base/sysFunc.h"
#include "classes/configuration.h"

DlPolyControlExportFileFormat::DlPolyControlExportFileFormat(std::string_view filename, DlPolyControlExportFormat format)
    : FileAndFormat(formats_, filename, (int)format)
{
    formats_ = EnumOptions<DlPolyControlExportFileFormat::DlPolyControlExportFormat>(
        "ControlExportFileFormat", {{DlPolyControlExportFormat::DLPOLY, "dlpoly", "DL_POLY CONFIG File"}});
}


// Export DlPolyControl as CONTROL file
bool DlPolyControlExportFileFormat::exportDLPOLY(LineParser &parser, Configuration *cfg, bool capForces, double capForcesAt, std::optional<double> cutoffDistance, double padding, std::string ensemble, std::string ensembleMethod, double ensembleThermostatCoupling, bool timestepVariable, double fixedTimestep, std::optional<int> energyFrequency, int nSteps, std::optional<int> outputFrequency, bool randomVelocities, std::optional<int> trajectoryFrequency, std::string trajectoryKey, std::string coulMethod, double coulPrecision, std::string minimisationCriterion, double minimisationTolerance, double minimisationFrequency)
{
    if (!parser.writeLineF("title {} @ {}\n\n", cfg->name(), cfg->contentsVersion()))
        return false;
    if (!parser.writeLineF("io_file_config CONFIG\n"))
        return false;
    if (!parser.writeLineF("io_file_field FIELD\n"))
        return false;
    if (!parser.writeLineF("io_file_statis STATIS\n"))
        return false;
    if (!parser.writeLineF("io_file_revive REVIVE\n"))
        return false;
    if (!parser.writeLineF("io_file_revcon REVCON\n"))
        return false;
    if (!parser.writeLineF("temperature {} K\n", cfg->temperature()))
        return false;
    if (!parser.writeLineF("print_frequency {} steps\n", energyFrequency.value()))
        return false;
    if (!parser.writeLineF("stats_frequency {} steps\n", outputFrequency.value()))
        return false;
    if (!parser.writeLineF("cutoff {} nm\n", cutoffDistance.value()))
        return false;
    if (!parser.writeLineF("padding {} ang\n", padding))
        return false;
    if (!parser.writeLineF("ensemble {}\n", ensemble))
        return false;
    // Export ensemble_method and ensemble_thermostat_coupling for ensemble types other than NVE     
    if (ensemble != "NVE"){
        if (!parser.writeLineF("ensemble_method {}\n", ensembleMethod))
            return false;
        if (!parser.writeLineF("ensemble_thermostat_coupling  {} ps\n", ensembleThermostatCoupling))
            return false;
    }
    // Export equilibration_force_cap if capForces is true
    if (capForces && !parser.writeLineF("equilibration_force_cap {} k_b.temp/ang\n", capForcesAt))
        return false;
    if (!parser.writeLineF("time_run {} steps\n", nSteps))
        return false;
    // Export timestep_variable as ON if timestepVariable is true
    if (timestepVariable && !parser.writeLineF("timestep_variable ON\n"))
        return false;
    if (!parser.writeLineF("timestep {} internal_t\n", fixedTimestep))
        return false;
    // Export traj_calculate, traj_interval, traj_key if trajectoryFrequency is greater than 0
    if (trajectoryFrequency.value_or(0) > 0)
    {
        if (!parser.writeLineF("traj_calculate ON\n"))
            return false;
        if (!parser.writeLineF("traj_interval {}\n", trajectoryFrequency.value()))
            return false;
        if (!parser.writeLineF("traj_key {}\n", trajectoryKey))
            return false;
    }
    if (!parser.writeLineF("coul_method {}\n", coulMethod))
        return false;
    if (!parser.writeLineF("coul_precision {}\n", coulPrecision))
        return false;
    
    // Note interaction is taken from the first atom only, not sure how to do this ...
    const auto sri = cfg->speciesPopulations()[0].first->atoms()[0].atomType()->interactionPotential().form();
    std::string vdw_mix_method;
    
    // Export vdw_mix_method based on short range function type
    switch (sri)
    {
        case (ShortRangeFunctions::Form::None):
            vdw_mix_method = "Off";
            break;
        case (ShortRangeFunctions::Form::LennardJones):
            vdw_mix_method = "Lorentz-Berthelot";
            break;
        case (ShortRangeFunctions::Form::LennardJonesGeometric):
            vdw_mix_method = "Hogervorst";
            break;
        default:
            throw(std::runtime_error(fmt::format("Short-range type {} is not accounted for in PairPotential::setUp().\n",
                                                 ShortRangeFunctions::forms().keyword(sri))));
    }
    if (!parser.writeLineF("vdw_mix_method {}\n", vdw_mix_method))
        return false;
    if (!parser.writeLineF("minimisation_criterion {}\n", minimisationCriterion))
        return false;
    if (!parser.writeLineF("minimisation_tolerance {}\n", minimisationTolerance))
        return false;
    if (!parser.writeLineF("minimisation_frequency {} steps\n", minimisationFrequency))
        return false;

    return true;
}

// Export DlPolyControl using current filename and format
bool DlPolyControlExportFileFormat::exportData(Configuration *cfg, bool capForces, double capForcesAt, std::optional<double> cutoffDistance, double padding, std::string ensemble, std::string ensembleMethod, double ensembleThermostatCoupling, bool timestepVariable, double fixedTimestep, std::optional<int> energyFrequency, int nSteps, std::optional<int> outputFrequency, bool randomVelocities, std::optional<int> trajectoryFrequency, std::string trajectoryKey, std::string coulMethod, double coulPrecision, std::string minimisationCriterion, double minimisationTolerance, double minimisationFrequency)
{
    // Open the file
    LineParser parser;
    if (!parser.openOutput(filename_))
    {
        parser.closeFiles();
        return false;
    }

    // Write data
    auto result = false;
    switch (formats_.enumerationByIndex(*formatIndex_))
    {
        case (DlPolyControlExportFormat::DLPOLY):
            result = exportDLPOLY(parser, 
                                  cfg,                                              
                                  capForces,
                                  capForcesAt,
                                  cutoffDistance,
                                  padding,
                                  ensemble,
                                  ensembleMethod,
                                  ensembleThermostatCoupling,
                                  timestepVariable,
                                  fixedTimestep,
                                  energyFrequency,
                                  nSteps,
                                  outputFrequency,
                                  randomVelocities,
                                  trajectoryFrequency,
                                  trajectoryKey,
                                  coulMethod,
                                  coulPrecision,
                                  minimisationCriterion,
                                  minimisationTolerance,
                                  minimisationFrequency);
            break;
        default:
            throw(std::runtime_error(fmt::format("DlPolyControl format '{}' export has not been implemented.\n",
                                                 formats_.keywordByIndex(*formatIndex_))));
    }

    return true;
}