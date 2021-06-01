// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2021 Team Dissolve and contributors

#include "io/import/data1d.h"
#include "base/lineparser.h"
#include "base/sysfunc.h"
#include "math/filters.h"

Data1DImportFileFormat::Data1DImportFileFormat(std::string_view filename, Data1DImportFileFormat::Data1DImportFormat format)
    : FileAndFormat(formats_, filename)
{
    formats_ = EnumOptions<Data1DImportFileFormat::Data1DImportFormat>(
        "Data1DImportFileFormat",
        {{Data1DImportFileFormat::XYData1D, "xy", "Simple XY data (x = bin centres)"},
         {Data1DImportFileFormat::HistogramData1D, "histogram", "Histogrammed Data (x = bin left-boundaries)"},
         {Data1DImportFileFormat::GudrunMintData1D, "mint", "Gudrun output (mint01)"}},
        format);
    setUpKeywords();
}

/*
 * Keyword Options
 */

// Set up keywords for the format
void Data1DImportFileFormat::setUpKeywords()
{
    keywords_.add("Columns", new IntegerKeyword(1, 1), "X", "Column index to use for x values");
    keywords_.add("Columns", new IntegerKeyword(2, 1), "Y", "Column index to use for y values");
    keywords_.add("Columns", new IntegerKeyword(0, 0), "Error", "Column index to use for error values");
    keywords_.add("Manipulations", new DoubleKeyword(-1.0, -1.0), "RemoveAverage",
                  "X axis value from which to form average value to subtract from data (-1 for no subtraction)");
    keywords_.add("Manipulations", new RangeKeyword(Range(0.0, 0.0), Vec3Labels::MinMaxDeltaLabels), "Trim",
                  "Trim the range of the loaded data to be within the specified boundaries");
    keywords_.add("Manipulations", new IntegerKeyword(0, 0), "RemovePoints",
                  "Remove a number of points from the start of the data");
}

/*
 * Import Functions
 */

// Import Data1D using current filename and format
bool Data1DImportFileFormat::importData(Data1D &data, ProcessPool *procPool)
{
    // Open file and check that we're OK to proceed importing from it
    LineParser parser(procPool);
    if ((!parser.openInput(filename_)) || (!parser.isFileGoodForReading()))
        return Messenger::error("Couldn't open file '{}' for loading Data1D data.\n", filename_);

    // Import the data
    auto result = importData(parser, data);

    parser.closeFiles();

    return result;
}

// Import Data1D using supplied parser and current format
bool Data1DImportFileFormat::importData(LineParser &parser, Data1D &data)
{
    // Import the data
    auto result = false;
    switch (formats_.enumeration())
    {
        case (Data1DImportFileFormat::XYData1D):
            result = importXY(parser, data);
            break;
        case (Data1DImportFileFormat::HistogramData1D):
            result = importHistogram(parser, data);
            break;
        case (Data1DImportFileFormat::GudrunMintData1D):
            result = importGudrunMint(parser, data);
            break;
        default:
            throw(std::runtime_error(fmt::format("Data1D format '{}' import has not been implemented.\n", formats_.keyword())));
    }

    // If we failed, may as well return now
    if (!result)
        return false;

    // Handle any additional options
    // -- Remove points from the start of the data?
    for (auto n = 0; n < keywords_.asInt("RemovePoints"); ++n)
        data.removeFirstPoint();
    // -- Trim range?
    if (keywords_.hasBeenSet("Trim"))
    {
        const auto range = keywords_.retrieve<Range>("Trim");
        Filters::trim(data, range.minimum(), range.maximum());
    }
    // -- Subtract average level from data?
    const auto removeAverage = keywords_.asDouble("RemoveAverage");
    if (removeAverage > 0.0)
    {
        double level = Filters::subtractAverage(data, removeAverage);
        Messenger::print("Removed average level of {} from data, forming average over x >= {}.\n", level, removeAverage);
    }

    return result;
}
