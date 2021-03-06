// The MIT License (MIT)

// Copyright (c) 2016, Microsoft

// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:

// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.

// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.

#include <limits>
#include <sstream>
#include <vector>

#include "gtest/gtest.h"

#include "BitFunnel/Configuration/Factories.h"
#include "BitFunnel/Configuration/IFileSystem.h"
#include "BitFunnel/Data/SimpleData.h"
#include "BitFunnel/Data/Sequential.h"
#include "BitFunnel/Data/Sonnets.h"
#include "BitFunnelTool.h"


namespace BitFunnel
{
    // TODO: refactor this copied code into common code.
    TEST(BitFunnelToolOld, ThreeToolsEndToEndSimpleInterpreter)
    {
        //
        // This test is going to run out of a RAM filesystem.
        //
        auto fileSystem = BitFunnel::Factories::CreateRAMFileSystem();
        auto fileManager =
            BitFunnel::Factories::CreateFileManager(
                "config",
                "statistics",
                "index",
                *fileSystem);

        //
        // Initialize RAM filesystem with input files.
        //
        {
            // Open the manifest file.
            auto manifest = fileSystem->OpenForWrite("manifest.txt");

            for (size_t i = 0; i < SimpleData::chunks.size(); ++i)
            {
                // Create chunk file name, and write chunk data.
                std::stringstream name;
                name << "simpledata" << i;
                auto out = fileSystem->OpenForWrite(name.str().c_str());
                out->write(SimpleData::chunks[i].second,
                           static_cast<std::streamsize>(Sonnets::chunks[i].first));

                // Add chunk file to manifest.
                *manifest << name.str() << std::endl;
            }

            auto script = fileSystem->OpenForWrite("testScript");
            *script << "failOnException" << std::endl
                    << "cache chunk simpledata0" << std::endl
                    << "cache chunk simpledata1" << std::endl;
        }

        //
        // Create the BitFunnelTool based on the RAM filesystem.
        //
        BitFunnel::BitFunnelTool tool(*fileSystem);

        //
        // Use the tool to run the statistics builder.
        //
        {
            std::vector<char const *> argv = {
                "BitFunnel",
                "statistics",
                "manifest.txt",
                "config"
            };

            tool.Main(std::cin,
                      std::cout,
                      static_cast<int>(argv.size()),
                      argv.data());
        }


        //
        // Use the tool to run the TermTable builder.
        //
        {
            std::vector<char const *> argv = {
                "BitFunnel",
                "termtable",
                "config",
                "0.1",
                "PrivateSharedRank0And3"
            };

            tool.Main(std::cin,
                      std::cout,
                      static_cast<int>(argv.size()),
                      argv.data());
        }


        //
        // Use the tool to run the REPL.
        //
        {
            std::vector<char const *> argv = {
                "BitFunnel",
                "repl",
                "config",
                // -script and testScript must be on seperate lines because
                // tokens are delimited by whitespace.
                "-script",
                "testScript"
            };

            // Create an input stream with commands to
            // load a chunk, verify a query, and inspect
            // some rows.
            std::stringstream input;
            input
                // This first line is run via -script.
                // << "cache chunk sonnet0" << std::endl
                << "interpreter" << std::endl
                << "verify one five" << std::endl
                << "show rows five" << std::endl;

            tool.Main(input,
                      std::cout,
                      static_cast<int>(argv.size()),
                      argv.data());
        }
    }


    // TODO: refactor this copied code into common code.
    // TEST(BitFunnelTool, ThreeToolsEndToEndSequentialInterpreter)
    // This test is currently disabled. See issues #382, #395.
    void ThisTestIsCurrentlyDisabled()
    {
        std::stringstream output;
        //
        // This test is going to run out of a RAM filesystem.
        //
        auto fileSystem = BitFunnel::Factories::CreateRAMFileSystem();
        auto fileManager =
            BitFunnel::Factories::CreateFileManager(
                "config",
                "statistics",
                "index",
                *fileSystem);

        //
        // Initialize RAM filesystem with input files.
        //
        {
            // Open the manifest file.
            auto manifest = fileSystem->OpenForWrite("manifest.txt");

            for (size_t i = 0; i < Sequential::chunks.size(); ++i)
            {
                // Create chunk file name, and write chunk data.
                std::stringstream name;
                name << "sequential" << i;
                auto out = fileSystem->OpenForWrite(name.str().c_str());
                out->write(Sequential::chunks[i].second,
                           static_cast<std::streamsize>(Sequential::chunks[i].first));

                // Add chunk file to manifest.
                *manifest << name.str() << std::endl;
            }

            auto script = fileSystem->OpenForWrite("testScript");
            *script << "failOnException" << std::endl
                    << "cache chunk sequential0" << std::endl;
        }

        //
        // Create the BitFunnelTool based on the RAM filesystem.
        //
        BitFunnel::BitFunnelTool tool(*fileSystem);

        //
        // Use the tool to run the statistics builder.
        //
        {
            std::vector<char const *> argv = {
                "BitFunnel",
                "statistics",
                "manifest.txt",
                "config"
            };

            tool.Main(std::cin,
                      output,
                      static_cast<int>(argv.size()),
                      argv.data());
        }


        //
        // Use the tool to run the TermTable builder.
        //
        {
            std::vector<char const *> argv = {
                "BitFunnel",
                "termtable",
                "config",
                "0.1",
                "PrivateSharedRank0And3"
            };

            tool.Main(std::cin,
                      output,
                      static_cast<int>(argv.size()),
                      argv.data());
        }


        //
        // Use the tool to run the REPL.
        //
        {
            std::vector<char const *> argv = {
                "BitFunnel",
                "repl",
                "config",
                // -script and testScript must be on seperate lines because
                // tokens are delimited by whitespace.
                "-script",
                "testScript"
            };

            // Create an input stream with commands to
            // load a chunk, verify a query, and inspect
            // some rows.
            std::stringstream input;
            input
                // This first line is run via -script.
                // << "cache chunk sonnet0" << std::endl
                    << "interpreter" << std::endl
                    << "verify one one" << std::endl
                    << "verify one two" << std::endl
                    << "verify one three" << std::endl
                    << "verify one four" << std::endl
                    << "verify one five" << std::endl
                    << "verify one six" << std::endl
                    << "verify one seven" << std::endl
                    << "verify one eight" << std::endl
                    << "verify one nine" << std::endl
                    << "verify one ten" << std::endl
                    << "verify one eleven" << std::endl
                    << "verify one twelve" << std::endl
                    << "verify one thirteen" << std::endl
                    << "verify one fourteen" << std::endl
                    << "verify one fifteen" << std::endl
                    << "verify one sixteen" << std::endl
                    << "verify one seventeen" << std::endl
                    << "verify one eighteen" << std::endl
                    << "verify one nineteen" << std::endl
                    << "verify one twenty" << std::endl
                    << "verify one twenty\\-one" << std::endl
                    << "verify one twenty\\-two" << std::endl
                    << "verify one twenty\\-three" << std::endl
                    << "verify one twenty\\-four" << std::endl
                    << "verify one twenty\\-five" << std::endl
                    << "verify one twenty\\-six" << std::endl
                    << "verify one twenty\\-seven" << std::endl
                    << "verify one twenty\\-eight" << std::endl
                    << "verify one twenty\\-nine" << std::endl
                    << "verify one thirty" << std::endl
                    << "verify one thirty\\-one" << std::endl
                    << "verify one thirty\\-two" << std::endl
                    << "verify one thirty\\-three" << std::endl
                    << "verify one thirty\\-four" << std::endl
                    << "verify one thirty\\-five" << std::endl
                    << "verify one thirty\\-six" << std::endl
                    << "verify one thirty\\-seven" << std::endl
                    << "verify one thirty\\-eight" << std::endl
                    << "verify one thirty\\-nine" << std::endl
                    << "verify one forty" << std::endl
                    << "verify one forty\\-one" << std::endl
                    << "verify one forty\\-two" << std::endl
                    << "verify one forty\\-three" << std::endl
                    << "verify one forty\\-four" << std::endl
                    << "verify one forty\\-five" << std::endl
                    << "verify one forty\\-six" << std::endl
                    << "verify one forty\\-seven" << std::endl
                    << "verify one forty\\-eight" << std::endl
                    << "verify one forty\\-nine" << std::endl
                    << "verify one fifty" << std::endl
                    << "verify one fifty\\-one" << std::endl
                    << "verify one fifty\\-two" << std::endl
                    << "verify one fifty\\-three" << std::endl
                    << "verify one fifty\\-four" << std::endl
                    << "verify one fifty\\-five" << std::endl
                    << "verify one fifty\\-six" << std::endl
                    << "verify one fifty\\-seven" << std::endl
                    << "verify one fifty\\-eight" << std::endl
                    << "verify one fifty\\-nine" << std::endl
                    << "verify one sixty" << std::endl
                    << "verify one sixty\\-one" << std::endl
                    << "verify one sixty\\-two" << std::endl
                    << "verify one sixty\\-three" << std::endl
                    << "verify one sixty\\-four" << std::endl
                    << "verify one sixty\\-five" << std::endl
                    << "verify one sixty\\-six" << std::endl
                    << "verify one sixty\\-seven" << std::endl
                    << "verify one sixty\\-eight" << std::endl
                    << "verify one sixty\\-nine" << std::endl
                    << "verify one seventy" << std::endl
                    << "verify one seventy\\-one" << std::endl
                    << "verify one seventy\\-two" << std::endl
                    << "verify one seventy\\-three" << std::endl
                    << "verify one seventy\\-four" << std::endl
                    << "verify one seventy\\-five" << std::endl
                    << "verify one seventy\\-six" << std::endl
                    << "verify one seventy\\-seven" << std::endl
                    << "verify one seventy\\-eight" << std::endl
                    << "verify one seventy\\-nine" << std::endl
                    << "verify one eighty" << std::endl
                    << "verify one eighty\\-one" << std::endl
                    << "verify one eighty\\-two" << std::endl
                    << "verify one eighty\\-three" << std::endl
                    << "verify one eighty\\-four" << std::endl
                    << "verify one eighty\\-five" << std::endl
                    << "verify one eighty\\-six" << std::endl
                    << "verify one eighty\\-seven" << std::endl
                    << "verify one eighty\\-eight" << std::endl
                    << "verify one eighty\\-nine" << std::endl
                    << "verify one ninety" << std::endl
                    << "verify one ninety\\-one" << std::endl
                    << "verify one ninety\\-two" << std::endl
                    << "verify one ninety\\-three" << std::endl
                    << "verify one ninety\\-four" << std::endl
                    << "verify one ninety\\-five" << std::endl
                    << "verify one ninety\\-six" << std::endl
                    << "verify one ninety\\-seven" << std::endl
                    << "verify one ninety\\-eight" << std::endl
                    << "verify one ninety\\-nine" << std::endl
                    << "verify one one\\-hundred" << std::endl
                    << "verify one one\\-hundred\\-and\\-one" << std::endl
                    << "verify one one\\-hundred\\-and\\-two" << std::endl
                    << "verify one one\\-hundred\\-and\\-three" << std::endl
                    << "verify one one\\-hundred\\-and\\-four" << std::endl
                    << "verify one one\\-hundred\\-and\\-five" << std::endl
                    << "verify one one\\-hundred\\-and\\-six" << std::endl
                    << "verify one one\\-hundred\\-and\\-seven" << std::endl
                    << "verify one one\\-hundred\\-and\\-eight" << std::endl
                    << "verify one one\\-hundred\\-and\\-nine" << std::endl
                    << "verify one one\\-hundred\\-and\\-ten" << std::endl
                    << "verify one one\\-hundred\\-and\\-eleven" << std::endl
                    << "verify one one\\-hundred\\-and\\-twelve" << std::endl
                    << "verify one one\\-hundred\\-and\\-thirteen" << std::endl
                    << "verify one one\\-hundred\\-and\\-fourteen" << std::endl
                    << "verify one one\\-hundred\\-and\\-fifteen" << std::endl
                    << "verify one one\\-hundred\\-and\\-sixteen" << std::endl
                    << "verify one one\\-hundred\\-and\\-seventeen" << std::endl
                    << "verify one one\\-hundred\\-and\\-eighteen" << std::endl
                    << "verify one one\\-hundred\\-and\\-nineteen" << std::endl
                    << "verify one one\\-hundred\\-and\\-twenty" << std::endl
                    << "verify one one\\-hundred\\-and\\-twenty\\-one" << std::endl
                    << "verify one one\\-hundred\\-and\\-twenty\\-two" << std::endl
                    << "verify one one\\-hundred\\-and\\-twenty\\-three" << std::endl
                    << "verify one one\\-hundred\\-and\\-twenty\\-four" << std::endl
                    << "verify one one\\-hundred\\-and\\-twenty\\-five" << std::endl
                    << "verify one one\\-hundred\\-and\\-twenty\\-six" << std::endl
                    << "verify one one\\-hundred\\-and\\-twenty\\-seven" << std::endl
                    << "verify one one\\-hundred\\-and\\-twenty\\-eight" << std::endl
                    << "verify one one\\-hundred\\-and\\-twenty\\-nine" << std::endl
                    << "verify one one\\-hundred\\-and\\-thirty" << std::endl
                    << "verify one one\\-hundred\\-and\\-thirty\\-one" << std::endl
                    << "verify one one\\-hundred\\-and\\-thirty\\-two" << std::endl
                    << "verify one one\\-hundred\\-and\\-thirty\\-three" << std::endl
                    << "verify one one\\-hundred\\-and\\-thirty\\-four" << std::endl
                    << "verify one one\\-hundred\\-and\\-thirty\\-five" << std::endl
                    << "verify one one\\-hundred\\-and\\-thirty\\-six" << std::endl
                    << "verify one one\\-hundred\\-and\\-thirty\\-seven" << std::endl
                    << "verify one one\\-hundred\\-and\\-thirty\\-eight" << std::endl
                    << "verify one one\\-hundred\\-and\\-thirty\\-nine" << std::endl
                    << "verify one one\\-hundred\\-and\\-forty" << std::endl
                    << "verify one one\\-hundred\\-and\\-forty\\-one" << std::endl
                    << "verify one one\\-hundred\\-and\\-forty\\-two" << std::endl
                    << "verify one one\\-hundred\\-and\\-forty\\-three" << std::endl
                    << "verify one one\\-hundred\\-and\\-forty\\-four" << std::endl
                    << "verify one one\\-hundred\\-and\\-forty\\-five" << std::endl
                    << "verify one one\\-hundred\\-and\\-forty\\-six" << std::endl
                    << "verify one one\\-hundred\\-and\\-forty\\-seven" << std::endl
                    << "verify one one\\-hundred\\-and\\-forty\\-eight" << std::endl
                    << "verify one one\\-hundred\\-and\\-forty\\-nine" << std::endl
                    << "verify one one\\-hundred\\-and\\-fifty" << std::endl
                    << "verify one one\\-hundred\\-and\\-fifty\\-one" << std::endl
                    << "verify one one\\-hundred\\-and\\-fifty\\-two" << std::endl
                    << "verify one one\\-hundred\\-and\\-fifty\\-three" << std::endl
                    << "verify one one\\-hundred\\-and\\-fifty\\-four" << std::endl
                    << "verify one one\\-hundred\\-and\\-fifty\\-five" << std::endl
                    << "verify one one\\-hundred\\-and\\-fifty\\-six" << std::endl
                    << "verify one one\\-hundred\\-and\\-fifty\\-seven" << std::endl
                    << "verify one one\\-hundred\\-and\\-fifty\\-eight" << std::endl
                    << "verify one one\\-hundred\\-and\\-fifty\\-nine" << std::endl
                    << "verify one one\\-hundred\\-and\\-sixty" << std::endl
                    << "verify one one\\-hundred\\-and\\-sixty\\-one" << std::endl
                    << "verify one one\\-hundred\\-and\\-sixty\\-two" << std::endl
                    << "verify one one\\-hundred\\-and\\-sixty\\-three" << std::endl
                    << "verify one one\\-hundred\\-and\\-sixty\\-four" << std::endl
                    << "verify one one\\-hundred\\-and\\-sixty\\-five" << std::endl
                    << "verify one one\\-hundred\\-and\\-sixty\\-six" << std::endl
                    << "verify one one\\-hundred\\-and\\-sixty\\-seven" << std::endl
                    << "verify one one\\-hundred\\-and\\-sixty\\-eight" << std::endl
                    << "verify one one\\-hundred\\-and\\-sixty\\-nine" << std::endl
                    << "verify one one\\-hundred\\-and\\-seventy" << std::endl
                    << "verify one one\\-hundred\\-and\\-seventy\\-one" << std::endl
                    << "verify one one\\-hundred\\-and\\-seventy\\-two" << std::endl
                    << "verify one one\\-hundred\\-and\\-seventy\\-three" << std::endl
                    << "verify one one\\-hundred\\-and\\-seventy\\-four" << std::endl
                    << "verify one one\\-hundred\\-and\\-seventy\\-five" << std::endl
                    << "verify one one\\-hundred\\-and\\-seventy\\-six" << std::endl
                    << "verify one one\\-hundred\\-and\\-seventy\\-seven" << std::endl
                    << "verify one one\\-hundred\\-and\\-seventy\\-eight" << std::endl
                    << "verify one one\\-hundred\\-and\\-seventy\\-nine" << std::endl
                    << "verify one one\\-hundred\\-and\\-eighty" << std::endl
                    << "verify one one\\-hundred\\-and\\-eighty\\-one" << std::endl
                    << "verify one one\\-hundred\\-and\\-eighty\\-two" << std::endl
                    << "verify one one\\-hundred\\-and\\-eighty\\-three" << std::endl
                    << "verify one one\\-hundred\\-and\\-eighty\\-four" << std::endl
                    << "verify one one\\-hundred\\-and\\-eighty\\-five" << std::endl
                    << "verify one one\\-hundred\\-and\\-eighty\\-six" << std::endl
                    << "verify one one\\-hundred\\-and\\-eighty\\-seven" << std::endl
                    << "verify one one\\-hundred\\-and\\-eighty\\-eight" << std::endl
                    << "verify one one\\-hundred\\-and\\-eighty\\-nine" << std::endl
                    << "verify one one\\-hundred\\-and\\-ninety" << std::endl
                    << "verify one one\\-hundred\\-and\\-ninety\\-one" << std::endl
                    << "verify one one\\-hundred\\-and\\-ninety\\-two" << std::endl;


            tool.Main(input,
                      output,
                      static_cast<int>(argv.size()),
                      argv.data());

             // TODO: this is an extremely brittle way to check if we have false
             // positives. This is being done this way because it appears that
             // it's a standard pattern for our commands to entire write to a
             // file or write to a stream. In order to make commands more easily
             // tstable, we should probably return a data structure and have the
             // REPL print out a readable form of the data structure.
             std::string text = output.str();


             size_t false_positive_found = text.find("False positives:");
             if (false_positive_found != std::string::npos)
             {
                 // TODO: only print out text on failure.
                 std::cout << text;
                 FAIL() << "Found false positives.";
             }
        }
    }


    // TODO: refactor this copied code into common code.
    TEST(BitFunnelToolOld, ThreeToolsEndToEndInterpreter)
    {
        //
        // This test is going to run out of a RAM filesystem.
        //
        auto fileSystem = BitFunnel::Factories::CreateRAMFileSystem();
        auto fileManager =
            BitFunnel::Factories::CreateFileManager(
                "config",
                "statistics",
                "index",
                *fileSystem);

        //
        // Initialize RAM filesystem with input files.
        //
        {
            // Open the manifest file.
            auto manifest = fileSystem->OpenForWrite("manifest.txt");

            // Iterate over sequence of Shakespeare sonnet chunk data.
            for (size_t i = 0; i < Sonnets::chunks.size(); ++i)
            {
                // Create chunk file name, and write chunk data.
                std::stringstream name;
                name << "sonnet" << i;
                auto out = fileSystem->OpenForWrite(name.str().c_str());
                out->write(Sonnets::chunks[i].second,
                           static_cast<std::streamsize>(Sonnets::chunks[i].first));

                // Add chunk file to manifest.
                *manifest << name.str() << std::endl;
            }

            auto script = fileSystem->OpenForWrite("testScript");
            *script << "failOnException" << std::endl
                    << "cache chunk sonnet0" << std::endl;
        }

        //
        // Create the BitFunnelTool based on the RAM filesystem.
        //
        BitFunnel::BitFunnelTool tool(*fileSystem);

        //
        // Use the tool to run the statistics builder.
        //
        {
            std::vector<char const *> argv = {
                "BitFunnel",
                "statistics",
                "manifest.txt",
                "config"
            };

            tool.Main(std::cin,
                      std::cout,
                      static_cast<int>(argv.size()),
                      argv.data());
        }


        //
        // Use the tool to run the TermTable builder.
        //
        {
            std::vector<char const *> argv = {
                "BitFunnel",
                "termtable",
                "config",
                "0.1",
                "PrivateSharedRank0And3"
            };

            tool.Main(std::cin,
                      std::cout,
                      static_cast<int>(argv.size()),
                      argv.data());
        }


        //
        // Use the tool to run the REPL.
        //
        {
            std::vector<char const *> argv = {
                "BitFunnel",
                "repl",
                "config",
                // -script and testScript must be on seperate lines because
                // tokens are delimited by whitespace.
                "-script",
                "testScript"
            };

            // Create an input stream with commands to
            // load a chunk, verify a query, and inspect
            // some rows.
            std::stringstream input;
            input
                // This first line is run via -script.
                // << "cache chunk sonnet0" << std::endl
                << "interpreter" << std::endl
                << "verify one blood" << std::endl
                << "show rows blood" << std::endl;

            tool.Main(input,
                      std::cout,
                      static_cast<int>(argv.size()),
                      argv.data());
        }

    }


    TEST(BitFunnelToolOld, ThreeToolsEndToEnd)
    {
        //
        // This test is going to run out of a RAM filesystem.
        //
        auto fileSystem = BitFunnel::Factories::CreateRAMFileSystem();
        auto fileManager =
            BitFunnel::Factories::CreateFileManager(
                "config",
                "statistics",
                "index",
                *fileSystem);

        //
        // Initialize RAM filesystem with input files.
        //
        {
            // Open the manifest file.
            auto manifest = fileSystem->OpenForWrite("manifest.txt");

            // Iterate over sequence of Shakespeare sonnet chunk data.
            for (size_t i = 0; i < Sonnets::chunks.size(); ++i)
            {
                // Create chunk file name, and write chunk data.
                std::stringstream name;
                name << "sonnet" << i;
                auto out = fileSystem->OpenForWrite(name.str().c_str());
                out->write(Sonnets::chunks[i].second,
                           static_cast<std::streamsize>(Sonnets::chunks[i].first));

                // Add chunk file to manifest.
                *manifest << name.str() << std::endl;
            }

            auto script = fileSystem->OpenForWrite("testScript");
            *script << "failOnException" << std::endl
                    << "cache chunk sonnet0" << std::endl;
        }

        //
        // Create the BitFunnelTool based on the RAM filesystem.
        //
        BitFunnel::BitFunnelTool tool(*fileSystem);

        //
        // Use the tool to run the statistics builder.
        //
        {
            std::vector<char const *> argv = {
                "BitFunnel",
                "statistics",
                "manifest.txt",
                "config"
            };

            tool.Main(std::cin,
                      std::cout,
                      static_cast<int>(argv.size()),
                      argv.data());
        }


        //
        // Use the tool to run the TermTable builder.
        //
        {
            std::vector<char const *> argv = {
                "BitFunnel",
                "termtable",
                "config",
                "0.1",
                "PrivateSharedRank0And3"
            };

            tool.Main(std::cin,
                      std::cout,
                      static_cast<int>(argv.size()),
                      argv.data());
        }


        //
        // Use the tool to run the REPL.
        //
        {
            std::vector<char const *> argv = {
                "BitFunnel",
                "repl",
                "config",
                // -script and testScript must be on seperate lines because
                // tokens are delimited by whitespace.
                "-script",
                "testScript"
            };

            // Create an input stream with commands to
            // load a chunk, verify a query, and inspect
            // some rows.
            std::stringstream input;
            input
                // This first line is run via -script.
                // << "cache chunk sonnet0" << std::endl
                << "verify one blood" << std::endl
                << "show rows blood" << std::endl;

            tool.Main(input,
                      std::cout,
                      static_cast<int>(argv.size()),
                      argv.data());
        }
    }


    // TODO: refactor this copied code into common code.
    TEST(BitFunnelToolOld, ThreeToolsEndToEndSequentialInterpreterMultiShard)
    {
        for (int shardSplit = 1; shardSplit < 64; shardSplit += 10) {
            //
            // This test is going to run out of a RAM filesystem.
            //
            auto fileSystem = BitFunnel::Factories::CreateRAMFileSystem();
            auto fileManager =
                    BitFunnel::Factories::CreateFileManager(
                            "config",
                            "statistics",
                            "index",
                            *fileSystem);


            //
            // Create ShardDefinition
            //
            {
                // According to FileManager, ShardDefinition is in statisticsDirectory.
                auto shardDefinition = fileSystem->OpenForWrite("config/ShardDefinition.csv");
                // NOTE: we'll get false negatives and false positives if we change this to 3.
                *shardDefinition << shardSplit;
            }


            //
            // Initialize RAM filesystem with input files.
            //
            {
                // Open the manifest file.
                auto manifest = fileSystem->OpenForWrite("manifest.txt");

                for (size_t i = 0; i < Sequential::chunks.size(); ++i) {
                    // Create chunk file name, and write chunk data.
                    std::stringstream name;
                    name << "sequential" << i;
                    auto out = fileSystem->OpenForWrite(name.str().c_str());
                    out->write(Sequential::chunks[i].second,
                               static_cast<std::streamsize>(Sequential::chunks[i].first));

                    // Add chunk file to manifest.
                    *manifest << name.str() << std::endl;
                }

                auto script = fileSystem->OpenForWrite("testScript");
                *script << "failOnException" << std::endl
                        << "cache chunk sequential0" << std::endl;
            }

            //
            // Create the BitFunnelTool based on the RAM filesystem.
            //
            BitFunnel::BitFunnelTool tool(*fileSystem);

            //
            // Use the tool to run the statistics builder.
            //
            {
                std::vector<char const *> argv = {
                        "BitFunnel",
                        "statistics",
                        "manifest.txt",
                        "config"
                };

                tool.Main(std::cin,
                          std::cout,
                          static_cast<int>(argv.size()),
                          argv.data());
            }


            //
            // Use the tool to run the TermTable builder.
            //
            {
                std::vector<char const *> argv = {
                        "BitFunnel",
                        "termtable",
                        "config",
                        "0.1",
                        "PrivateSharedRank0And3"
                };

                tool.Main(std::cin,
                          std::cout,
                          static_cast<int>(argv.size()),
                          argv.data());
            }


            //
            // Use the tool to run the REPL.
            //
            {
                std::vector<char const *> argv = {
                        "BitFunnel",
                        "repl",
                        "config",
                        // -script and testScript must be on seperate lines because
                        // tokens are delimited by whitespace.
                        "-script",
                        "testScript"
                };

                // Create an input stream with commands to
                // load a chunk, verify a query, and inspect
                // some rows.
                std::stringstream input;
                input
                        // This first line is run via -script.
                        // << "cache chunk sonnet0" << std::endl
                        << "interpreter" << std::endl
                        << "verify one one" << std::endl
                        << "verify one two" << std::endl
                        << "verify one three" << std::endl
                        << "verify one four" << std::endl
                        << "verify one five" << std::endl
                        << "verify one six" << std::endl
                        << "verify one seven" << std::endl
                        << "verify one eight" << std::endl
                        << "verify one nine" << std::endl
                        << "verify one ten" << std::endl
                        << "verify one eleven" << std::endl
                        << "verify one twelve" << std::endl
                        << "verify one thirteen" << std::endl
                        << "verify one fourteen" << std::endl
                        << "verify one fifteen" << std::endl
                        << "verify one sixteen" << std::endl
                        << "verify one seventeen" << std::endl
                        << "verify one eighteen" << std::endl
                        << "verify one nineteen" << std::endl
                        << "verify one twenty" << std::endl
                        << "verify one twenty\\-one" << std::endl
                        << "verify one twenty\\-two" << std::endl
                        << "verify one twenty\\-three" << std::endl
                        << "verify one twenty\\-four" << std::endl
                        << "verify one twenty\\-five" << std::endl
                        << "verify one twenty\\-six" << std::endl
                        << "verify one twenty\\-seven" << std::endl
                        << "verify one twenty\\-eight" << std::endl
                        << "verify one twenty\\-nine" << std::endl
                        << "verify one thirty" << std::endl
                        << "verify one thirty\\-one" << std::endl
                        << "verify one thirty\\-two" << std::endl
                        << "verify one thirty\\-three" << std::endl
                        << "verify one thirty\\-four" << std::endl
                        << "verify one thirty\\-five" << std::endl
                        << "verify one thirty\\-six" << std::endl
                        << "verify one thirty\\-seven" << std::endl
                        << "verify one thirty\\-eight" << std::endl
                        << "verify one thirty\\-nine" << std::endl
                        << "verify one forty" << std::endl
                        << "verify one forty\\-one" << std::endl
                        << "verify one forty\\-two" << std::endl
                        << "verify one forty\\-three" << std::endl
                        << "verify one forty\\-four" << std::endl
                        << "verify one forty\\-five" << std::endl
                        << "verify one forty\\-six" << std::endl
                        << "verify one forty\\-seven" << std::endl
                        << "verify one forty\\-eight" << std::endl
                        << "verify one forty\\-nine" << std::endl
                        << "verify one fifty" << std::endl
                        << "verify one fifty\\-one" << std::endl
                        << "verify one fifty\\-two" << std::endl
                        << "verify one fifty\\-three" << std::endl
                        << "verify one fifty\\-four" << std::endl
                        << "verify one fifty\\-five" << std::endl
                        << "verify one fifty\\-six" << std::endl
                        << "verify one fifty\\-seven" << std::endl
                        << "verify one fifty\\-eight" << std::endl
                        << "verify one fifty\\-nine" << std::endl
                        << "verify one sixty" << std::endl
                        << "verify one sixty\\-one" << std::endl
                        << "verify one sixty\\-two" << std::endl
                        << "verify one sixty\\-three" << std::endl
                        << "verify one sixty\\-four" << std::endl
                        << "verify one sixty\\-five" << std::endl
                        << "verify one sixty\\-six" << std::endl
                        << "verify one sixty\\-seven" << std::endl
                        << "verify one sixty\\-eight" << std::endl
                        << "verify one sixty\\-nine" << std::endl
                        << "verify one seventy" << std::endl
                        << "verify one seventy\\-one" << std::endl
                        << "verify one seventy\\-two" << std::endl
                        << "verify one seventy\\-three" << std::endl
                        << "verify one seventy\\-four" << std::endl
                        << "verify one seventy\\-five" << std::endl
                        << "verify one seventy\\-six" << std::endl
                        << "verify one seventy\\-seven" << std::endl
                        << "verify one seventy\\-eight" << std::endl
                        << "verify one seventy\\-nine" << std::endl
                        << "verify one eighty" << std::endl
                        << "verify one eighty\\-one" << std::endl
                        << "verify one eighty\\-two" << std::endl
                        << "verify one eighty\\-three" << std::endl
                        << "verify one eighty\\-four" << std::endl
                        << "verify one eighty\\-five" << std::endl
                        << "verify one eighty\\-six" << std::endl
                        << "verify one eighty\\-seven" << std::endl
                        << "verify one eighty\\-eight" << std::endl
                        << "verify one eighty\\-nine" << std::endl
                        << "verify one ninety" << std::endl
                        << "verify one ninety\\-one" << std::endl
                        << "verify one ninety\\-two" << std::endl
                        << "verify one ninety\\-three" << std::endl
                        << "verify one ninety\\-four" << std::endl
                        << "verify one ninety\\-five" << std::endl
                        << "verify one ninety\\-six" << std::endl
                        << "verify one ninety\\-seven" << std::endl
                        << "verify one ninety\\-eight" << std::endl
                        << "verify one ninety\\-nine" << std::endl
                        << "verify one one\\-hundred" << std::endl
                        << "verify one one\\-hundred\\-and\\-one" << std::endl
                        << "verify one one\\-hundred\\-and\\-two" << std::endl
                        << "verify one one\\-hundred\\-and\\-three" << std::endl
                        << "verify one one\\-hundred\\-and\\-four" << std::endl
                        << "verify one one\\-hundred\\-and\\-five" << std::endl
                        << "verify one one\\-hundred\\-and\\-six" << std::endl
                        << "verify one one\\-hundred\\-and\\-seven" << std::endl
                        << "verify one one\\-hundred\\-and\\-eight" << std::endl
                        << "verify one one\\-hundred\\-and\\-nine" << std::endl
                        << "verify one one\\-hundred\\-and\\-ten" << std::endl
                        << "verify one one\\-hundred\\-and\\-eleven" << std::endl
                        << "verify one one\\-hundred\\-and\\-twelve" << std::endl
                        << "verify one one\\-hundred\\-and\\-thirteen" << std::endl
                        << "verify one one\\-hundred\\-and\\-fourteen" << std::endl
                        << "verify one one\\-hundred\\-and\\-fifteen" << std::endl
                        << "verify one one\\-hundred\\-and\\-sixteen" << std::endl
                        << "verify one one\\-hundred\\-and\\-seventeen" << std::endl
                        << "verify one one\\-hundred\\-and\\-eighteen" << std::endl
                        << "verify one one\\-hundred\\-and\\-nineteen" << std::endl
                        << "verify one one\\-hundred\\-and\\-twenty" << std::endl
                        << "verify one one\\-hundred\\-and\\-twenty\\-one" << std::endl
                        << "verify one one\\-hundred\\-and\\-twenty\\-two" << std::endl
                        << "verify one one\\-hundred\\-and\\-twenty\\-three" << std::endl
                        << "verify one one\\-hundred\\-and\\-twenty\\-four" << std::endl
                        << "verify one one\\-hundred\\-and\\-twenty\\-five" << std::endl
                        << "verify one one\\-hundred\\-and\\-twenty\\-six" << std::endl
                        << "verify one one\\-hundred\\-and\\-twenty\\-seven" << std::endl
                        << "verify one one\\-hundred\\-and\\-twenty\\-eight" << std::endl
                        << "verify one one\\-hundred\\-and\\-twenty\\-nine" << std::endl
                        << "verify one one\\-hundred\\-and\\-thirty" << std::endl
                        << "verify one one\\-hundred\\-and\\-thirty\\-one" << std::endl
                        << "verify one one\\-hundred\\-and\\-thirty\\-two" << std::endl
                        << "verify one one\\-hundred\\-and\\-thirty\\-three" << std::endl
                        << "verify one one\\-hundred\\-and\\-thirty\\-four" << std::endl
                        << "verify one one\\-hundred\\-and\\-thirty\\-five" << std::endl
                        << "verify one one\\-hundred\\-and\\-thirty\\-six" << std::endl
                        << "verify one one\\-hundred\\-and\\-thirty\\-seven" << std::endl
                        << "verify one one\\-hundred\\-and\\-thirty\\-eight" << std::endl
                        << "verify one one\\-hundred\\-and\\-thirty\\-nine" << std::endl
                        << "verify one one\\-hundred\\-and\\-forty" << std::endl
                        << "verify one one\\-hundred\\-and\\-forty\\-one" << std::endl
                        << "verify one one\\-hundred\\-and\\-forty\\-two" << std::endl
                        << "verify one one\\-hundred\\-and\\-forty\\-three" << std::endl
                        << "verify one one\\-hundred\\-and\\-forty\\-four" << std::endl
                        << "verify one one\\-hundred\\-and\\-forty\\-five" << std::endl
                        << "verify one one\\-hundred\\-and\\-forty\\-six" << std::endl
                        << "verify one one\\-hundred\\-and\\-forty\\-seven" << std::endl
                        << "verify one one\\-hundred\\-and\\-forty\\-eight" << std::endl
                        << "verify one one\\-hundred\\-and\\-forty\\-nine" << std::endl
                        << "verify one one\\-hundred\\-and\\-fifty" << std::endl
                        << "verify one one\\-hundred\\-and\\-fifty\\-one" << std::endl
                        << "verify one one\\-hundred\\-and\\-fifty\\-two" << std::endl
                        << "verify one one\\-hundred\\-and\\-fifty\\-three" << std::endl
                        << "verify one one\\-hundred\\-and\\-fifty\\-four" << std::endl
                        << "verify one one\\-hundred\\-and\\-fifty\\-five" << std::endl
                        << "verify one one\\-hundred\\-and\\-fifty\\-six" << std::endl
                        << "verify one one\\-hundred\\-and\\-fifty\\-seven" << std::endl
                        << "verify one one\\-hundred\\-and\\-fifty\\-eight" << std::endl
                        << "verify one one\\-hundred\\-and\\-fifty\\-nine" << std::endl
                        << "verify one one\\-hundred\\-and\\-sixty" << std::endl
                        << "verify one one\\-hundred\\-and\\-sixty\\-one" << std::endl
                        << "verify one one\\-hundred\\-and\\-sixty\\-two" << std::endl
                        << "verify one one\\-hundred\\-and\\-sixty\\-three" << std::endl
                        << "verify one one\\-hundred\\-and\\-sixty\\-four" << std::endl
                        << "verify one one\\-hundred\\-and\\-sixty\\-five" << std::endl
                        << "verify one one\\-hundred\\-and\\-sixty\\-six" << std::endl
                        << "verify one one\\-hundred\\-and\\-sixty\\-seven" << std::endl
                        << "verify one one\\-hundred\\-and\\-sixty\\-eight" << std::endl
                        << "verify one one\\-hundred\\-and\\-sixty\\-nine" << std::endl
                        << "verify one one\\-hundred\\-and\\-seventy" << std::endl
                        << "verify one one\\-hundred\\-and\\-seventy\\-one" << std::endl
                        << "verify one one\\-hundred\\-and\\-seventy\\-two" << std::endl
                        << "verify one one\\-hundred\\-and\\-seventy\\-three" << std::endl
                        << "verify one one\\-hundred\\-and\\-seventy\\-four" << std::endl
                        << "verify one one\\-hundred\\-and\\-seventy\\-five" << std::endl
                        << "verify one one\\-hundred\\-and\\-seventy\\-six" << std::endl
                        << "verify one one\\-hundred\\-and\\-seventy\\-seven" << std::endl
                        << "verify one one\\-hundred\\-and\\-seventy\\-eight" << std::endl
                        << "verify one one\\-hundred\\-and\\-seventy\\-nine" << std::endl
                        << "verify one one\\-hundred\\-and\\-eighty" << std::endl
                        << "verify one one\\-hundred\\-and\\-eighty\\-one" << std::endl
                        << "verify one one\\-hundred\\-and\\-eighty\\-two" << std::endl
                        << "verify one one\\-hundred\\-and\\-eighty\\-three" << std::endl
                        << "verify one one\\-hundred\\-and\\-eighty\\-four" << std::endl
                        << "verify one one\\-hundred\\-and\\-eighty\\-five" << std::endl
                        << "verify one one\\-hundred\\-and\\-eighty\\-six" << std::endl
                        << "verify one one\\-hundred\\-and\\-eighty\\-seven" << std::endl
                        << "verify one one\\-hundred\\-and\\-eighty\\-eight" << std::endl
                        << "verify one one\\-hundred\\-and\\-eighty\\-nine" << std::endl
                        << "verify one one\\-hundred\\-and\\-ninety" << std::endl
                        << "verify one one\\-hundred\\-and\\-ninety\\-one" << std::endl
                        << "verify one one\\-hundred\\-and\\-ninety\\-two" << std::endl;


                tool.Main(input,
                          std::cout,
                          static_cast<int>(argv.size()),
                          argv.data());
            }
        }
    }


//     // TODO: extract logic into class/function instead of copy-pasting above
//     // test.
//     TEST(BitFunnelToolOld, ThreeToolsEndToEndInterpreterMultiShard)
//     {
//         //
//         // This test is going to run out of a RAM filesystem.
//         //
//         auto fileSystem = BitFunnel::Factories::CreateRAMFileSystem();
//         auto fileManager =
//             BitFunnel::Factories::CreateFileManager(
//                 "config",
//                 "statistics",
//                 "index",
//                 *fileSystem);
//
//
//         //
//         // Create ShardDefinition
//         //
//         {
//             // This should cause the test to fail because sharding is
//             // incompletely implemented. However, that doesn't seem to happen
//             // because it's so incompletely implemented that we don't even read
//             // this file. Neither StatisticsBuilder nor TermTableBuilder are
//             // shard aware.
//             //
//             // TODO: have tools actually use ShardDefinition. See #291 for more
//             // discussion.
//
//             // According to FileManager, ShardDefinition is in statisticsDirectory.
//             auto shardDefinition = fileSystem->OpenForWrite("config/ShardDefinition.csv");
//             *shardDefinition << "50";
//         }
//
//         //
//         // Initialize RAM filesystem with input files.
//         //
//         {
//             // Open the manifest file.
//             auto manifest = fileSystem->OpenForWrite("manifest.txt");
//
//             // Iterate over sequence of Shakespeare sonnet chunk data.
//             for (size_t i = 0; i < Sonnets::chunks.size(); ++i)
//             {
//                 // Create chunk file name, and write chunk data.
//                 std::stringstream name;
//                 name << "sonnet" << i;
//                 auto out = fileSystem->OpenForWrite(name.str().c_str());
//                 out->write(Sonnets::chunks[i].second,
//                            static_cast<std::streamsize>(Sonnets::chunks[i].first));
//
//                 // Add chunk file to manifest.
//                 *manifest << name.str() << std::endl;
//             }
//
//             auto script = fileSystem->OpenForWrite("testScript");
//             *script << "failOnException" << std::endl
//                     << "cache chunk sonnet0" << std::endl;
//         }
//
//         //
//         // Create the BitFunnelTool based on the RAM filesystem.
//         //
//         BitFunnel::BitFunnelTool tool(*fileSystem);
//
//         //
//         // Use the tool to run the statistics builder.
//         //
//         {
//             std::vector<char const *> argv = {
//                 "BitFunnel",
//                 "statistics",
//                 "manifest.txt",
//                 "config"
//             };
//
//             tool.Main(std::cin,
//                       std::cout,
//                       static_cast<int>(argv.size()),
//                       argv.data());
//         }
//
//
//         //
//         // Use the tool to run the TermTable builder.
//         //
//         {
//             std::vector<char const *> argv = {
//                 "BitFunnel",
//                 "termtable",
//                 "config",
//                 "0.1",
//                 "PrivateSharedRank0And3"
//             };
//
//             tool.Main(std::cin,
//                       std::cout,
//                       static_cast<int>(argv.size()),
//                       argv.data());
//         }
//
//
//         //
//         // Use the tool to run the REPL.
//         //
//         {
//             std::vector<char const *> argv = {
//                 "BitFunnel",
//                 "repl",
//                 "config",
//                 // -script and testScript must be on seperate lines because
//                 // tokens are delimited by whitespace.
//                 "-script",
//                 "testScript"
//             };
//
//             // Create an input stream with commands to
//             // load a chunk, verify a query, and inspect
//             // some rows.
//             std::stringstream input;
//             input
//                 // This first line is run via -script.
//                 // << "cache chunk sonnet0" << std::endl
//                 << "interpreter" << std::endl
//                 << "verify one blood" << std::endl
//                 << "show rows blood" << std::endl;
//
//             std::stringstream output;
//             tool.Main(input,
//                       output,
//                       static_cast<int>(argv.size()),
//                       argv.data());
//
//             // TODO: this is an extremely brittle way to check if we have false
//             // positives. This is being done this way because it appears that
//             // it's a standard pattern for our commands to entire write to a
//             // file or write to a stream. In order to make commands more easily
//             // tstable, we should probably return a data structure and have the
//             // REPL print out a readable form of the data structure.
//             std::string text = output.str();
//
//             // TODO: only print out text on failure.
//             std::cout << text;
//
//             size_t false_positive_found = text.find("False positives:");
//             if (false_positive_found != std::string::npos)
//             {
//                 FAIL() << "Found false positives.";
//             }
//         }
//     }


    // // TODO: extract logic into class/function instead of copy-pasting above
    // // test.
    // TEST(BitFunnelToolOld, ThreeToolsEndToEndMultiShard)
    // {
    //     //
    //     // This test is going to run out of a RAM filesystem.
    //     //
    //     auto fileSystem = BitFunnel::Factories::CreateRAMFileSystem();
    //     auto fileManager =
    //         BitFunnel::Factories::CreateFileManager(
    //             "config",
    //             "statistics",
    //             "index",
    //             *fileSystem);


    //     //
    //     // Create ShardDefinition
    //     //
    //     {
    //         // This should cause the test to fail because sharding is
    //         // incompletely implemented. However, that doesn't seem to happen
    //         // because it's so incompletely implemented that we don't even read
    //         // this file. Neither StatisticsBuilder nor TermTableBuilder are
    //         // shard aware.
    //         //
    //         // TODO: have tools actually use ShardDefinition. See #291 for more
    //         // discussion.

    //         // According to FileManager, ShardDefinition is in statisticsDirectory.
    //         auto shardDefinition = fileSystem->OpenForWrite("config/ShardDefinition.csv");
    //         *shardDefinition << "70";
    //     }

    //     //
    //     // Initialize RAM filesystem with input files.
    //     //
    //     {
    //         // Open the manifest file.
    //         auto manifest = fileSystem->OpenForWrite("manifest.txt");

    //         // Iterate over sequence of Shakespeare sonnet chunk data.
    //         for (size_t i = 0; i < Sonnets::chunks.size(); ++i)
    //         {
    //             // Create chunk file name, and write chunk data.
    //             std::stringstream name;
    //             name << "sonnet" << i;
    //             auto out = fileSystem->OpenForWrite(name.str().c_str());
    //             out->write(Sonnets::chunks[i].second,
    //                        static_cast<std::streamsize>(Sonnets::chunks[i].first));

    //             // Add chunk file to manifest.
    //             *manifest << name.str() << std::endl;
    //         }

    //         auto script = fileSystem->OpenForWrite("testScript");
    //         *script << "failOnException" << std::endl
    //                 << "cache chunk sonnet0" << std::endl;
    //     }

    //     //
    //     // Create the BitFunnelTool based on the RAM filesystem.
    //     //
    //     BitFunnel::BitFunnelTool tool(*fileSystem);

    //     //
    //     // Use the tool to run the statistics builder.
    //     //
    //     {
    //         std::vector<char const *> argv = {
    //             "BitFunnel",
    //             "statistics",
    //             "manifest.txt",
    //             "config"
    //         };

    //         tool.Main(std::cin,
    //                   std::cout,
    //                   static_cast<int>(argv.size()),
    //                   argv.data());
    //     }


    //     //
    //     // Use the tool to run the TermTable builder.
    //     //
    //     {
    //         std::vector<char const *> argv = {
    //             "BitFunnel",
    //             "termtable",
    //             "config",
    //             "0.1",
    //             "PrivateSharedRank0And3"
    //         };

    //         tool.Main(std::cin,
    //                   std::cout,
    //                   static_cast<int>(argv.size()),
    //                   argv.data());
    //     }


    //     //
    //     // Use the tool to run the REPL.
    //     //
    //     {
    //         std::vector<char const *> argv = {
    //             "BitFunnel",
    //             "repl",
    //             "config",
    //             // -script and testScript must be on seperate lines because
    //             // tokens are delimited by whitespace.
    //             "-script",
    //             "testScript"
    //         };

    //         // Create an input stream with commands to
    //         // load a chunk, verify a query, and inspect
    //         // some rows.
    //         std::stringstream input;
    //         input
    //             // This first line is run via -script.
    //             // << "cache chunk sonnet0" << std::endl
    //             << "verify one blood" << std::endl
    //             << "show rows blood" << std::endl;

    //         std::stringstream output;
    //         tool.Main(input,
    //                   output,
    //                   static_cast<int>(argv.size()),
    //                   argv.data());

    //         // TODO: this is an extremely brittle way to check if we have false
    //         // positives. This is being done this way because it appears that
    //         // it's a standard pattern for our commands to entire write to a
    //         // file or write to a stream. In order to make commands more easily
    //         // tstable, we should probably return a data structure and have the
    //         // REPL print out a readable form of the data structure.
    //         std::string text = output.str();

    //         // TODO: only print out text on failure.
    //         std::cout << text;

    //         size_t false_positive_found = text.find("False positives:");
    //         if (false_positive_found != std::string::npos)
    //         {
    //             FAIL() << "Found false positives.";
    //         }
    //     }
    // }
}
