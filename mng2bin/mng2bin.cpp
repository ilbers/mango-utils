/*
 * Mango partitions configuration sctipt converter.
 *
 * Copyright (c) 2014-2015 ilbers GmbH
 * Alexander Smirnov <asmirnov@ilbers.de>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2
 * as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

#include <stdint.h>

#include <fstream>
#include <sstream>
#include <iostream>
#include <iomanip>
#include <vector>
#include <iterator>

#define PARTITIONS_NUMBER	2
#define PARTITION_ENTRIES	64

typedef struct {
    uint32_t pa;
    uint32_t va;
    uint32_t size;
    uint32_t type;
} memory_table_t;

typedef struct {
    uint32_t irq;
} irq_table_t;

typedef struct {
    irq_table_t    irq_table[PARTITION_ENTRIES];
    uint32_t       irq_fill;
    memory_table_t mem_table[PARTITION_ENTRIES];
    uint32_t       mem_fill;
} partition_t;

static partition_t Partitions[PARTITIONS_NUMBER];

static inline int32_t CheckTokensLength(const std::vector<std::string>& LineTokens,
                                        uint32_t Length,
                                        uint32_t Line)
{
    uint32_t retVal = 0;

    if ((LineTokens.size() > Length) && (LineTokens[Length][0] != ';'))
    {
        std::cerr << "[error] line " << Line << ": too many arguments" << std::endl;
        retVal = -1;
    }
    else
    if (LineTokens.size() < Length)
    {
        std::cerr << "[error] line " << Line << ": too few arguments" << std::endl;
        retVal = -1;
    }

    return retVal;
}

static int32_t Mango_ParseConfigFile(const std::string& FileName)
{
    std::ifstream configFile(FileName.c_str());
    std::string configLine;
    int32_t partitionId = -1;
    int32_t retVal = 0;
    uint32_t count = 1;

    if (!configFile.is_open())
    {
        std::cerr << "[error]: failed to open file " << FileName << std::endl;
        return -1;
    }

    while (std::getline(configFile, configLine, '\n'))
    {
        std::vector<std::string> lineTokens;
        std::istringstream issTokens(configLine);

        copy(std::istream_iterator<std::string>(issTokens),
             std::istream_iterator<std::string>(),
             std::back_inserter< std::vector<std::string> >(lineTokens));

        if ((lineTokens.size() == 0) || (lineTokens[0][0] == ';'))
        {
            count++;
            continue;
        }

        if (lineTokens[0] == "partition")
        {
            if (CheckTokensLength(lineTokens, 2, count))
            {
                retVal = -1;
                break;
            }
            else
            if (partitionId == -1)
            {
                if (((std::istringstream(lineTokens[1]) >> partitionId) == 0) ||
                    (partitionId >= PARTITIONS_NUMBER))
                {
                    std::cerr << "[error] line " << count << ": invalid partition id" << std::endl;
                    retVal = -1;
                    break;
                }

            }
            else
            {
                std::cerr << "[error]: line " << count << ": nested partitions" << std::endl;
                retVal = -1;
                break;
            }
        }
        else
        if (lineTokens[0] == "endp")
        {
            if (CheckTokensLength(lineTokens, 1, count))
            {
                retVal = -1;
                break;
            }
            else
            if (partitionId == -1)
            {
                    std::cerr << "[error] line " << count << ": no partition section openned" << std::endl;
                    retVal = -1;
                    break;
            }
            else
            {
                partitionId = -1;
            }
        }
        else
        if (lineTokens[0] == "mem")
        {
            if (CheckTokensLength(lineTokens, 5, count))
            {
                retVal = -1;
                break;
            }
            else
            if (partitionId == -1)
            {
                std::cerr << "[error] line " << count << ": no partition section openned" << std::endl;
                retVal = -1;
                break;
            }
            else
            {
                uint32_t va;
                uint32_t pa;
                uint32_t size;
                uint32_t type;
                std::stringstream ss;

                ss << std::hex << lineTokens[1];
                if ((ss >> va) == 0)
                {
                    std::cerr << "[error] line " << count << ": invalid virtual address" << std::endl;
                    retVal = -1;
                    break;
                }

                ss.str("");
                ss.clear();
                ss << std::hex << lineTokens[2];
                if ((ss >> pa) == 0)
                {
                    std::cerr << "[error] line " << count << ": invalid physical address" << std::endl;
                    retVal = -1;
                    break;
                }

                ss.str(std::string());
                ss.clear();
                ss << std::hex << lineTokens[3];
                if ((ss >> size) == 0)
                {
                    std::cerr << "[error] line " << count << ": invalid region size" << std::endl;
                    retVal = -1;
                    break;
                }

                if (lineTokens[4] == "normal")
                {
                    type = 0;
                }
                else
                if (lineTokens[4] == "device")
                {
                    type = 1;
                }
                else
                {
                    std::cerr << "[error] line " << count << ": invalid memory type" << std::endl;
                    retVal = -1;
                    break;
                }

                if (Partitions[partitionId].mem_fill < PARTITION_ENTRIES)
                {
                    uint32_t id = Partitions[partitionId].mem_fill++;

                    Partitions[partitionId].mem_table[id].va   = va;
                    Partitions[partitionId].mem_table[id].pa   = pa;
                    Partitions[partitionId].mem_table[id].size = size;
                    Partitions[partitionId].mem_table[id].type = type;
                }
                else
                {
                    std::cerr << "[error] line " << count << ": table entries exceeded" << std::endl;
                    retVal = -1;
                    break;
                }
            }
        }
        else
        if (lineTokens[0] == "irq")
        {
            if (CheckTokensLength(lineTokens, 2, count))
            {
                retVal = -1;
                break;
            }
            else
            if (partitionId == -1)
            {
                std::cerr << "[error] line " << count << ": no partition section openned" << std::endl;
                retVal = -1;
                break;
            }
            else
            {
                uint32_t irq;
                std::stringstream ss;

                ss << std::hex << lineTokens[1];
                if ((ss >> irq) == 0)
                {
                    std::cerr << "[error] line " << count << ": invalid irq number" << std::endl;
                    retVal = -1;
                    break;
                }

                if (Partitions[partitionId].irq_fill < PARTITION_ENTRIES)
                {
                    uint32_t id = Partitions[partitionId].irq_fill++;

                    Partitions[partitionId].irq_table[id].irq = irq;
                }
            }
        }
        else
        {
            std::cerr << "[error] line " << count << ": invalid token" << std::endl;
            retVal = -1;
            break;
        }

        count++;
    }

    if (retVal)
    {
        return retVal;
    }

    if (partitionId != -1)
    {
        std::cerr << "[error]: partition section should be closed" << std::endl;
        retVal = -1;
    }

    return retVal;
}

static void Mango_ShowConfiguration()
{
    uint32_t i;

    for (i = 0; i < PARTITIONS_NUMBER; i++)
    {
        uint32_t j;

        std::cout << "Partition #" << i << ":" << std::endl;

        std::cout << "  Memory:" << std::endl;
        for (j = 0; j < Partitions[i].mem_fill; j++)
        {
            std::cout << "    "  << std::setw(8) << std::hex << Partitions[i].mem_table[j].va;
            std::cout << " "  << std::setw(8) << std::hex << Partitions[i].mem_table[j].pa;
            std::cout << " "  << std::setw(8) << std::hex << Partitions[i].mem_table[j].size;
            std::cout << " "  << std::setw(8) << std::hex << Partitions[i].mem_table[j].type << std::endl;
        }

        std::cout << "  Interrupts:" << std::endl;
        for (j = 0; j < Partitions[i].irq_fill; j++)
        {
            std::cout << "    "  << std::hex << Partitions[i].irq_table[j].irq << std::endl;
        }
    }
}

typedef struct {
    uint32_t MemoryEntries;
    uint32_t IRQEntries;
    uint32_t Offset;
} script_part_t;

typedef struct {
    uint32_t      Magic;
    uint32_t      PartsNum;
    uint32_t      HeaderSize;
    script_part_t Parts[PARTITIONS_NUMBER];
} script_header_t;

static int32_t Mango_SaveConfiguration(const std::string& FileName)
{
    std::ofstream scriptFile(FileName.c_str(), std::ios::trunc | std::ios::out | std::ios::binary);
    script_header_t scriptHeader;
    uint32_t retVal = 0;
    uint32_t offset = 0;
    uint32_t i;

    scriptHeader.Magic = 0xaa10bb20;
    scriptHeader.PartsNum = PARTITIONS_NUMBER;
    scriptHeader.HeaderSize = sizeof(script_header_t);

    for (i = 0; i < PARTITIONS_NUMBER; i++)
    {
        scriptHeader.Parts[i].Offset = offset;
        scriptHeader.Parts[i].MemoryEntries = Partitions[i].mem_fill;
        scriptHeader.Parts[i].IRQEntries    = Partitions[i].irq_fill;

        offset +=  Partitions[i].mem_fill * 16;
        offset +=  Partitions[i].irq_fill * 4;
    }

    scriptFile.write(reinterpret_cast<char*>(&scriptHeader), sizeof(script_header_t));

    for (i = 0; i < PARTITIONS_NUMBER; i++)
    {
        uint32_t j;

        for (j = 0; j < Partitions[i].mem_fill; j++)
        {
            scriptFile.write(reinterpret_cast<char*>(&Partitions[i].mem_table[j].va), 4);
            scriptFile.write(reinterpret_cast<char*>(&Partitions[i].mem_table[j].pa), 4);
            scriptFile.write(reinterpret_cast<char*>(&Partitions[i].mem_table[j].size), 4);
            scriptFile.write(reinterpret_cast<char*>(&Partitions[i].mem_table[j].type), 4);
        }

        for (j = 0; j < Partitions[i].irq_fill; j++)
        {
            scriptFile.write(reinterpret_cast<char*>(&Partitions[i].irq_table[j].irq), 4);
        }
    }

    scriptFile.close();

    return retVal;
}

static void Mango_ShowHelp()
{
    std::cout << "This tools converts partitions configuration file to" << std::endl;
    std::cout << "binary script, that can be processed by Mango hypervisor." << std::endl;
    std::cout << std::endl;
    std::cout << "    Usage:" << std::endl;
    std::cout << "        mng2bin config.mng config.bin" << std::endl;
    std::cout << std::endl;
}

int32_t main(int32_t argc, char* argv[])
{
    uint32_t i;

    if (argc != 3)
    {
        Mango_ShowHelp();
        return -1;
    }

    for (i = 0; i < PARTITIONS_NUMBER; i++)
    {
        Partitions[i].mem_fill = 0;
        Partitions[i].irq_fill = 0;
    }

    Mango_ParseConfigFile(argv[1]);

    Mango_ShowConfiguration();

    Mango_SaveConfiguration(argv[2]);

    return 0;
}
