#include <stdio.h>
#include <cstdint>
#include <iostream>
#include "gtest/gtest.h"
#include "glog/logging.h"

#include "test_util.h"
#include "tudocomp_driver/registry.h"
#include "tudocomp_driver/AlgorithmStringParser.hpp"

#include "tudocomp_driver_util.h"

TEST(TudocompDriver, help) {
    ASSERT_EQ(driver("--help"),
"TuDo Comp.\n"
"\n"
"Usage:\n"
"    tudocomp [options] [-k]  -a <alg>  [-o <output>] [--] ( <input> | - )\n"
"    tudocomp [options]  -d  [-a <alg>] [-o <output>] [--] ( <input> | - )\n"
"    tudocomp --list\n"
"    tudocomp --help\n"
"\n"
"Options:\n"
"    -h --help               Show this screen.\n"
"    -a --algorithm <alg>    Use algorithm <alg> for (de)compression.\n"
"                            <alg> can be a dot-separated chain of\n"
"                            sub-algorithms. See --list for a complete list\n"
"                            of them.\n"
"                            Example: -a lz77rule.esa.esa_code0\n"
"    -k --compress           Compress input instead of compressing it.\n"
"    -d --decompress         Decompress input instead of compressing it.\n"
"    -o --output <output>    Choose output filename instead the the default of\n"
"                            <input>.<compressor name>.<encoder name>.tdc\n"
"                            or stdout if reading from stdin.\n"
"    -s --stats              Print statistics to stdout.\n"
"    -f --force              Overwrite output even if it exists.\n"
"    -l --list               List all Compression algorithms supported\n"
"                            by this tool.\n"
"                            Algorithms may consist out of sub-algorithms,\n"
"                            which will be displayed in a hierarchical fashion.\n"
"    -r --raw                Do not emit an header when compressing.\n"
"    -O --option <option>    An additional option of the form key=value.\n"
"\n"
    );
}

TEST(TudocompDriver, list) {
    // Test that we got at least the amount of algorithms
    // we had when writing this test.

    //TODO this test does not make any sense this way...
    //TODO should somehow compare against registry instead

    /*
    auto list = list::tudocomp_list();

    ASSERT_EQ(list.header, "This build supports the following algorithms:");

    auto& root = list.root;

    ASSERT_GE(root.algos.size(), 2u);

    {
        auto& r0 = root.algos[0];
        ASSERT_GE(r0.subalgos.size(), 2u);
        {
            auto& r00 = r0.subalgos[0];
            ASSERT_GE(r00.algos.size(), 4u);
        }
        {
            auto& r01 = r0.subalgos[1];
            ASSERT_GE(r01.algos.size(), 4u);
        }
    }
    {
        auto& r1 = root.algos[1];
        ASSERT_GE(r1.subalgos.size(), 1u);
        {
            auto& r10 = r1.subalgos[0];
            ASSERT_GE(r10.algos.size(), 1u);
        }
    }
    */
}

TEST(TudocompDriver, algorithm_header) {
    std::string text = "asdfghjklöä";
    bool abort = false;
    // Without header
    roundtrip("lz78.debug", "_header_test_0", text, true, abort);

    // With header
    roundtrip("lz78.debug", "_header_test_1", text, false, abort);

    ASSERT_FALSE(abort);

    std::string text0 = read_test_file(roundtrip_comp_file_name(
        "lz78.debug", "_header_test_0"));

    ASSERT_TRUE(text0.find('(') == 0);

    std::string text1 = read_test_file(roundtrip_comp_file_name(
        "lz78.debug", "_header_test_1"));

    ASSERT_TRUE(text1.find("lz78.debug%(") == 0);

}

TEST(NewAlgorithmStringParser, smoketest) {
    using namespace tudocomp_driver;
    Parser p { "foo(abc, def=ghi, ijk=lmn(o, p))" };

    auto x = p.parse().unwrap();

    ASSERT_EQ(x.name, "foo");
    ASSERT_EQ(x.args.size(), 3);
}
